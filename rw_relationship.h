#ifndef RW_RELATIONSHIP_H
#define RW_RELATIONSHIP_H

/*
RWImporter
Copyright (C) 2018 Martin Smith

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

#include <QObject>
#include "datafield.h"

class QXmlStreamWriter;

class RWRelationship : public QObject
{
    Q_OBJECT
public:
    explicit RWRelationship(QObject *parent = nullptr);

    enum Nature { Arbitrary, Generic, Union, Parent_To_Offspring,
                  //Offspring_To_Parent,   // RW import only creates with Parent_To_Offspring
                  Master_To_Minion,
                  //Minion_To_Master,       // RW import only creates with Master_To_Minion
                  Public_Attitude_Towards, Private_Attitude_Towards };
    enum Attitude { Hostile, Angry, Annoyed, Neutral, Pleased, Friendly, Gracious };
    Q_ENUM(Nature)
    Q_ENUM(Attitude)

    DataField &thisLink()   { return p_this_link; }
    DataField &otherLink()  { return p_other_link; }
    bool is_revealed{false};
    Nature   nature{Arbitrary};
    Attitude attitude;
    QString qualifier_tag_name;

    void writeToContents(QXmlStreamWriter*, const QModelIndex &index) const;

signals:

public slots:

private:
    DataField p_this_link;   // The column in THIS row containing the value to match
    DataField p_other_link;  // The column in THE MODEL in which the match should be found
    friend QDataStream& operator<<(QDataStream&, const RWRelationship&);
    friend QDataStream& operator>>(QDataStream&, RWRelationship&);
};

extern QDataStream& operator<<(QDataStream&, const RWRelationship&);
extern QDataStream& operator>>(QDataStream&, RWRelationship&);

#endif // RW_CONNECTION_H
