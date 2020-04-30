#include "addcolumndialog.h"
#include "ui_addcolumndialog.h"

AddColumnDialog::AddColumnDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddColumnDialog)
{
    ui->setupUi(this);
}

AddColumnDialog::~AddColumnDialog()
{
    delete ui;
}

void AddColumnDialog::setVisible(bool visible)
{
    if (visible) emit requestColumnNames();
    QDialog::setVisible(visible);
}


void AddColumnDialog::setColumnNames(const QStringList &names)
{
    QString current = ui->columnNames->currentText();
    ui->columnNames->clear();
    ui->columnNames->addItems(names);
    if (names.contains(current)) ui->columnNames->setCurrentText(current);
}

void AddColumnDialog::setExpression(const QString &expression)
{
    ui->expression->setText(expression);
}

void AddColumnDialog::on_columnNames_activated(const QString &column)
{
    emit requestExpression(column);
}

void AddColumnDialog::on_columnNames_currentIndexChanged(const QString &column)
{
    emit requestExpression(column);
}

void AddColumnDialog::on_columnNames_editTextChanged(const QString &column)
{
    emit requestExpression(column);
}

void AddColumnDialog::on_deleteButton_clicked()
{
    emit deleteColumn(ui->columnNames->currentText());
    emit requestColumnNames();
}

void AddColumnDialog::on_okButton_clicked()
{
    on_applyButton_clicked();
    hide();
}

void AddColumnDialog::on_applyButton_clicked()
{
    emit updateColumn(ui->columnNames->currentText(), ui->expression->toPlainText());
    emit requestColumnNames();
}

