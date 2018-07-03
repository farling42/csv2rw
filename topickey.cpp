#include "topickey.h"
#include "ui_topickey.h"

#include <QAbstractItemModel>

static QAbstractItemModel *model = 0;

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
        ui->csvColumn->clear();
        columns.append("--All--");
        for (int col=0; col<max; col++)
            columns.append(model->headerData(col, Qt::Horizontal).toString());
        ui->csvColumn->addItems(columns);
        ui->csvValue->clear();
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
    return ui->csvColumn->currentIndex() - 1;
}

QString TopicKey::selectedValue() const
{
    return ui->csvValue->currentText();
}

void TopicKey::setSelectedColumn(int column)
{
    ui->csvColumn->setCurrentIndex(column+1);
}

void TopicKey::setSelectedValue(const QString &value)
{
    ui->csvValue->setCurrentText(value);
}

void TopicKey::on_csvColumn_currentIndexChanged(int column)
{
    // populate csvValue with the possible different values from the CSV model
    ui->csvValue->clear();

    // Get the unique set of values from the selected column
    if (column == 0)  return;

    // Convert to model column number
    --column;

    QSet<QString> values;
    int max = model->rowCount();
    for (int row=0; row<max; row++)
        values.insert(model->index(row,column).data().toString());
    ui->csvValue->addItems(values.toList());
}
