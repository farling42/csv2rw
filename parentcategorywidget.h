#ifndef PARENTCATEGORYWIDGET_H
#define PARENTCATEGORYWIDGET_H

#include <QFrame>
#include "rw_topic.h"

class QComboBox;
class QHBoxLayout;
class QLabel;
class QPushButton;
class RWTopicWidget;
class RealmWorksStructure;

class ParentCategoryWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ParentCategoryWidget(RealmWorksStructure *structure, QAbstractItemModel *columns, int indent, QWidget *parent = nullptr);
    RWTopic *topic() const;

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
    RWTopicWidget *category_widget;
    QAbstractItemModel *header_model;
};

#endif // PARENTCATEGORYWIDGET_H
