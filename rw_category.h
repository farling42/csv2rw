#ifndef RW_CATEGORY_H
#define RW_CATEGORY_H

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

#include "rw_base_item.h"

class QXmlStreamWriter;
class RWPartition;

class RWCategory : public RWBaseItem
{
    Q_OBJECT
    Q_PROPERTY(int nameColumn   READ modelColumnForName   WRITE setModelColumnForName)
    Q_PROPERTY(int prefixColumn READ modelColumnForPrefix WRITE setModelColumnForPrefix)
    Q_PROPERTY(int suffixColumn READ modelColumnForSuffix WRITE setModelColumnForSuffix)

public:
    RWCategory(QXmlStreamReader *stream, QObject *parent = 0);

    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);
    virtual void writeParentStartToContents(QXmlStreamWriter*,
                                            bool revealed,
                                            const QString &title,
                                            const QString &prefix,
                                            const QString &suffix);
    virtual void postLoad();

    virtual bool canBeGenerated() const;

public Q_SLOTS:
    void setModelColumnForName(int column);
    void setModelColumnForPrefix(int column);
    void setModelColumnForSuffix(int column);

public:
    int  modelColumnForName() const;
    QString modelValueForName(const QModelIndex &index) const;

    int  modelColumnForPrefix() const;
    QString modelValueForPrefix(const QModelIndex &index) const;

    int  modelColumnForSuffix() const;
    QString modelValueForSuffix(const QModelIndex &index) const;

private:
    int p_model_column_for_name;
    int p_model_column_for_prefix;
    int p_model_column_for_suffix;
};

#endif // RWCATEGORY_H
