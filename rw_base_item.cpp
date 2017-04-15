#include "rw_base_item.h"
#include <QXmlStreamWriter>
#include <QModelIndex>
#include <QDebug>

#undef DEBUG_XML

RWBaseItem::RWBaseItem(QXmlStreamReader *reader, QObject *parent, bool ignore_for_contents) :
    QObject(parent),
    p_gm_only(false),
    p_model_column_for_name(-1), p_model_column_for_prefix(-1),
    p_model_column_for_suffix(-1), p_model_column_for_text(-1),
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
    if (!p_text.isEmpty()) writer->writeCharacters(p_text);
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

    writer->writeAttribute(p_element_name + "_id", id());   // e.g. partition_id, not the same as <element>_id
    writer->writeAttribute("name", modelColumnForName() >= 0 ? modelValueForName(index) : name());

    if (modelColumnForText() >= 0)
        writer->writeCharacters(modelValueForText(index));
//    else if (!p_text.isEmpty())
//        writer->writeCharacters(p_text);

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


void RWBaseItem::setModelColumnForName(int column)
{
    qDebug() << "setModelColumnForName:" << name() << ":=" << column;
    p_model_column_for_name = column;
}

int RWBaseItem::modelColumnForName() const
{
    return p_model_column_for_name;
}

QString RWBaseItem::modelValueForName(const QModelIndex &index) const
{
    return index.sibling(index.row(), p_model_column_for_name).data().toString();
}

void RWBaseItem::setModelColumnForPrefix(int column)
{
    qDebug() << "setModelColumnForPrefix:" << name() << ":=" << column;
    p_model_column_for_prefix = column;
}

int RWBaseItem::modelColumnForPrefix() const
{
    return p_model_column_for_prefix;
}

QString RWBaseItem::modelValueForPrefix(const QModelIndex &index) const
{
    return index.sibling(index.row(), p_model_column_for_prefix).data().toString();
}

void RWBaseItem::setModelColumnForSuffix(int column)
{
    qDebug() << "setModelColumnForSuffix:" << name() << ":=" << column;
    p_model_column_for_suffix = column;
}

int RWBaseItem::modelColumnForSuffix() const
{
    return p_model_column_for_suffix;
}

QString RWBaseItem::modelValueForSuffix(const QModelIndex &index) const
{
    return index.sibling(index.row(), p_model_column_for_suffix).data().toString();
}

void RWBaseItem::setModelColumnForTag(int column)
{
    qDebug() << "setModelColumnForTag:" << name() << ":=" << column;
    p_model_column_for_tag = column;
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
    qDebug() << "setModelColumnForText:" << name() << ":=" << column;
    p_model_column_for_text = column;
}

int RWBaseItem::modelColumnForText() const
{
    return p_model_column_for_text;
}

QString RWBaseItem::modelValueForText(const QModelIndex &index) const
{
    return index.sibling(index.row(), p_model_column_for_text).data().toString();
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
    if (!item.p_text.isEmpty()) stream.noquote().nospace() << ":: value=\"" + item.p_text + "\"";
    stream.nospace() << ")";

    return stream;
}




QString RWBaseItem::xmlParagraphStart()
{
    return "<p class=\"RWDefault\">";
}

QString RWBaseItem::xmlParagraphFinish()
{
    return "</p>";
}

QString RWBaseItem::xmlSpanStart(bool bold)
{
    if (bold)
        return "<span class=\"RWSnippet\" style=\"font-weight:bold\">";
    else
        return "<span class=\"RWSnippet\">";
}

QString RWBaseItem::xmlSpanFinish()
{
    return "</span>";
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
