#ifndef FIELDCOMBOBOX_H
#define FIELDCOMBOBOX_H

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

#include <QComboBox>
#include "datafield.h"
class RWDomain;

class FieldComboBox : public QComboBox
{
    Q_OBJECT
public:
    FieldComboBox(DataField &datafield, RWDomain *domain, QWidget *parent = nullptr);

public Q_SLOTS:
    void setIndexString(const QString&);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
private slots:
    void text_changed(const QString &value);
private:
    RWDomain *p_domain;
    DataField &p_data;
    void set_domain_list();
};


#endif // FIELDCOMBOBOX_H
