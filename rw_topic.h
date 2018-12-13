#ifndef RW_TOPIC_H
#define RW_TOPIC_H

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
#include "rw_alias.h"

class QDataStream;
class QXmlStreamWriter;
class RWAlias;
class RWCategory;
class RWRelationship;
class RWPartition;

class RWTopic : public RWContentsItem
{
    Q_OBJECT

public:
    RWTopic(RWCategory *item, RWContentsItem *parent);

    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index, bool use_index_topic_id) const;
    virtual void writeStartToContents(QXmlStreamWriter*, const QModelIndex &index, bool use_index_topic_id) const;

    virtual bool canBeGenerated() const;

    QList<RWAlias*> aliases;
    const RWCategory *const category;

    QList<RWTopic*> parents;
    QList<RWRelationship*> relationships;

public:
    RWAlias &publicName() { return p_public_name; }
    DataField &prefix()  { return p_prefix; }
    DataField &suffix()  { return p_suffix; }

    static void setDefaultName(const QString &name);
    static void initBeforeExport(int model_row_count);
    int keyColumn() const { return p_key_column; }
    QString keyValue() const { return p_key_column >= 0 ? p_key_value : QString(); }

signals:
    void duplicateTopicInOutput(const QString &public_name) const;

public slots:
    void setKeyColumn(int column) { p_key_column = column; }
    void setKeyValue(const QString &value) { p_key_value = value; }

private:
    RWAlias p_public_name;
    DataField p_prefix;
    DataField p_suffix;
    int p_key_column;
    QString p_key_value;
    friend QDataStream& operator<<(QDataStream&, const RWTopic&);
    friend QDataStream& operator>>(QDataStream&, RWTopic&);
};

extern QDataStream& operator<<(QDataStream&, const RWTopic&);
extern QDataStream& operator>>(QDataStream&, RWTopic&);

#endif // RW_TOPIC_H
