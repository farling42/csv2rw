#ifndef RW_TOPIC_H
#define RW_TOPIC_H

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

class QXmlStreamWriter;
class RWAlias;
class RWCategory;
class RWPartition;

class RWTopic : public RWContentsItem
{
    Q_OBJECT

public:
    RWTopic(RWCategory *item, RWContentsItem *parent);

    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);
    virtual void writeStartToContents(QXmlStreamWriter*, const QModelIndex &index);

    virtual bool canBeGenerated() const;

    QList<RWAlias*> aliases;
    const RWCategory *const category;

public:
    DataField &namefield()   { return p_name; }
    DataField &prefix() { return p_prefix; }
    DataField &suffix() { return p_suffix; }

    static void setDefaultName(const QString &name);

private:
    DataField p_name;
    DataField p_prefix;
    DataField p_suffix;
};

#endif // RW_TOPIC_H
