#include "fieldlineedit.h"

#include <QDebug>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMouseEvent>

FieldLineEdit::FieldLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setAcceptDrops(true);
    setReadOnly(true);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void FieldLineEdit::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
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
