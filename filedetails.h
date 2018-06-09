#ifndef FILEDETAILS_H
#define FILEDETAILS_H

#include <QDialog>

namespace Ui {
class fileDetails;
}

class RealmWorksStructure;

class FileDetails : public QDialog
{
    Q_OBJECT

public:
    explicit FileDetails(RealmWorksStructure *structure, QWidget *parent = 0);
    ~FileDetails();

protected:
    virtual void showEvent(QShowEvent *event);

private slots:
    void on_fileDetails_accepted();

private:
    Ui::fileDetails *ui;
    RealmWorksStructure *rw_structure;
};

#endif // FILEDETAILS_H
