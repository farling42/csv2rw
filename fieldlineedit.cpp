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

#include "fieldlineedit.h"

#include <QDebug>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMouseEvent>

FieldLineEdit::FieldLineEdit(DataField &datafield, QWidget *parent) :
    QLineEdit(parent),
    p_mode(Mode_Index),  // we are going to call setMode
    p_data(datafield)
{
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setMode(Mode_Empty);
    connect(this, &QLineEdit::textEdited, this, &FieldLineEdit::text_changed);
}

void FieldLineEdit::dragEnterEvent(QDragEnterEvent *event)
{
    switch (p_mode)
    {
    case Mode_Fixed:
        QLineEdit::dragEnterEvent(event);
        break;

    case Mode_Empty:
    case Mode_Index:
        event->acceptProposedAction();
        break;
    }
}

void FieldLineEdit::dropEvent(QDropEvent *event)
{
    QByteArray encoded(event->mimeData()->data("application/x-qabstractitemmodeldatalist"));
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    while (!stream.atEnd())
    {
        int row, col;
        QMap<int,QVariant> map;
        stream >> row >> col >> map;
        if (map.contains(Qt::DisplayRole))
        {
            p_data.setModelColumn(row);
            setText(map.value(Qt::DisplayRole).toString());
            setMode(Mode_Index);
            //qDebug() << "dropEvent for row" << row << ", col" << col << ":=" << text();
            // Drag from list of column names, so transpose row number to equate to column number
        }
    }
}

void FieldLineEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        p_data.setModelColumn(-1);
        setText(QString());
        setMode(Mode_Empty);
    }
    else
        QLineEdit::mousePressEvent(event);
}

void FieldLineEdit::setMode(DataMode mode)
{
    if (p_mode == mode) return;
    p_mode = mode;
    setReadOnly(p_mode == Mode_Index);
    setClearButtonEnabled(p_mode == Mode_Fixed);
}


void FieldLineEdit::text_changed(const QString &value)
{
    //qDebug() << "FieldLineEdit::text_changed:" << value;
    switch (p_mode)
    {
    case Mode_Index:
        return;
    case Mode_Empty:
        if (!value.isEmpty()) setMode(Mode_Fixed);
        break;
    case Mode_Fixed:
        if (value.isEmpty()) setMode(Mode_Empty);
        break;
    }
    p_data.setFixedText(value);
}
