#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    static ErrorDialog *theInstance();
    ~ErrorDialog();

public slots:
    void addMessage(const QString&);
    void clear();

private:
    Ui::ErrorDialog *ui;
};

#endif // ERRORDIALOG_H
