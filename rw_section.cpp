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

void RWSection::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index) const
{
    if (p_is_multiple && p_first_multiple.modelColumn() >= 0)
    {
        if (p_second_multiple.modelColumn() == -1)
        {
            // Custom name for the section
            write_one(writer, "name", p_first_multiple.valueString(index).left(50), index);
        }
        else
        {
            // Ensure last column is valid
            int first_column  = p_first_multiple.modelColumn();
            int second_column = p_second_multiple.modelColumn();
            int last_column   = p_last_multiple.modelColumn();
            if (last_column == -1) last_column = index.model()->columnCount();
            // Handle multiple repetitions of the same section
            int step = second_column - first_column;
            for (int column = first_column; column <= last_column; column += step)
            {
                DataField::setColumnOffset(column - first_column);
                QString name = index.sibling(index.row(), column).data().toString();
                if (name.isEmpty()) break;
                // Name is an xs:token of 1-50 characters
                write_one(writer, "name", name.left(50), index);
            }
            DataField::setColumnOffset(0);
        }
    }
    else
    {
        write_one(writer, "partition_id", structure->id(), index);
    }
}

void RWSection::write_one(QXmlStreamWriter *writer, const QString &attr_name, const QString &attr_value, const QModelIndex &index) const
{
    writer->writeStartElement("section");
    if (!structure->id().isEmpty()) writer->writeAttribute(attr_name, attr_value);
    if (p_start_collapsed) writer->writeAttribute("is_collapsed_by_default", "true");

    // writeChildren has to be done in 2 passes
    //writeChildrenToContents(writer, index);

    // A section has 1+ snippets (which come before the contents (if any)
    for (auto snippet: childItems<::RWSnippet*>())
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
            for (auto para: user_text.split("\n\n"))
                text.append(xmlParagraph(xmlSpan(para, bold)));
            writer->writeTextElement("contents", text);
        }
        writer->writeEndElement(); // snippet
    }

    // And it may have sub-sections, after the snippet/content (if any)
    for (auto section: childItems<RWSection*>())
    {
        section->writeToContents(writer, index);
    }

    writer->writeEndElement();  //section
}
