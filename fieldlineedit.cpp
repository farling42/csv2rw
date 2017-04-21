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

FieldLineEdit::FieldLineEdit(QWidget *parent) :
    QLineEdit(parent),
    p_mode(Mode_Index)  // we are going to call setMode
{
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setMode(Mode_Empty);
    connect (this, &QLineEdit::textEdited, this, &FieldLineEdit::text_changed);
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
            setText(map.value(Qt::DisplayRole).toString());
            setMode(Mode_Index);
            //qDebug() << "dropEvent for row" << row << ", col" << col << ":=" << text();
            // Drag from list of column names, so transpose row number to equate to column number
            emit modelColumnSelected(row);
        }
    }
}

void FieldLineEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        setText(QString());
        setMode(Mode_Empty);
        emit modelColumnSelected(-1);
    }
#if 0
    qDebug() << "FieldLineEdit::mousePressEvent:" << event;
    Q_UNUSED(event)
    if (text().isEmpty()) return;

    QMimeData *data = new QMimeData;
    data->setText(text());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(data);
    Qt::DropAction drop_action = drag->exec(Qt::MoveAction);
    qDebug() << "FieldLineEdit::mousePressEvent: drag result =" << drop_action;
    if (drop_action == Qt::MoveAction)
    {
        setText("");
    }
#endif
}

void FieldLineEdit::setMode(FieldLineEdit::DataMode mode)
{
    if (p_mode == mode) return;
    p_mode = mode;
    setReadOnly(p_mode == Mode_Index);
    setClearButtonEnabled(p_mode == Mode_Fixed);
}


void FieldLineEdit::text_changed(const QString &value)
{
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
    emit fixedTextChanged(value);
}
