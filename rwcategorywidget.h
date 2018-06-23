#ifndef RWCATEGORYWIDGET_H
#define RWCATEGORYWIDGET_H

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

#include <QFrame>

class RWAlias;
class RWBaseItem;
class RWCategory;
class RWFacet;
class RWPartition;
class QAbstractItemModel;
class QActionGroup;
class QComboBox;
class QMenu;

class RWCategoryWidget : public QFrame
{
    Q_OBJECT
    QWidget *add_partition(QList<int> sections, QAbstractItemModel *columns, RWPartition *partition);
    QWidget *add_facet(QAbstractItemModel *columns, RWFacet *facet);

public:
    explicit RWCategoryWidget(RWCategory *category, QAbstractItemModel *columns, bool include_sections = true, QWidget *parent = 0);
    RWCategory *category() const { return p_category; }
signals:

public slots:

private slots:
    void do_insert();
    void add_name();
    void remove_name();

private:
    QAbstractItemModel *p_columns;
    RWCategory *p_category;
    QWidget *p_first_section;
    void add_rwalias(RWAlias *alias);
    QWidget *create_option_button(RWBaseItem *Item);
    template<typename T>
    QActionGroup *create_enum_actions(const QString &section_name, T current_value, QMenu *menu, QMap<QString,QString> &rename);
};

#endif // RWCATEGORYWIDGET_H
