/*
RWImporter
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

#define SHOW_FORMATTING

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
#include <rw_topic_widget.h>

#include "rw_topic.h"

#include "excel_xlsxmodel.h"
#include "filedetails.h"
#include "realmworksstructure.h"
#include "performxsltranslation.h"
#include "parentcategorywidget.h"
#include "topickey.h"
#ifdef SHOW_FORMATTING
#include "htmlitemdelegate.h"
#endif
#include "rw_relationship.h"
#include "rw_relationship_widget.h"

static const QString PROJECT_DIRECTORY_PARAM("csvProjectDirectory");
static const QString DATA_DIRECTORY_PARAM("csvDirectory");
static const QString STRUCTURE_DIRECTORY_PARAM("structureDirectory");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(QString("%1   v%2").arg(windowTitle()).arg(qApp->applicationVersion()));

    csv_full_model = new CsvModel(this);
    QActionGroup *separators = new QActionGroup(this);
    separators->addAction(ui->actionUse_Comma);
    separators->addAction(ui->actionUse_Semicolon);
    separators->addAction(ui->actionUse_Windows_List_Separator);
    connect(ui->actionUse_Comma,     &QAction::triggered, [=] { csv_full_model->setSeparator(','); });
    connect(ui->actionUse_Semicolon, &QAction::triggered, [=] { csv_full_model->setSeparator(';'); });
    connect(ui->actionUse_Windows_List_Separator, &QAction::triggered, [=] { csv_full_model->setSeparator(QChar()); });
    QChar sep = csv_full_model->fieldSeparator();
    if (sep.isNull())
        ui->actionUse_Windows_List_Separator->setChecked(true);
    else if (sep == ',')
        ui->actionUse_Comma->setChecked(true);
    else if (sep == ';')
        ui->actionUse_Semicolon->setChecked(true);

    // Set up the full data view
    proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(csv_full_model);
    ui->dataContentsTableView->setModel(proxy);
#ifdef SHOW_FORMATTING
    ui->dataContentsTableView->setItemDelegate(new HtmlItemDelegate);
#endif
    // Set up the list of headers
    header_model = new QStringListModel;
    ui->headerListView->setModel(header_model);
    ui->addParent->setEnabled(false);
    ui->addRelationship->setEnabled(false);

    // Set some icons that can't be set in Qt Creator
    ui->loadDataButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogStart));
    ui->loadStructureButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogStart));
    ui->generateButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->generateButton->setEnabled(false);
    rw_structure.details_name = "Test Import";

    // Create the dialog box for entering file details
    file_details = new FileDetails(&rw_structure, this);
    connect(ui->fileDetails, &QPushButton::pressed, file_details, &QDialog::show);
    ui->fileDetails->setEnabled(false);

    // Standard shortcuts
    ui->actionLoad->setShortcut(QKeySequence(tr("Ctrl+O")));
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionSave_AS->setShortcut(QKeySequence::SaveAs);
    ui->actionQuit->setShortcut(QKeySequence::Quit);
    //ui->actionBriefHelp->setShortcut();
    //ui->actionAbout_RWImport->setShortcut();

    // Connect menu options
    connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::loadProject);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveProject);
    connect(ui->actionSave_AS, &QAction::triggered, this, &MainWindow::saveProjectAs);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::fileQuit);
    connect(ui->actionBriefHelp, &QAction::triggered, this, &MainWindow::showBriefHelp);
    connect(ui->actionAbout_RWImport, &QAction::triggered, this, &MainWindow::showAbout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_project_filename(const QString &filename)
{
    project_name = filename;
}

bool MainWindow::save_project(const QString &filename)
{
    //qDebug() << "Saving project to" << filename;
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) return false;
    QDataStream stream(&file);
    stream << ui->dataFilename->text();
    stream << ui->structureFilename->text();
    stream << ui->categoryComboBox->currentText();  // name of topic currently on display
    rw_structure.saveState(stream);
    // Get a list of all the defined topics
    QStringList topic_list;
    for (auto topic : p_all_topics)
    {
        if (topic->publicName().namefield().modelColumn() >= 0)
        {
            topic_list.append(topic->structure->name());
        }
    }
    stream << topic_list;
    // Now write out the contents of each defined topic
    for (auto topic : p_all_topics)
    {
        if (topic->publicName().namefield().modelColumn() >= 0)
        {
            stream << *topic;
        }
    }
    return true;
}

bool MainWindow::load_project(const QString &filename)
{
    //qDebug() << "Loading project from" << filename;
    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) return false;
    QDataStream stream(&file);

    QString value;
    QString current_topic;

    // Read parameters in order
    stream >> value;
    if (!load_data(value)) return false;
    stream >> value;
    if (!load_structure(value)) return false;
    stream >> current_topic;
    rw_structure.loadState(stream);

    // Get the list of configured topics
    QStringList topic_list;
    stream >> topic_list;
    //qDebug() << "Loading topics" << topic_list;

    // Create a lookup table of the known categories
    QMap<QString,RWCategory*> cat_map;
    for (auto cat: rw_structure.categories)
        cat_map.insert(cat->name(), cat);

    // Now all the defined column mappings
    for (auto name : topic_list)
    {
        RWCategory *category = cat_map.value(name, nullptr);
        if (category)
        {
            RWTopic *topic = qobject_cast<RWTopic*>(category->createContentsTree());
            stream >> *topic;
            p_all_topics.insert(name, topic);
        }
        else
        {
            qDebug() << "MainWindow::load_project:" << name << "not in category list";
            stream.setStatus(QDataStream::ReadCorruptData);
        }
    }
    ui->categoryComboBox->setCurrentText(current_topic);

    return true;
}

void MainWindow::loadProject()
{
    QSettings settings;

    // Offer a file open dialog
    QString filename = QFileDialog::getOpenFileName(this, tr("RWImport Project File"), /*dir*/ settings.value(PROJECT_DIRECTORY_PARAM).toString(), /*template*/ tr("RWImport Project Files (*.csv2rw)"));
    if (filename.isEmpty()) return;
    // read the contents of the selected file
    if (load_project(filename))
    {
        set_project_filename(filename);
        settings.setValue(PROJECT_DIRECTORY_PARAM, QFileInfo(filename).absolutePath());
    }
}

void MainWindow::saveProject()
{
    // If no project has been saved yet, do save-as
    if (project_name.isEmpty())
    {
        saveProjectAs();
        return;
    }
    save_project(project_name);
}

void MainWindow::saveProjectAs()
{
    QSettings settings;

    // Open a file open dialog
    QString filename = QFileDialog::getSaveFileName(this,
                                                    /*caption*/ tr("RWImport Project File"),
                                                    /*dir*/ settings.value(PROJECT_DIRECTORY_PARAM).toString() + '/' + rw_structure.details_name + ".csv2rw",
                                                    /*filter*/ tr("RWImport Project Files (*.csv2rw)"));
    if (filename.isEmpty()) return;
    if (save_project(filename))
    {
        set_project_filename(filename);
        settings.setValue(PROJECT_DIRECTORY_PARAM, QFileInfo(filename).absolutePath());
    }
}

void MainWindow::fileQuit()
{
    // Prompt if we have an unsaved project
    qApp->quit();
}

bool MainWindow::load_data(const QString &filename)
{
    //qDebug() << "MainWindow::load_data" << filename;
    QSettings settings;

    QAbstractItemModel *model;

    if (filename.endsWith((".csv")))
    {
        QFile file(filename);
        if (!file.open(QFile::ReadOnly))
        {
            qWarning() << tr("Failed to find file") << file.fileName();
            return false;
        }
        csv_full_model->readCSV(file);
        model = csv_full_model;
    }
    else
    {
        // Excel file
        if (excel_full_model) delete excel_full_model;
        excel_full_model = new ExcelXlsxModel(filename, this);
        model = excel_full_model;
    }
    ui->dataFilename->setText(filename);

    // Remember the data directory
    settings.setValue(DATA_DIRECTORY_PARAM, QFileInfo(filename).absolutePath());

    // Switch to the data directory, in case we need to load images.
    QDir::setCurrent(QFileInfo(filename).absolutePath());

    // Put headers into the header model
    QStringList headers;
    for (int row = 0; row < model->columnCount(); row++)
    {
        headers.append(model->headerData(row, Qt::Horizontal).toString());
    }
    header_model->setStringList(headers);

    //qDebug() << "Model size: " << model->rowCount() << "rows and" << model->columnCount() << "columns";
    ui->generateButton->setEnabled(rw_structure.categories.size() > 0);
    TopicKey::setModel(model);
    proxy->setSourceModel(model);

    return true;
}

void MainWindow::on_loadDataButton_pressed()
{
    QSettings settings;
    // Prompt use to select a data file
    QString filename = QFileDialog::getOpenFileName(this, tr("Data File"), /*dir*/ settings.value(DATA_DIRECTORY_PARAM).toString(), /*template*/ tr("CSV Files (*.csv);;Excel Workbook (*.xlsx)"));
    if (filename.isEmpty()) return;
    load_data(filename);
}

bool MainWindow::load_structure(const QString &filename)
{
    //qDebug() << "MainWindow::load_structure" << filename;
    QSettings settings;
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
    {
        qWarning() << tr("Failed to find file") << file.fileName();
        return false;
    }

    // Delete all previous information
    qDeleteAll(p_all_topics);
    p_all_topics.clear();

    ui->structureFilename->setText(filename);
    rw_structure.loadFile(&file);

    // Remember the Structure directory
    settings.setValue(STRUCTURE_DIRECTORY_PARAM, QFileInfo(file).absolutePath());

    // Get the list of category names into a sorted list
    QStringList cats;
    for (auto category: rw_structure.categories)
    {
        cats.append(category->name());
    }
    cats.sort();
    ui->categoryComboBox->clear();
    ui->categoryComboBox->addItems(cats);
    ui->addParent->setEnabled(true);
    ui->addRelationship->setEnabled(true);
    ui->generateButton->setEnabled(proxy->sourceModel()->rowCount() > 0);
    ui->fileDetails->setEnabled(true);
    return true;
}

void MainWindow::on_loadStructureButton_pressed()
{
    QSettings settings;

    // Prompt use to select a data file
    QString filename = QFileDialog::getOpenFileName(this, tr("Structure File"), /*dir*/ settings.value(STRUCTURE_DIRECTORY_PARAM).toString(), /*template*/ tr("Realm Works® Structure Files (*.rwstructure)"));
    if (filename.isEmpty()) return;
    load_structure(filename);
}

void MainWindow::on_categoryComboBox_currentIndexChanged(const QString &selection)
{
    //qDebug() << "MainWindow::on_categoryComboBox_currentIndexChanged:" << selection;

    RWCategory *choice = nullptr;
    for (auto category: rw_structure.categories)
    {
        if (category->name() == selection)
        {
            choice = category;
            break;
        }
    }
    if (choice == nullptr)
    {
        qDebug() << "MainWindow::on_categoryComboBox_currentIndexChanged:" << selection << "not in category list";
        return;
    }

    //qDebug() << "Selected category" << choice->name();

    if (!p_all_topics.contains(selection))
        p_all_topics.insert(selection, qobject_cast<RWTopic*>(choice->createContentsTree()));

    current_topic = p_all_topics.value(selection);
    topic_widget = new RWTopicWidget(current_topic, header_model);   // TODO - build a hierarchy of TOPIC items
    ui->categoryScrollArea->setWidget(topic_widget);

    // Set up the parents (if any)
    while (!parents.isEmpty())
        parents.takeLast()->deleteLater();

    for (auto parent: current_topic->parents)
    {
        if (parents.size() > 0) parents.last()->setCanDelete(false);
        ParentCategoryWidget *widget = new ParentCategoryWidget(&rw_structure, header_model, parents.count() * 20, parent);
        connect(widget, &ParentCategoryWidget::deleteRequested, this, &MainWindow::delete_parent);
        parents.append(widget);
        ui->parentWidget->layout()->addWidget(widget);
    }

    // Set up the relationships (if any)
    qDeleteAll(relationships);
    relationships.clear();

    for (auto relationship : current_topic->relationships)
    {
        RWRelationshipWidget *widget = new RWRelationshipWidget(relationship, header_model);
        connect(widget, &RWRelationshipWidget::deleteRequested, [=]() { delete_relationship(widget); });
        relationships.append(widget);
        ui->relationshipWidget->layout()->addWidget(widget);
    }
}


void MainWindow::on_generateButton_clicked()
{
    QSettings settings;
    const QString OUTPUT_DIRECTORY_PARAM("outputDirectory");

    ui->generateButton->setEnabled(false);

    // Check that the topic has been configured correctly.
    if (!topic_widget->topic()->canBeGenerated())
    {
        QMessageBox::critical(this, tr("Incomplete Data"),
                              tr("The topic/article needs to have a field allocated to the name."));
        ui->generateButton->setEnabled(true);
        return;
    }

    // Check that if a parent has been specified, that it has a name
    QSet<RWTopic*> used_topics;
    used_topics.insert(topic_widget->topic());
    for (auto widget: parents)
    {
        if (!widget->topic()->publicName().namefield().isDefined())
        {
            QMessageBox::critical(this, tr("Incomplete Parent"),
                                  tr("The parent topic/article needs a name."));
            ui->generateButton->setEnabled(true);
            return;
        }
        used_topics.insert(widget->topic());
    }

    // Prompt for output filename
    QString filename = QFileDialog::getSaveFileName(this,
                                                    /*caption*/ tr("Realm Works® Export File"),
                                                    /*dir*/ settings.value(OUTPUT_DIRECTORY_PARAM).toString() + '/' + rw_structure.details_name + ".rwexport",
                                                    /*filter*/ tr("Realm Works® Export Files (*.rwexport)"));
    if (filename.isEmpty())
    {
        ui->generateButton->setEnabled(true);
        return;
    }

    QFile file(filename);
    // On windows, the creation time-stamp needs to be created properly
    if (file.exists() && !file.remove())
    {
        qWarning() << tr("Failed to remove old file") << file.fileName();
        ui->generateButton->setEnabled(true);
        return;
    }

    if (!file.open(QFile::WriteOnly))
    {
        qWarning() << tr("Failed to create file") << file.fileName();
        ui->generateButton->setEnabled(true);
        return;
    }

    settings.setValue(OUTPUT_DIRECTORY_PARAM, QFileInfo(file).absolutePath());

    QList<RWTopic*> parent_topics;
    for (auto widget: parents)
        parent_topics.append(widget->topic());

    // TODO - parent_topics is specific to each RWTopic
    rw_structure.writeExportFile(&file, p_all_topics.values(), proxy->sourceModel());
    file.close();

    // Enable button again (so that we know it is finished
    ui->generateButton->setEnabled(true);
}

void MainWindow::showBriefHelp()
{
    static QString help_text = tr("There are various steps to converting your data data into a Realm Works® import file\n\n"
            "Step 1: Use the 'Load Data' button to choose the file containing your data in data file format. The first line in the file should contain the header for each column.\n\n"
            "Step 2: Use the 'Load Structure' to select the Realm Works structure file containing the structure that you've exported from Realm Works.\n\n"
            "Step 3: Choose the category/article that you want to have created within Realm Works from your data.\n\n"
            "(Hint: the bottom panel shows the information that has been loaded from your data file, so you can check which fields contain which data.)\n\n"
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
    ParentCategoryWidget *widget = new ParentCategoryWidget(&rw_structure, header_model, parents.count() * 20, nullptr);
    connect(widget, &ParentCategoryWidget::deleteRequested, this, &MainWindow::delete_parent);
    connect(widget, &ParentCategoryWidget::categoryChanged, this, &MainWindow::parent_topics_changed);
    parents.append(widget);
    current_topic->parents.append(widget->topic());
    ui->parentWidget->layout()->addWidget(widget);
}


void MainWindow::parent_topics_changed()
{
    current_topic->parents.clear();
    for (auto parent: parents)
    {
        current_topic->parents.append(parent->topic());
    }
}

void MainWindow::showAbout()
{
    static QString about_text = tr("Realm Works® Importer\n\n"
                                   "Copyright (C) 2017-2018 Martin Smith\n\n"
                                   "Version %1").arg(qApp->applicationVersion());
    QMessageBox::information(this, tr("About"), about_text);
}

/**
 * @brief MainWindow::delete_parent
 * Remove the most recently added parent.
 */
void MainWindow::delete_parent()
{
    parents.takeLast()->deleteLater();
    current_topic->parents.takeLast()->deleteLater();
    if (parents.size() > 0) parents.last()->setCanDelete(true);
}

/**
 * @brief MainWindow::on_addConnection_clicked
 * Add a new connection to be established from an instance of this topic to another named topic.
 */
void MainWindow::on_addRelationship_clicked()
{
    RWRelationship *relationship = new RWRelationship;
    RWRelationshipWidget *widget = new RWRelationshipWidget(relationship, header_model);
    connect(widget, &RWRelationshipWidget::deleteRequested, [=]() { delete_relationship(widget); });
    //connect(widget, &RWConnectionWidget::categoryChanged, this, &MainWindow::connection_changed);
    relationships.append(widget);
    current_topic->relationships.append(widget->relationship());
    ui->relationshipWidget->layout()->addWidget(widget);
}

void MainWindow::delete_relationship(RWRelationshipWidget *w_conn)
{
    ui->relationshipWidget->layout()->removeWidget(w_conn);
    relationships.removeAll(w_conn);
    current_topic->relationships.removeAll(w_conn->relationship());
    w_conn->relationship()->deleteLater();
    w_conn->deleteLater();
}
