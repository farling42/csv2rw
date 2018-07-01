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
#include "rw_section.h"
#include <QXmlStreamWriter>
#include <QMetaEnum>


static QMetaEnum snip_style_enum = QMetaEnum::fromType<RWContentsItem::SnippetStyle>();

RWPartition::RWPartition(QXmlStreamReader *stream, QObject *parent) :
    RWStructureItem(stream, parent)
{
}

RWContentsItem *RWPartition::createContentsItem(RWContentsItem *parent)
{
    return new RWSection(this, parent);
}
