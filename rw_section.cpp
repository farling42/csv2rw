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
#include "rw_section.h"
#include "rw_snippet.h"
#include <QXmlStreamWriter>
#include <QMetaEnum>


static QMetaEnum snip_style_enum = QMetaEnum::fromType<RWContentsItem::SnippetStyle>();

RWSection::RWSection(RWPartition *partition, RWContentsItem *parent) :
    RWContentsItem(partition, parent),
    partition(partition)
{
}

void RWSection::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    writer->writeStartElement("section");
    if (!structure->id().isEmpty()) writer->writeAttribute("partition_id", structure->id());

    // writeChildren has to be done in 2 passes
    //writeChildrenToContents(writer, index);

    // A section has 1+ snippets (which come before the contents (if any)
    for (auto snippet : childItems<::RWSnippet*>())
    {
        snippet->writeToContents(writer, index);
    }

    // It may have some text directly on it, not stored in a facet
    const QString user_text = contentsText().valueString(index);
    if (!user_text.isEmpty())
    {
        bool bold = false;

        writer->writeStartElement("snippet");
        {
            writer->writeAttribute("type", "Multi_Line");
            // no facet_id
            if (p_snippet_style != RWContentsItem::Normal) writer->writeAttribute("style", snip_style_enum.valueToKey(p_snippet_style));
            if (isRevealed()) writer->writeAttribute("is_revealed", "true");
            QString text;
            foreach (const QString &para, user_text.split("\n\n"))
                text.append(xmlParagraph(xmlSpan(para, bold)));
            writer->writeTextElement("contents", text);
        }
        writer->writeEndElement(); // snippet
    }

    // And it may have sub-sections, after the snippet/content (if any)
    QList<RWSection*> child_partitions = childItems<RWSection*>();
    foreach (RWSection *partition, child_partitions)
    {
        partition->writeToContents(writer, index);
    }

    writer->writeEndElement();  //section
}
