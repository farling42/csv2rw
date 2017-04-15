#include "rw_partition.h"
#include "rw_facet.h"
#include <QXmlStreamWriter>

RWPartition::RWPartition(QXmlStreamReader *stream, QObject *parent) :
    RWBaseItem(stream, parent)
{

}

void RWPartition::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    writer->writeStartElement("section");
    writer->writeAttribute("partition_id", id());

    // writeChildren has to be done in 2 passes
    //writeChildrenToContents(writer, index);

    // A section has 1+ snippets (which come before the contents (if any)
    QList<RWFacet*> child_facets = childItems<RWFacet*>();
    foreach (RWFacet *facet, child_facets)
    {
        facet->writeToContents(writer, index);
    }

    // It may have some text directly on it, not stored in a facet
    if (modelColumnForText() >= 0)
    {
        bool bold = false;

        writer->writeStartElement("snippet");
        {
            writer->writeAttribute("type", "Multi_Line");
            // no facet_id
            if (isRevealed()) writer->writeAttribute("is_revealed", "true");

            writer->writeStartElement("contents");
            {
                // Maybe the snippet has some contents
                writer->writeCharacters(xmlParagraphStart());
                writer->writeCharacters(xmlSpanStart(bold));
                writer->writeCharacters(modelValueForText(index));
                writer->writeCharacters(xmlSpanFinish() + xmlParagraphFinish());
            }
            writer->writeEndElement(); // contents

            // </snippet>
        }
        writer->writeEndElement(); // snippet
    }

    // And it may have sub-sections, after the snippet/content (if any)
    QList<RWPartition*> child_partitions = childItems<RWPartition*>();
    foreach (RWPartition *partition, child_partitions)
    {
        partition->writeToContents(writer, index);
    }

    writer->writeEndElement();  //section
}
