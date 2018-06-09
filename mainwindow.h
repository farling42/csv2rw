#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

#include <QMainWindow>
#include "realmworksstructure.h"

namespace Ui {
class MainWindow;
}

class CsvModel;
class FileDetails;
class QStringListModel;
class ParentCategoryWidget;
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
    void on_generateButton_clicked();
    void on_helpButton_clicked();
    void on_convertOP_triggered();

    void on_addParent_clicked();

    void delete_parent();
    void on_importName_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    CsvModel *csv_full_model;
    QStringListModel *header_model;
    RealmWorksStructure rw_structure;
    RWCategoryWidget *category_widget;
    QList<ParentCategoryWidget*> parents;
    FileDetails *file_details;
};

#endif // MAINWINDOW_H
