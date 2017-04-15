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
