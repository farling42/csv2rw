#ifndef FIELDLINEEDIT_H
#define FIELDLINEEDIT_H

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

#include <QLineEdit>
#include "datafield.h"

class FieldLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    FieldLineEdit(DataField &datafield, QWidget *parent = 0);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);

private slots:
    void text_changed(const QString &value);

private:
    enum DataMode { Mode_Empty, Mode_Index, Mode_Fixed };
    DataMode   p_mode;
    DataField &p_data;
    void setMode(DataMode);
};

#endif // FIELDLINEEDIT_H
