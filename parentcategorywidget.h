#ifndef PARENTCATEGORYWIDGET_H
#define PARENTCATEGORYWIDGET_H

/*
CSV2RW
Copyright (C) 2018 Martin Smith

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
    explicit ParentCategoryWidget(RealmWorksStructure *structure, QAbstractItemModel *columns, int indent, RWTopic *topic, QWidget *parent = nullptr);
    RWTopic *topic() const;

    void setTopic(RWTopic *topic);
signals:
    void deleteRequested();
    void categoryChanged();

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
    bool ignore_select;
};

#endif // PARENTCATEGORYWIDGET_H
