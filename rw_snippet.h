#ifndef RW_SNIPPET_H
#define RW_SNIPPET_H

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

#include "rw_contents_item.h"

class QXmlStreamWriter;
class RWFacet;

class RWSnippet : public RWContentsItem
{
    Q_OBJECT

public:
    RWSnippet(RWFacet *item, RWContentsItem *parent);
    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index) const;

    DataField &tags()      { return p_tags; }
    DataField &labelText() { return p_label_text; }
    DataField &filename() { return p_filename; }
    DataField &startDate() { return p_start_date; }
    DataField &finishDate() { return p_finish_date; }
    DataField &number() { return p_number; }

    const DataField &tags()      const { return p_tags; }
    const DataField &labelText() const { return p_label_text; }
    const DataField &filename() const { return p_filename; }
    const DataField &startDate() const { return p_start_date; }
    const DataField &finishDate() const { return p_finish_date; }
    const DataField &number() const { return p_number; }

    const RWFacet *const facet;

public slots:

private:
    void write_asset(QXmlStreamWriter *writer, const QString &filename) const;
    void write_ext_object(QXmlStreamWriter *writer, const QString &exttype, const QString &filename) const;
    void write_smart_image(QXmlStreamWriter *writer, const QString &filename) const;
    DataField p_tags;
    DataField p_label_text;   // for Labeled_Text fields
    DataField p_filename;
    DataField p_start_date;
    DataField p_finish_date;
    DataField p_number;
    friend QDataStream& operator<<(QDataStream&,const RWSnippet&);
    friend QDataStream& operator>>(QDataStream&,RWSnippet&);
};

extern QDataStream& operator<<(QDataStream&,const RWSnippet&);
extern QDataStream& operator>>(QDataStream&,RWSnippet&);

#endif // RW_SNIPPET_H
