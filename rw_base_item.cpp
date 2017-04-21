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

#include "rw_base_item.h"
#include <QXmlStreamWriter>
#include <QModelIndex>
#include <QDebug>

#undef DEBUG_XML

RWBaseItem::RWBaseItem(QXmlStreamReader *reader, QObject *parent, bool ignore_for_contents) :
    QObject(parent),
    p_model_column_for_text(-1),
    p_model_column_for_tag(-1),
    p_ignore_for_contents(ignore_for_contents)
{
    // Extract common data from this XML element
    p_element_name = reader->name().toString();
    p_global = reader->name().endsWith("_global");
    p_attributes = reader->attributes();
    p_namespace_uri = reader->namespaceUri().toString();

    if (p_global)
    {
        p_element_name = p_element_name.remove("_global");
        p_uuid = p_attributes.value("global_uuid").toString();
    }
    else
    {
        p_uuid = p_attributes.value("original_uuid").toString();
    }
    p_name = p_attributes.value("name").toString();
    p_id = p_attributes.value(p_element_name + "_id").toString();
    p_signature = p_attributes.value("signature").toString();
    p_revealed = p_attributes.value("is_revealed") == "true";

#ifdef DEBUG_XML
    qDebug() << *this;
#endif
}

void RWBaseItem::writeToStructure(QXmlStreamWriter *writer)
{
    writer->writeStartElement(p_global ? p_element_name + "_global" : p_element_name);
    writer->writeAttributes(p_attributes);
    writeChildrenToStructure(writer);
    if (!p_fixed_text.isEmpty()) writer->writeCharacters(p_fixed_text);
    writer->writeEndElement();
}

void RWBaseItem::writeChildrenToStructure(QXmlStreamWriter *writer)
{
    QList<RWBaseItem*> child_items = childItems<RWBaseItem*>();
    foreach (RWBaseItem *child, child_items)
    {
        child->writeToStructure(writer);
    }
}

void RWBaseItem::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    // Special case: never put text_override into the contents section
    if (p_ignore_for_contents) return;

    writer->writeStartElement(p_element_name);

    if (!id().isEmpty()) writer->writeAttribute(p_element_name + "_id", id());   // e.g. partition_id, not the same as <element>_id

    const QString user_text = modelValueForText(index);
    if (!user_text.isEmpty())
    {
        writer->writeCharacters(user_text);
    }

    writeChildrenToContents(writer, index);
    writer->writeEndElement();
}

void RWBaseItem::writeChildrenToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    QList<RWBaseItem*> child_items = childItems<RWBaseItem*>();
    foreach (RWBaseItem *child, child_items)
    {
        child->writeToContents(writer, index);
    }
}

void RWBaseItem::writeExportTag(QXmlStreamWriter *writer)
{
    writer->writeStartElement("tag_assign");
    writer->writeAttribute("tag_id", "Tag_1");
    writer->writeEndElement();
}


void RWBaseItem::setModelColumnForTag(int column)
{
    //qDebug() << "setModelColumnForTag:" << name() << ":=" << column;
    p_model_column_for_tag = column;
}

void RWBaseItem::setFixedText(const QString &text)
{
    qDebug() << "RWBaseItem::setFixedText:" << text;
    p_fixed_text = text;
}

/**
 * @brief RWBaseItem::canBeGenerated
 * @return true if this element has all the data required for the GENERATE to be a success.
 */
bool RWBaseItem::canBeGenerated() const
{
    QList<RWBaseItem*> list = findChildren<RWBaseItem*>();
    foreach (RWBaseItem *item, list)
    {
        if (!item->canBeGenerated()) return false;
    }
    return true;
}

int RWBaseItem::modelColumnForTag() const
{
    return p_model_column_for_tag;
}

QString RWBaseItem::modelValueForTag(const QModelIndex &index) const
{
    return index.sibling(index.row(), p_model_column_for_tag).data().toString();
}

void RWBaseItem::setModelColumnForText(int column)
{
    //qDebug() << "setModelColumnForText:" << name() << ":=" << column;
    p_model_column_for_text = column;
}

int RWBaseItem::modelColumnForText() const
{
    return p_model_column_for_text;
}

QString RWBaseItem::modelValueForText(const QModelIndex &index) const
{
    if (p_model_column_for_text != -1)
        return index.sibling(index.row(), p_model_column_for_text).data().toString();
    else
        return p_fixed_text;
}

QDebug operator<<(QDebug stream, const RWBaseItem &item)
{
    stream.noquote().nospace() << item.metaObject()->className() << "(" << item.p_element_name;
    if (item.p_global) stream.noquote().nospace() << "_global";
    stream.noquote().nospace() << " : ";
    if (!item.p_name.isEmpty()) stream.noquote().nospace() << item.p_name;
    if (!item.p_id.isEmpty()) stream.noquote().nospace() << ", id=" + item.p_id;
    if (!item.p_uuid.isEmpty()) stream.noquote().nospace() << ", uuid=" + item.p_uuid;
    if (!item.p_signature.isEmpty()) stream.noquote().nospace() << ", signature=" + item.p_signature;
    if (!item.p_fixed_text.isEmpty()) stream.noquote().nospace() << ":: value=\"" + item.p_fixed_text + "\"";
    stream.nospace() << ")";

    return stream;
}




QString RWBaseItem::xmlParagraph(const QString &text)
{
    return "<p class=\"RWDefault\">" + text + "</p>";
}

QString RWBaseItem::xmlSpan(const QString &text, bool bold)
{
    QString style;
    if (bold) style = " style=\"font-weight:bold\">";
    return "<span class=\"RWSnippet\"" + style + ">" + text + "</span>";
}

RWBaseItem *RWBaseItem::childElement(const QString &element_name) const
{
    foreach (QObject *child, children())
    {
        RWBaseItem *item = qobject_cast<RWBaseItem*>(child);
        if (item && item->elementName() == element_name)
            return item;
    }
    return 0;
}
