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

#include "topickey.h"
#include "ui_topickey.h"

#include <QAbstractItemModel>
#include <QSet>

static QAbstractItemModel *model = nullptr;

TopicKey::TopicKey(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TopicKey)
{
    ui->setupUi(this);
    if (model)
    {
        // Get the list of column names
        QStringList columns;
        int max = model->columnCount();
        ui->dataColumn->clear();
        columns.append("--All--");
        for (int col=0; col<max; col++)
            columns.append(model->headerData(col, Qt::Horizontal).toString());
        ui->dataColumn->addItems(columns);
        ui->dataValue->clear();
    }
}

TopicKey::~TopicKey()
{
    delete ui;
}

void TopicKey::setModel(QAbstractItemModel *the_model)
{
    model = the_model;
}

int TopicKey::selectedColumn() const
{
    return ui->dataColumn->currentIndex() - 1;
}

QString TopicKey::selectedValue() const
{
    return ui->dataValue->currentText();
}

void TopicKey::setSelectedColumn(int column)
{
    ui->dataColumn->setCurrentIndex(column+1);
}

void TopicKey::setSelectedValue(const QString &value)
{
    ui->dataValue->setCurrentText(value);
}

void TopicKey::on_dataColumn_currentIndexChanged(int column)
{
    // populate dataValue with the possible different values from the data model
    ui->dataValue->clear();

    // Get the unique set of values from the selected column
    if (column == 0)  return;

    // Convert to model column number
    --column;

    QSet<QString> values;
    int max = model->rowCount();
    for (int row=0; row<max; row++)
        values.insert(model->index(row,column).data().toString());
    ui->dataValue->addItems(values.toList());
}
