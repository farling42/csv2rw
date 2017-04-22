/*
CSV2RW
Copyright (C) 2017 Martin Smith

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "rw_facet.h"
#include <QXmlStreamWriter>
#include <QDebug>
#include <QModelIndex>
#include <QMetaEnum>
#include "rw_domain.h"

static QMetaEnum snip_type_enum  = QMetaEnum::fromType<RWFacet::SnippetType>();
static QMetaEnum snip_style_enum = QMetaEnum::fromType<RWFacet::SnippetStyle>();

RWFacet::RWFacet(QXmlStreamReader *stream, QObject *parent) :
    RWBaseItem(stream, parent),
    p_snippet_type(Multi_Line),
    p_snippet_style(Normal)
{
    static bool first = true;
    if (first)
    {
        qRegisterMetaType<SnippetType>();
        first = false;
    }
    const QXmlStreamAttributes attr = attributes();

    // Decode the 'type' attribute (if present)
    if (attr.hasAttribute("type"))
    {
        QString ftype = attributes().value("type").toString();
        bool ok = true;
        p_snippet_type = (SnippetType) snip_type_enum.keyToValue(qPrintable(ftype), &ok);
        if (!ok) qWarning() << "Unknown SNIPPET type" << ftype;
    }
    else
        p_snippet_type = Multi_Line;
}

void RWFacet::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    Q_UNUSED(index);
    bool bold = false;

    writer->writeStartElement("snippet");
    {
        if (!id().isEmpty()) writer->writeAttribute("facet_id", id());
        if (isRevealed()) writer->writeAttribute("is_revealed", "true");
        writer->writeAttribute("type", snip_type_enum.valueToKey(p_snippet_type));

        // Maybe an ANNOTATION or CONTENTS (before TAG_ASSIGN)
        const QString user_text = contentsText().valueString(index);
        if (!user_text.isEmpty())
        {
#if 0
            if (id().isEmpty() && p_snippet_type == Labeled_Text && !p_label_text.isEmpty())
            {
                // For locally added snippets of the Labeled_Text variety
                writer->writeAttribute("label", p_label_text);
            }
            if (p_snippet_style != Normal)
            {
                writer->writeAttribute("style", snip_type_enum.valueToKey(p_snippet_style));
            }
#endif
            QString sub_element;
            switch (p_snippet_type)
            {
            case Multi_Line:
                sub_element = "contents";
                break;
            case Labeled_Text:
                sub_element = "contents";
                break;
            case Tag_Standard:
                sub_element = "annotation";
                break;
            case Hybrid_Tag:
                sub_element = "annotation";
                break;
            default:
                qFatal("RWFacet::writeToContents: invalid snippet type: %d", p_snippet_type);
                break;
            }

            // Maybe the snippet has some contents
            writer->writeStartElement(sub_element);
            writer->writeCharacters(xmlParagraph(xmlSpan(user_text, bold)));
            writer->writeEndElement();  // for contents or annotation
        }

        // Maybe one or more TAG_ASSIGN (to be entered AFTER the contents/annotation)
        QString tag_names = p_tags.valueString(index);
        if (!tag_names.isEmpty())
        {
            // Find the domain to use
            QString domain_id = attributes().value("domain_id").toString();
            RWDomain *domain = RWDomain::getDomainById(domain_id);
            if (domain)
            {
                foreach (QString tag_name, tag_names.split(","))
                {
                    QString tag_id = domain->tagId(tag_name.trimmed());
                    if (!tag_id.isEmpty())
                    {
                        writer->writeStartElement("tag_assign");
                        writer->writeAttribute("tag_id", tag_id);
                        writer->writeAttribute("type","Indirect");
                        writer->writeAttribute("is_auto_assign", "true");
                        writer->writeEndElement();
                    }
                    else
                        qWarning() << "No TAG defined for" << tag_name.trimmed() << "in DOMAIN" << domain->name();
                }
            }
            else if (!domain_id.isEmpty())
                qWarning() << "DOMAIN not found for" << domain_id << "on FACET" << id();
            else
                qWarning() << "domain_id does not exist on FACET" << id();
        }

    }
    writer->writeEndElement();  // snippet
}
