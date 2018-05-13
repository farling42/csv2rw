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

#include "fieldmultilineedit.h"

#include <QDebug>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMouseEvent>

FieldMultiLineEdit::FieldMultiLineEdit(DataField &datafield, QWidget *parent) :
    QTextEdit(parent),
    p_mode(Mode_Index),  // we are going to call setMode
    p_data(datafield)
{
    //setAcceptDrops(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setMode(Mode_Empty);
    // We need the field to start at 1 line high,
    // and stretch in height as more text is added.
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizeAdjustPolicy(AdjustToContents);
    QFontMetrics metrics=fontMetrics();
    setMinimumHeight(metrics.height() + contentsMargins().top() + contentsMargins().bottom() + 2 * document()->documentMargin());
    //setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    connect(this, &QTextEdit::textChanged, this, &FieldMultiLineEdit::text_changed);
}


void FieldMultiLineEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        p_data.setModelColumn(-1);
        setText(QString());
        setMode(Mode_Empty);
    }
    else
        QTextEdit::mousePressEvent(event);
}

bool FieldMultiLineEdit::canInsertFromMimeData(const QMimeData *source) const
{
    if (source->hasFormat("application/x-qabstractitemmodeldatalist"))
        return p_mode != Mode_Fixed;
    else
        return QTextEdit::canInsertFromMimeData(source);
}


void FieldMultiLineEdit::insertFromMimeData(const QMimeData *source)
{
    QByteArray encoded(source->data("application/x-qabstractitemmodeldatalist"));
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    while (!stream.atEnd())
    {
        int row, col;
        QMap<int,QVariant> map;
        stream >> row >> col >> map;
        if (map.contains(Qt::DisplayRole))
        {
            p_data.setModelColumn(row);
            setMode(Mode_Index);    // must be done before the textChanged signal is issued.
            setText(map.value(Qt::DisplayRole).toString());
            //qDebug() << "dropEvent for row" << row << ", col" << col << ":=" << text();
            // Drag from list of column names, so transpose row number to equate to column number
        }
    }
}


void FieldMultiLineEdit::setMode(DataMode mode)
{
    if (p_mode == mode) return;
    p_mode = mode;
    setReadOnly(p_mode == Mode_Index);
    //setClearButtonEnabled(p_mode == Mode_Fixed);
}


void FieldMultiLineEdit::text_changed()
{
    //qDebug() << "FieldMultiLineEdit::text_changed:" << toHtml();
    switch (p_mode)
    {
    case Mode_Index:
        return;
    case Mode_Empty:
        if (!document()->isEmpty()) setMode(Mode_Fixed);
        break;
    case Mode_Fixed:
        if (document()->isEmpty()) setMode(Mode_Empty);
        break;
    }
    p_data.setFixedText(toPlainText());     // TODO: allow proper formatted text
}
