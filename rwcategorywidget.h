#ifndef RWCATEGORYWIDGET_H
#define RWCATEGORYWIDGET_H

#include <QFrame>

class RWCategory;
class RWPartition;
class QAbstractItemModel;

class RWCategoryWidget : public QFrame
{
    Q_OBJECT
    QWidget *add_partition(QList<int> sections, QAbstractItemModel *columns, RWPartition *partition);
public:
    explicit RWCategoryWidget(RWCategory *category, QAbstractItemModel *columns, QWidget *parent = 0);
    RWCategory *category() const { return p_category; }
signals:

public slots:

private slots:
    void do_insert();

private:
    RWCategory *p_category;
};

#endif // RWCATEGORYWIDGET_H
