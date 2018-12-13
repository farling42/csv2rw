/*
RWImporter
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

#include "rw_alias.h"
#include "rw_category.h"
#include "rw_partition.h"
#include "rw_topic.h"
#include <QXmlStreamWriter>
#include <QMetaEnum>
#include <QModelIndex>
#include <QDebug>

#include "rw_contents_item.h"   // TODO - remove this?

RWCategory::RWCategory(QXmlStreamReader *stream, QObject *parent) :
    RWStructureItem(stream, parent)
{
}

void RWCategory::postLoad()
{
    // Mark any child Description or Summary child elements as NOT for output in CONTENTS
    for (auto child: childItems<RWStructureItem*>())
    {
        if (child->structureElement() == "description" || child->structureElement() == "summary")
        {
            child->setIgnoreForContents(true);
        }
    }
}

RWContentsItem *RWCategory::createContentsItem(RWContentsItem *parent)
{
    return new RWTopic(this, parent);
}
