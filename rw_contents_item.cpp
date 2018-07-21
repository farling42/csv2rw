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

#include "rw_contents_item.h"
#include <QXmlStreamWriter>
#include <QModelIndex>
#include <QDebug>
#include "rw_category.h"    // to stop iteration into lower RWCategory
#include "regexp.h"

#undef DEBUG_XML

/* This is only here because DataField doesn't have a .c file */
int DataField::p_column_offset = 0;

RWContentsItem::RWContentsItem(RWStructureItem *item, RWContentsItem *parent) :
    QObject(parent),
    structure(item),
    p_snippet_style(Normal),
    p_snippet_veracity(Truth),
    p_snippet_purpose(Story_Only)
{
    // Extract common data from this XML element
//    p_attributes = reader->attributes();  // TODO

    p_revealed = item->attributes().value("is_revealed") == "true";

#ifdef DEBUG_XML
    qDebug() << *this;
#endif
}

void RWContentsItem::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index) const
{
    // Special case: never put text_override into the contents section
    if (structure->ignoreForContents()) return;

    if (p_structure_element == "overlay")
    {
        // "<overlay>" can appear in domain_global, facet_global, partition_globaland category_global elements,
        // but there is no clear indication what they are used for!
        return;
    }
    writer->writeStartElement(p_structure_element);

    if (!structure->id().isEmpty()) writer->writeAttribute(p_structure_element + "_id", structure->id());   // e.g. partition_id, not the same as <element>_id

    //QString user_text = p_text.valueString(index);
    //if (!user_text.isEmpty()) writer->writeCharacters(user_text);
    writeChildrenToContents(writer, index);
    writer->writeEndElement();
}

void RWContentsItem::writeChildrenToContents(QXmlStreamWriter *writer, const QModelIndex &index) const
{
    // Get only the content items (ignore all RWCategory children)
    for (auto item: childItems<RWContentsItem*>())
    {
        item->writeToContents(writer, index);
    }
}

void RWContentsItem::writeExportTag(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("tag_assign");
    writer->writeAttribute("tag_id", "Tag_1");
    writer->writeEndElement();
}

/**
 * @brief RWBaseItem::canBeGenerated
 * @return true if this element has all the data required for the GENERATE to be a success.
 */
bool RWContentsItem::canBeGenerated() const
{
    for (auto item: findChildren<RWContentsItem*>())
    {
        if (!item->canBeGenerated()) return false;
    }
    return true;
}

QDebug operator<<(QDebug stream, const RWContentsItem &item)
{
    stream.noquote().nospace() << item.metaObject()->className() << "(" << item.p_structure_element;
    stream.noquote().nospace() << " : ";
    QString text = item.p_contents_text.valueString();
    if (!text.isEmpty()) stream.noquote().nospace() << ":: value=\"" + text + "\"";
    stream.nospace() << ")";

    return stream;
}


QString RWContentsItem::xmlParagraph(const QString &text, TextClass text_class, int margin)
{
    switch (text_class)
    {
    case RWDefault:
        return "<p class=\"RWDefault\">" + text + "</p>";
    case RWEnumerated:
        return "<p class=\"RWEnumerated\">" + text + "</p>";
    case RWSnippet:
        return "<p class=\"RWSnippet\" style=\"margin:" + QString::number(margin) + "pt 0pt 0pt 0pt;text-align:left;text-indent:0pt\">" + text + "</p>";
    }
    return QString();
}

QString RWContentsItem::xmlSpan(const QString &text, bool bold, bool italic, bool line_through, bool underline)
{
    // text-decoration - space separated list
    QStringList decorations;
    if (underline) decorations.append("underline");
    if (line_through) decorations.append("line-through");
    // Styles - semi-colon separated list
    QStringList styles;
    if (!decorations.isEmpty()) styles.append("text-decoration:" + decorations.join(' '));
    if (bold) styles.append("font-weight:bold");
    if (italic) styles.append("font-style:italic");

    QString style;
    if (!styles.isEmpty())
    {
        style = " style=\"" + styles.join(';') + "\"";
    }

    const QString start_rwsnippet(QString("<span class=\"RWSnippet\"%1>").arg(style));

    // Subscript   uses <sub> ... </sub>
    // Superscript uses <sup> ... </sup>
    QString buffer = text;

    // Prevent "<" being interpreted as the start of an element, but not if the entire field looks like XML/HTML
    if (!buffer.startsWith('<') || !buffer.endsWith('>'))
    {
        buffer.replace("<", "&lt;");
    }

    // Parse for possible URLs, remembering style for future text
    const QString url_replacement("<a class=\"RWLink\" style=\"color:#000000;text-decoration:none\" href=\"\\1\" title=\"\\1\"><span class=\"RWLink\">\\1</span></a></span>" + start_rwsnippet);
    return start_rwsnippet + buffer.replace(url_regexp, url_replacement) + "</span>";
}

RWContentsItem *RWContentsItem::childElement(const QString &element_name) const
{
    for (auto child: children())
    {
        RWContentsItem *item = qobject_cast<RWContentsItem*>(child);
        if (item && item->structureElement() == element_name)
            return item;
    }
    return nullptr;
}
