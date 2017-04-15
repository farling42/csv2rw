#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "realmworksstructure.h"

namespace Ui {
class MainWindow;
}

class CsvModel;
class QStringListModel;
class RWCategoryWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QStringList parseCSV(const QString &string);

public Q_SLOTS:
    void fileLoad();
    void fileSave();
    void fileQuit();
    void on_loadCsvButton_pressed();
    void on_loadStructureButton_pressed();
    void on_categoryComboBox_currentIndexChanged(const QString&);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    CsvModel *csv_full_model;
    QStringListModel *header_model;
    RealmWorksStructure rw_structure;
    RWCategoryWidget *category_widget;
};

#endif // MAINWINDOW_H
