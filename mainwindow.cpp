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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "csvmodel.h"
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <rwcategorywidget.h>

#include "realmworksstructure.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    category_widget(0)
{
    ui->setupUi(this);

    csv_full_model = new CsvModel(this);
    // Set up the full CSV view
    QAbstractProxyModel *proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(csv_full_model);
    ui->csvContentsTableView->setModel(proxy);

    // Set up the list of headers
    header_model = new QStringListModel;
    ui->headerListView->setModel(header_model);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fileLoad()
{

}

void MainWindow::fileSave()
{

}

void MainWindow::fileQuit()
{

}

void MainWindow::on_loadCsvButton_pressed()
{
    // Prompt use to select a CSV file
    QString filename = QFileDialog::getOpenFileName(this, tr("CSV File"), /*dir*/ QString(), /*template*/ tr("CSV Files (*.csv)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
    {
        qWarning() << "Failed to find file" << file.fileName();
        return;
    }
    ui->csvFilename->setText(filename);
    QTextStream stream(&file);
    csv_full_model->readCSV(stream);

    // Put headers into the header model
    QStringList headers;
    for (int row = 0; row < csv_full_model->columnCount(); row++)
    {
        headers.append(csv_full_model->headerData(row, Qt::Horizontal).toString());
    }
    header_model->setStringList(headers);
    qDebug() << "Model size: " << csv_full_model->rowCount() << "rows and" << csv_full_model->columnCount() << "columns";
}

void MainWindow::on_loadStructureButton_pressed()
{
    // Prompt use to select a CSV file
    QString filename = QFileDialog::getOpenFileName(this, tr("Structure File"), /*dir*/ QString(), /*template*/ tr("RealmWorks Structure Files (*.rwstructure)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
    {
        qWarning() << "Failed to find file" << file.fileName();
        return;
    }
    ui->structureFilename->setText(filename);
    rw_structure.loadFile(&file);

    // Get the list of category names into a sorted list
    QStringList cats;
    foreach (RWCategory *category, rw_structure.categories)
    {
        cats.append(category->name());
    }
    cats.sort();
    ui->categoryComboBox->clear();
    foreach (const QString &str, cats)
        ui->categoryComboBox->addItem(str);
}

void MainWindow::on_categoryComboBox_currentIndexChanged(const QString &selection)
{
    RWCategory *choice = 0;
    foreach (RWCategory *category, rw_structure.categories)
    {
        if (category->name() == selection)
        {
            choice = category;
            break;
        }
    }
    if (choice == 0) return;

    qDebug() << "Selected category" << choice->name();

    category_widget = new RWCategoryWidget(choice, header_model);
    ui->categoryScrollArea->setWidget(category_widget);
}


void MainWindow::on_pushButton_clicked()
{
    // Prompt for output filename
    QString filename = QFileDialog::getSaveFileName(this, tr("RealmWorks Export File"), /*dir*/ QString(), /*filter*/ tr("RealmWorks Export Files (*.rwexport)"));
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        qWarning() << "Failed to create file" << file.fileName();
        return;
    }
    rw_structure.writeExportFile(&file, category_widget->category(), csv_full_model);
    file.close();
}

void MainWindow::on_pushButton_2_clicked()
{
    static QString help_text = "There are various steps to converting your CSV data into a RealmWorks import file\n\n"
            "Step 1: Use the 'Load 'CSV' button to choose the file containing your data in CSV file format. The first line in the file should contain the header for each column.\n\n"
            "Step 2: Use the 'Load Structure' to select the RealmWorks structure file containing the structure that you've exported from RealmWorks.\n\n"
            "Step 3: Choose the category/article that you want to have created within RealmWorks from your CSV data.\n\n"
            "(Hint: the bottom panel shows the information that has been loaded from your CSV file, so you can check which fields contain which data.)\n\n"
            "Step 4: Drag each of the field names from the left panel to the appropriate place within the category/article template in the right panel.\n\n"
            "Step 5: Press the 'Generate' to produce the RealmWorks .rwexport file which you can load into your RealmWorks database."
            ;
    QMessageBox::information(this, "Help", help_text);
}
