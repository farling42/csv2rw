#ifndef HTMLITEMDELEGATE_H
#define HTMLITEMDELEGATE_H

#include <QStyledItemDelegate>

class HtmlItemDelegate : public QStyledItemDelegate
{
public:
    HtmlItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // HTMLITEMDELEGATE_H
