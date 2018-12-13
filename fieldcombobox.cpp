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

#include "fieldcombobox.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QLineEdit>
#include <QMimeData>
#include "rw_domain.h"

FieldComboBox::FieldComboBox(DataField &datafield, RWDomain *domain, QWidget *parent) :
    QComboBox(parent),
    p_domain(domain),
    p_data(datafield)
{
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setToolTip(domain ? "Tag Domain: " + domain->name() : "Domain");
    setInsertPolicy(NoInsert);
    set_domain_list();
    connect(this, &FieldComboBox::currentTextChanged, this, &FieldComboBox::text_changed);
}


void FieldComboBox::set_domain_list()
{
    QStringList items;
    if (p_domain) items = p_domain->tagNames();
    items.prepend(QString());

    clear();
    addItems(items);
    setCurrentIndex(0);
}

void FieldComboBox::setIndexString(const QString &value)
{
    clear();
    addItem(value);
    setCurrentIndex(0);
}

void FieldComboBox::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
    {
        event->acceptProposedAction();
    }
}

void FieldComboBox::dragMoveEvent(QDragMoveEvent *event)
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
            p_data.setModelColumn(row);
            setIndexString(map.value(Qt::DisplayRole).toString());
            //qDebug() << "dropEvent for row" << row << ", col" << col << ":=" << currentText();
            // Drag from list of column names, so transpose row number to equate to column number
        }
    }
}

void FieldComboBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        if (p_data.modelColumn() >= 0)
        {
            p_data.setModelColumn(-1);
            clearEditText();
            set_domain_list();
        }
    }
    else
        QComboBox::mousePressEvent(event);
}


void FieldComboBox::text_changed(const QString &value)
{
    if (p_data.modelColumn() < 0)
    {
        p_data.setFixedText(value);
    }
}
