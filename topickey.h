#ifndef TOPICKEY_H
#define TOPICKEY_H

/*
RWImporter
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

#include <QDialog>

namespace Ui {
class TopicKey;
}

class QAbstractItemModel;

class TopicKey : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int selectedColumn READ selectedColumn WRITE setSelectedColumn)
    Q_PROPERTY(QString selectedValue READ selectedValue WRITE setSelectedValue)

public:
    explicit TopicKey(QWidget *parent = nullptr);
    ~TopicKey();

public:
    static void setModel(QAbstractItemModel *model);
    int selectedColumn() const;
    QString selectedValue() const;

public slots:
    void setSelectedColumn(int);
    void setSelectedValue(const QString &value);

private slots:
    void on_csvColumn_currentIndexChanged(int column);

private:
    Ui::TopicKey *ui;
};

#endif // TOPICKEY_H
