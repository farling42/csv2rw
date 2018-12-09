#ifndef RW_SECTION_H
#define RW_SECTION_H

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
class RWFacet;
class RWPartition;

class RWSection : public RWContentsItem
{
    Q_OBJECT
public:
    RWSection(RWPartition *partition, RWContentsItem *parent);
    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index) const;
    const RWPartition *const partition;

    DataField &firstMultiple() { return p_first_multiple; }
    const DataField &firstMultiple() const { return p_first_multiple; }

    DataField &secondMultiple() { return p_second_multiple; }
    const DataField &secondMultiple() const { return p_second_multiple; }

    DataField &lastMultiple() { return p_last_multiple; }
    const DataField &lastMultiple() const { return p_last_multiple; }

    DataField &lastContents() { return p_last_contents; }
    const DataField &lastContents() const { return p_last_contents; }

//protected:
    bool p_start_collapsed{false};
    bool p_is_multiple{false};
private:
    void write_one(QXmlStreamWriter*, const QString &attr_name, const QString &attr_value, const QModelIndex &index) const;
    void write_text(QXmlStreamWriter *writer, const QString &user_text, const QString &gm_dir) const;
    DataField p_first_multiple;
    DataField p_second_multiple;
    DataField p_last_multiple;
    DataField p_last_contents;
    friend QDataStream& operator<<(QDataStream&,const RWSection&);
    friend QDataStream& operator>>(QDataStream&,RWSection&);
};

extern QDataStream& operator<<(QDataStream&,const RWSection&);
extern QDataStream& operator>>(QDataStream&,RWSection&);

#endif // RW_SECTION_H
