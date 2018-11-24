#include "errordialog.h"
#include "ui_errordialog.h"
#include <QDebug>
#include <QPushButton>

ErrorDialog::ErrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
    QPushButton *button = ui->buttonBox->button(QDialogButtonBox::Discard);
    if (button) connect(button, &QPushButton::clicked, this, &ErrorDialog::clear);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

ErrorDialog *ErrorDialog::theInstance()
{
    static ErrorDialog *instance = nullptr;
    if (instance == nullptr) instance = new ErrorDialog;
    return instance;
}

void ErrorDialog::addMessage(const QString &message)
{
    show();
    ui->listWidget->addItem(message);
    qDebug() << "ERROR:" << message;
}

void ErrorDialog::clear()
{
    ui->listWidget->clear();
}
