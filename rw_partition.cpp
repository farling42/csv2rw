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
        const QString user_text = modelValueForText(index);
        if (!user_text.isEmpty())
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
                    writer->writeCharacters(xmlParagraph(xmlSpan(user_text, bold)));
                }
                writer->writeEndElement(); // contents

                // </snippet>
            }
            writer->writeEndElement(); // snippet
        }
    }

    // And it may have sub-sections, after the snippet/content (if any)
    QList<RWPartition*> child_partitions = childItems<RWPartition*>();
    foreach (RWPartition *partition, child_partitions)
    {
        partition->writeToContents(writer, index);
    }

    writer->writeEndElement();  //section
}
