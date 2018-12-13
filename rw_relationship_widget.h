#ifndef RW_RELATIONSHIP_WIDGET_H
#define RW_RELATIONSHIP_WIDGET_H

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

#include <QWidget>
#include "datafield.h"
#include "rw_relationship.h"

class RWRelationship;
class FieldLineEdit;
class QComboBox;
class QPushButton;
class QRadioButton;

class RWRelationshipWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RWRelationshipWidget(RWRelationship *relationship, QAbstractItemModel *columns, QWidget *parent = nullptr);
    //DataField &thisLink()  { return this_link; }
    //DataField &otherLink()  { return other_link; }
    RWRelationship *relationship() { return p_relationship; }

signals:
    void deleteRequested();

public slots:
    void setNature(const QString &reason);
    void setQualifier(const QString &qualifier);

private:
    RWRelationship *p_relationship;
    DataField p_linkedTopic;
    QRadioButton *reveal;
    QComboBox *nature;
    QComboBox *qualifier;
    FieldLineEdit *this_link;
    FieldLineEdit *other_link;
    QPushButton *remove;
};

#endif // RW_CONNECTION_WIDGET_H
