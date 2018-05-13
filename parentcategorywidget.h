#ifndef PARENTCATEGORYWIDGET_H
#define PARENTCATEGORYWIDGET_H

#include <QFrame>
#include "rw_category.h"

class QComboBox;
class QHBoxLayout;
class QLabel;
class QPushButton;
class RWCategoryWidget;
class RealmWorksStructure;

class ParentCategoryWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ParentCategoryWidget(RealmWorksStructure *structure, QAbstractItemModel *columns, int indent, QWidget *parent = nullptr);
    RWCategory *category() const;

signals:
    void deleteRequested();

public slots:
    void setCanDelete(bool);

private slots:
    void select_category(const QString &selection);
private:
    RealmWorksStructure *structure;
    QComboBox   *combo;
    QHBoxLayout *category_area;
    QPushButton *delete_button;
    RWCategoryWidget *category_widget;
    QAbstractItemModel *header_model;
};

#endif // PARENTCATEGORYWIDGET_H
