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

class RWCategory;
class RWFacet;
class RWPartition;
class QAbstractItemModel;

class RWCategoryWidget : public QFrame
{
    Q_OBJECT
    QWidget *add_partition(QList<int> sections, QAbstractItemModel *columns, RWPartition *partition);
    QWidget *add_facet(QAbstractItemModel *columns, RWFacet *facet);

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
