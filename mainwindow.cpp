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
#include <QSettings>
#include <rwcategorywidget.h>

#include "realmworksstructure.h"
#include "performxsltranslation.h"
#include "parentcategorywidget.h"

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
    ui->addParent->setEnabled(false);

    // Set some icons that can't be set in Qt Creator
    ui->loadCsvButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogStart));
    ui->loadStructureButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogStart));
    ui->generateButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->generateButton->setEnabled(false);
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
    QSettings settings;
    const QString CSV_DIRECTORY_PARAM("csvDirectory");

    // Prompt use to select a CSV file
    QString filename = QFileDialog::getOpenFileName(this, tr("CSV File"), /*dir*/ settings.value(CSV_DIRECTORY_PARAM).toString(), /*template*/ tr("CSV Files (*.csv)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
    {
        qWarning() << tr("Failed to find file") << file.fileName();
        return;
    }
    ui->csvFilename->setText(filename);
    csv_full_model->readCSV(file);

    // Remember the CSV directory
    settings.setValue(CSV_DIRECTORY_PARAM, QFileInfo(file).absolutePath());

    // Switch to the CSV directory, in case we need to load images.
    QDir::setCurrent(QFileInfo(file).absolutePath());

    // Put headers into the header model
    QStringList headers;
    for (int row = 0; row < csv_full_model->columnCount(); row++)
    {
        headers.append(csv_full_model->headerData(row, Qt::Horizontal).toString());
    }
    header_model->setStringList(headers);
    //qDebug() << "Model size: " << csv_full_model->rowCount() << "rows and" << csv_full_model->columnCount() << "columns";
    ui->generateButton->setEnabled(rw_structure.categories.size() > 0);
}


void MainWindow::on_loadStructureButton_pressed()
{
    QSettings settings;
    const QString STRUCTURE_DIRECTORY_PARAM("structureDirectory");

    // Prompt use to select a CSV file
    QString filename = QFileDialog::getOpenFileName(this, tr("Structure File"), /*dir*/ settings.value(STRUCTURE_DIRECTORY_PARAM).toString(), /*template*/ tr("Realm Works® Structure Files (*.rwstructure)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
    {
        qWarning() << tr("Failed to find file") << file.fileName();
        return;
    }
    ui->structureFilename->setText(filename);
    rw_structure.loadFile(&file);

    // Remember the Structure directory
    settings.setValue(STRUCTURE_DIRECTORY_PARAM, QFileInfo(file).absolutePath());

    // Get the list of category names into a sorted list
    QStringList cats;
    foreach (RWCategory *category, rw_structure.categories)
    {
        cats.append(category->name());
    }
    cats.sort();
    ui->categoryComboBox->clear();
    ui->categoryComboBox->addItems(cats);
    ui->addParent->setEnabled(true);
    ui->generateButton->setEnabled(csv_full_model->rowCount() > 0);
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

    //qDebug() << "Selected category" << choice->name();

    category_widget = new RWCategoryWidget(choice, header_model);
    ui->categoryScrollArea->setWidget(category_widget);
}


void MainWindow::on_generateButton_clicked()
{
    QSettings settings;
    const QString OUTPUT_DIRECTORY_PARAM("outputDirectory");

    ui->generateButton->setEnabled(false);

    // Check that the topic has been configured correctly.
    if (!category_widget->category()->canBeGenerated())
    {
        QMessageBox::critical(this, tr("Incomplete Data"),
                              tr("The topic/article needs to have a field allocated to the name."));
        ui->generateButton->setEnabled(true);
        return;
    }

    // Check that if a parent has been specified, that it has a name
    QSet<RWCategory*> used_categories;
    used_categories.insert(category_widget->category());
    foreach (ParentCategoryWidget *widget, parents)
    {
        if (!widget->category()->namefield().isDefined())
        {
            QMessageBox::critical(this, tr("Incomplete Parent"),
                                  tr("The parent topic/article needs a name."));
            ui->generateButton->setEnabled(true);
            return;
        }
        if (used_categories.contains(widget->category()))
        {
            QMessageBox::critical(this, tr("Bad Parent Category"),
                                  tr("The parent should be a different category to the CSV topic and other parents."));
            ui->generateButton->setEnabled(true);
            return;
        }
        used_categories.insert(widget->category());
    }

    // Prompt for output filename
    QString filename = QFileDialog::getSaveFileName(this, tr("Realm Works® Export File"), /*dir*/ settings.value(OUTPUT_DIRECTORY_PARAM).toString(), /*filter*/ tr("Realm Works® Export Files (*.rwexport)"));
    if (filename.isEmpty())
    {
        ui->generateButton->setEnabled(true);
        return;
    }
    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        qWarning() << tr("Failed to create file") << file.fileName();
        ui->generateButton->setEnabled(true);
        return;
    }

    settings.setValue(OUTPUT_DIRECTORY_PARAM, QFileInfo(file).absolutePath());

    QList<RWCategory*> parent_cats;
    foreach (ParentCategoryWidget *widget, parents)
        parent_cats.append(widget->category());
    rw_structure.writeExportFile(&file, category_widget->category(), csv_full_model, parent_cats);
    file.close();

    // Enable button again (so that we know it is finished
    ui->generateButton->setEnabled(true);
}

void MainWindow::on_helpButton_clicked()
{
    static QString help_text = tr("There are various steps to converting your CSV data into a Realm Works® import file\n\n"
            "Step 1: Use the 'Load 'CSV' button to choose the file containing your data in CSV file format. The first line in the file should contain the header for each column.\n\n"
            "Step 2: Use the 'Load Structure' to select the Realm Works structure file containing the structure that you've exported from Realm Works.\n\n"
            "Step 3: Choose the category/article that you want to have created within Realm Works from your CSV data.\n\n"
            "(Hint: the bottom panel shows the information that has been loaded from your CSV file, so you can check which fields contain which data.)\n\n"
            "Step 4: Drag each of the field names from the left panel to the appropriate place within the category/article template in the right panel.\n\n"
            "Step 5: Press the 'Generate' to produce the Realm Works® .rwexport file which you can load into your Realm Works database.")
            ;
    QMessageBox::information(this, tr("Help"), help_text);
}

void MainWindow::on_convertOP_triggered()
{
    translate_obsidian_portal();
}

/**
 * @brief MainWindow::on_addParent_clicked
 * Add another parent to the list of nested parents, and don't allow the previous parents to be deleted.
 */
void MainWindow::on_addParent_clicked()
{
    if (parents.size() > 0) parents.last()->setCanDelete(false);
    ParentCategoryWidget *widget = new ParentCategoryWidget(&rw_structure, header_model, parents.count() * 20);
    connect(widget, &ParentCategoryWidget::deleteRequested, this, &MainWindow::delete_parent);
    parents.append(widget);
    ui->parentWidget->layout()->addWidget(widget);
}

/**
 * @brief MainWindow::delete_parent
 * Remove the most recently added parent.
 */
void MainWindow::delete_parent()
{
    parents.takeLast()->deleteLater();
    if (parents.size() > 0) parents.last()->setCanDelete(true);
}
