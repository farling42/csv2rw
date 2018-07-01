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

RWContentsItem::RWContentsItem(QXmlStreamReader *reader, QObject *parent, bool ignore_for_contents) :
    QObject(parent),
    p_snippet_style(Normal),
    p_snippet_veracity(Truth),
    p_snippet_purpose(Story_Only),
    p_ignore_for_contents(ignore_for_contents)
{
    // Extract common data from this XML element
    p_structure_element = reader->name().toString();
    p_global = reader->name().endsWith("_global");
    p_attributes = reader->attributes();
    p_namespace_uri = reader->namespaceUri().toString();

    if (p_global)
    {
        p_structure_element = p_structure_element.remove("_global");
        p_uuid = p_attributes.value("global_uuid").toString();
    }
    else
    {
        p_uuid = p_attributes.value("original_uuid").toString();
    }
    p_name = p_attributes.value("name").toString();
    p_id = p_attributes.value(p_structure_element + "_id").toString();
    p_signature = p_attributes.value("signature").toString();
    p_revealed = p_attributes.value("is_revealed") == "true";

#ifdef DEBUG_XML
    qDebug() << *this;
#endif
}

void RWContentsItem::writeToStructure(QXmlStreamWriter *writer)
{
    writer->writeStartElement(p_global ? p_structure_element + "_global" : p_structure_element);
    writer->writeAttributes(p_attributes);
    writeChildrenToStructure(writer);
    if (!p_structure_text.isEmpty()) writer->writeCharacters(p_structure_text);
    writer->writeEndElement();
}

void RWContentsItem::writeChildrenToStructure(QXmlStreamWriter *writer)
{
    QList<RWContentsItem*> child_items = childItems<RWContentsItem*>();
    foreach (RWContentsItem *child, child_items)
    {
        child->writeToStructure(writer);
    }
}

void RWContentsItem::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    // Special case: never put text_override into the contents section
    if (p_ignore_for_contents) return;

    if (p_structure_element == "overlay")
    {
        // "<overlay>" can appear in domain_global, facet_global, partition_globaland category_global elements,
        // but there is no clear indication what they are used for!
        return;
    }
    writer->writeStartElement(p_structure_element);

    if (!id().isEmpty()) writer->writeAttribute(p_structure_element + "_id", id());   // e.g. partition_id, not the same as <element>_id

    //QString user_text = p_text.valueString(index);
    //if (!user_text.isEmpty()) writer->writeCharacters(user_text);
    writeChildrenToContents(writer, index);
    writer->writeEndElement();
}

void RWContentsItem::writeChildrenToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    QList<RWContentsItem*> child_items = childItems<RWContentsItem*>();
    foreach (RWContentsItem *child, child_items)
    {
        // Don't write children which are of type RWCategory
        if (qobject_cast<RWCategory*>(child) == 0)
            child->writeToContents(writer, index);
    }
}

void RWContentsItem::writeExportTag(QXmlStreamWriter *writer)
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
    QList<RWContentsItem*> list = findChildren<RWContentsItem*>();
    foreach (RWContentsItem *item, list)
    {
        if (!item->canBeGenerated()) return false;
    }
    return true;
}

QDebug operator<<(QDebug stream, const RWContentsItem &item)
{
    stream.noquote().nospace() << item.metaObject()->className() << "(" << item.p_structure_element;
    if (item.p_global) stream.noquote().nospace() << "_global";
    stream.noquote().nospace() << " : ";
    if (!item.p_name.isEmpty()) stream.noquote().nospace() << item.p_name;
    if (!item.p_id.isEmpty()) stream.noquote().nospace() << ", id=" + item.p_id;
    if (!item.p_uuid.isEmpty()) stream.noquote().nospace() << ", uuid=" + item.p_uuid;
    if (!item.p_signature.isEmpty()) stream.noquote().nospace() << ", signature=" + item.p_signature;
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
    foreach (QObject *child, children())
    {
        RWContentsItem *item = qobject_cast<RWContentsItem*>(child);
        if (item && item->structureElement() == element_name)
            return item;
    }
    return 0;
}
