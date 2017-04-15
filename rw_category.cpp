#include "rw_category.h"
#include "rw_partition.h"
#include <QXmlStreamWriter>
#include <QDebug>


RWCategory::RWCategory(QXmlStreamReader *stream, QObject *parent) :
    RWBaseItem(stream, parent)
{
}

void RWCategory::postLoad()
{
    // Mark any child Description or Summary child elements as NOT for output in CONTENTS
    QList<RWBaseItem*> child_items = childItems<RWBaseItem*>();
    foreach (RWBaseItem *child, child_items)
    {
        if (child->elementName() == "description" || child->elementName() == "summary")
        {
            child->setIgnoreForContents(true);
        }
    }
}

void RWCategory::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    static int topic_id = 1;
    writer->writeStartElement("topic");
    {
        writer->writeAttribute("topic_id", QString("topic_%1").arg(topic_id++));
        writer->writeAttribute("category_id", id());
        writer->writeAttribute("public_name", modelValueForName(index));
        if (modelColumnForPrefix() >= 0) writer->writeAttribute("prefix", modelValueForPrefix(index));
        if (modelColumnForSuffix() >= 0) writer->writeAttribute("suffix", modelValueForSuffix(index));
        //if (isRevealed()) writer->writeAttribute("is_revealed", "true");

        writeChildrenToContents(writer, index);

        // Relevant export tag on every topic
        writer->writeStartElement("tag_assign");
        writer->writeAttribute("tag_id", "Tag_1");
        writer->writeEndElement();

        // No actual TEXT for this element (only children)
        //if (modelValueForText(index) >= 0) writer->writeCharacters(modelValueForText(index));
    }
    writer->writeEndElement();  // </topic>
}
