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

#include "fieldcombobox.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>

FieldComboBox::FieldComboBox(QWidget *parent) : QComboBox(parent)
{
    setAcceptDrops(true);
    setEditable(false);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void FieldComboBox::setValue(const QString &value)
{
    clear();
    addItem(value);
    setCurrentIndex(0);
}

void FieldComboBox::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void FieldComboBox::dropEvent(QDropEvent *event)
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
            setValue(map.value(Qt::DisplayRole).toString());
            //qDebug() << "dropEvent for row" << row << ", col" << col << ":=" << text();
            // Drag from list of column names, so transpose row number to equate to column number
            emit modelColumnSelected(row);
        }
    }
}

void FieldComboBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        setValue(QString());
        emit modelColumnSelected(-1);
    }
}
