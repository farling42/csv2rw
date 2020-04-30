#ifndef ADDCOLUMNDIALOG_H
#define ADDCOLUMNDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
    class AddColumnDialog;
}


class AddColumnDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddColumnDialog(QWidget *parent = nullptr);
    ~AddColumnDialog();

    void setVisible(bool visible) override;

public slots:
    void setColumnNames(const QStringList&);
    void setExpression(const QString&);

Q_SIGNALS:
    void updateColumn(const QString &name, const QString &expression);
    void deleteColumn(const QString &name);
    void requestExpression(const QString &name);
    void requestColumnNames();

private slots:
    void on_columnNames_activated(const QString &arg1);
    void on_deleteButton_clicked();
    void on_okButton_clicked();
    void on_applyButton_clicked();

    void on_columnNames_currentIndexChanged(const QString &arg1);

    void on_columnNames_editTextChanged(const QString &arg1);

private:
    Ui::AddColumnDialog *ui;
};

#endif // ADDCOLUMNDIALOG_H
