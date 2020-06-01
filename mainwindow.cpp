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
#include "yamlmodel.h"
#include "jsonmodel.h"
#include "derivedcolumnsproxymodel.h"
#include "columnnamemodel.h"
#include "addcolumndialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QComboBox>
#include <QTextStream>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QSettings>
#include <QCloseEvent>
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
static const QString DATA_EXTENSION_PARAM("dataExtension");
static const QString STRUCTURE_DIRECTORY_PARAM("structureDirectory");
static const QString CUSTOM_COLUMNS_DIRECTORY_PARAM("customColumnsDirectory");

/*
 *   CSV/YAML/JSON model -> DerivedColumnsProxyModel -> QSortFilterProxyModel
 *
 */

MainWindow::MainWindow(const QString &filename, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    base_window_title = QString("%1 V%2").arg(windowTitle()).arg(qApp->applicationVersion());
    setWindowTitle(base_window_title + "[*]");

    csv_full_model = new CsvModel(this);
    yaml_model = new YamlModel(this);
    json_model = new JsonModel(this);

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

    derived_columns = new DerivedColumnsProxyModel(this);
    derived_columns->setSourceModel(csv_full_model);    // temporary place holder

    // Set up the full data view
    proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(derived_columns);

    // TEST
    derived_columns->setColumn("derived", "row.column('name') + ' = ' + row.column('school')");
    derived_columns->setColumn("newname", "(row.column('material') == 1) ? row.column('school') : 'pp'");

    ui->dataContentsTableView->setModel(proxy);
#ifdef SHOW_FORMATTING
    ui->dataContentsTableView->setItemDelegate(new HtmlItemDelegate);
#endif
    // Set up the list of headers
    header_model = new ColumnNameModel(this);
    header_model->setSourceModel(derived_columns);
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
    connect(ui->actionBriefHelp, &QAction::triggered, this, &MainWindow::showBriefHelp);
    connect(ui->actionAbout_RWImport, &QAction::triggered, this, &MainWindow::showAbout);

    connect(&rw_structure, &RealmWorksStructure::modificationDone, [=]{setWindowModified(true);});

    // Read current (Default) option
    on_actionForce_Format_3_toggled(ui->actionForce_Format_3->isChecked());

    // Some options not available at startup
    ui->sheetBox->hide();
    ui->arrayBox->hide();
    connect(ui->arrayName, &QComboBox::currentTextChanged, json_model,
            [=](const QString &array_name)
    {
        if (array_name != json_model->currentArray() &&
            discardChanges(tr("Discard changes and switch to worksheet %1?").arg(array_name)))
        {
            json_model->setArray(array_name);
            // Reload structure to clear out all the field mappings
            if (!p_all_topics.isEmpty()) if (!p_all_topics.isEmpty()) load_structure(ui->structureFilename->text());
        }
        else
        {
            // Revert name
            ui->arrayName->setCurrentText(json_model->currentArray());
        }
    });

    // An optional starting filename might have been supplied.
    if (!filename.isEmpty())
    {
        load_project(filename);
    }

    // Create the dialog box to allow for creating derived columns.
    AddColumnDialog *acd = new AddColumnDialog(this);
    connect(acd, &AddColumnDialog::updateColumn, derived_columns, &DerivedColumnsProxyModel::setColumn);
    connect(acd, &AddColumnDialog::deleteColumn, derived_columns, &DerivedColumnsProxyModel::deleteColumn);
    connect(acd, &AddColumnDialog::requestExpression,  [=](const QString &name) { acd->setExpression(derived_columns->expression(name)); });
    connect(acd, &AddColumnDialog::requestColumnNames, [=]() { acd->setColumnNames(derived_columns->columnNames()); });

    connect(ui->derivedColumns, &QPushButton::clicked, acd, &QDialog::show);
    //connect(acd, &AddColumnDialog::, derived_columns, &DerivedColumnsProxyModel::);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_project_filename(const QString &filename)
{
    project_name = filename;
    setWindowTitle(base_window_title + " : " + filename + "[*]");
}

const QString VERSION_LABEL{"VERSION"};

bool MainWindow::save_project(const QString &filename)
{
    //qDebug() << "Saving project to" << filename;
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) return false;
    QDataStream stream(&file);
    stream << VERSION_LABEL;
    stream << 0x0214;   // save file version number
    stream << ui->dataFilename->text();
    stream << ui->sheetName->currentText();
    stream << ui->arrayName->currentText();
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
    // Optional extra check box saved
    stream << ui->actionForce_Format_3->isChecked();
    // Any custom columns
    stream << *derived_columns;

    setWindowModified(false);
    return true;
}

bool MainWindow::load_project(const QString &filename)
{
    //qDebug() << "Loading project from" << filename;
    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) return false;
    QDataStream stream(&file);

    QString datafile;
    QString worksheet;
    QString array_name;
    QString structurefile;
    QString current_topic;

    // Read parameters in order (Versions 2.9 onwards has VERSION as first keyword)
    int save_file_version = 0x0208;
    stream >> datafile;
    if (datafile == VERSION_LABEL)  // Save versions from v2.8 and earlier didn't have this keyword
    {
        stream >> save_file_version;
        stream >> datafile;
    }
    qApp->setProperty("saveVersion", save_file_version);
    qDebug() << "Loading save file in format" << QString::number(save_file_version, 16);

    stream >> worksheet;
    if (save_file_version >= 0x0214)
    {
        stream >> array_name;
    }
    stream >> structurefile;
    stream >> current_topic;

    if (!load_data(datafile, worksheet))
    {
        QMessageBox::critical(this, tr("Load Project Failed"), tr("Failed to load data from %1").arg(datafile));
        return false;
    }
    if (derived_columns->sourceModel() == json_model)
    {
        json_model->setArray(array_name);
    }
    if (!load_structure(structurefile))
    {
        QMessageBox::critical(this, tr("Load Project Failed"), tr("Failed to load structure from %1").arg(structurefile));
        return false;
    }
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

    // Some optional addition stuff
    if (!stream.atEnd())
    {
        // Optional flag that contains ForceFormat3 flag
        bool flag;
        stream >> flag;
        ui->actionForce_Format_3->setChecked(flag);
        on_actionForce_Format_3_toggled(flag);
    }
    if (save_file_version >= 0x0212)
        stream >> *derived_columns;
    else
        derived_columns->clearColumns();

    ui->categoryComboBox->setCurrentText(current_topic);
    // Force loading of correct field mappings
    on_categoryComboBox_currentTextChanged(current_topic);

    setWindowModified(false);
    set_project_filename(filename);
    QSettings settings;
    settings.setValue(PROJECT_DIRECTORY_PARAM, QFileInfo(filename).absolutePath());
    return true;
}

void MainWindow::loadProject()
{
    QString filename;

    if (!discardChanges(tr("Discard changes and load new project?"))) return;

    // Offer a file open dialog
    // (Ensure settings is destroyed before load_project is called)
    {
        QSettings settings;
        filename = QFileDialog::getOpenFileName(this, tr("RWImport Project File"), /*dir*/ settings.value(PROJECT_DIRECTORY_PARAM).toString(), /*template*/ tr("RWImport Project Files (*.csv2rw)"));
        if (filename.isEmpty()) return;
    }
    // read the contents of the selected file
    load_project(filename);
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

bool MainWindow::load_data(const QString &filename, const QString &worksheet)
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
        ui->sheetBox->hide();
        ui->arrayBox->hide();
    }
    else if (filename.endsWith((".xlsx")))
    {
        // Excel file
        if (excel_full_model) delete excel_full_model;
        excel_full_model = new ExcelXlsxModel(filename, this);
        model = excel_full_model;
        QStringList sheet_names = excel_full_model->sheetNames();
        if (sheet_names.size() > 1)
        {
            QString sheet = worksheet.isEmpty() ? excel_full_model->currentSheetName() : worksheet;
            if (!worksheet.isEmpty()) excel_full_model->selectSheet(sheet);

            ui->sheetName->clear();
            ui->sheetName->addItems(sheet_names);
            ui->sheetName->setCurrentText(sheet);
            ui->sheetBox->show();
            ui->arrayBox->hide();

            connect(ui->sheetName, &QComboBox::currentTextChanged, excel_full_model,
                    [=](const QString &sheetname)
            {
                if (sheetname != excel_full_model->currentSheetName() &&
                    discardChanges(tr("Discard changes and switch to worksheet %1?").arg(sheetname)))
                {
                    excel_full_model->selectSheet(sheetname);
                    // Reload structure to clear out all the field mappings
                    if (!p_all_topics.isEmpty()) load_structure(ui->structureFilename->text());
                }
                else
                {
                    // Revert name
                    ui->sheetName->setCurrentText(excel_full_model->currentSheetName());
                }
            });
        }
        else
        {
            ui->sheetBox->show();
            ui->arrayBox->hide();
        }
    }
    else if (filename.endsWith(".yaml"))
    {
        if (!yaml_model->readFile(filename))
        {
            qWarning() << tr("Failed to read YAML file") << filename;
            return false;
        }
        model = yaml_model;
        ui->sheetBox->hide();
        ui->arrayBox->hide();
    }
    else if (filename.endsWith(".json"))
    {
        QFile file(filename);
        if (!file.open(QFile::ReadOnly))
        {
            qWarning() << tr("Failed to find file") << file.fileName();
            return false;
        }
        if (!json_model->readFile(file))
        {
            qWarning() << tr("Failed to read JSON file") << file.fileName();
            return false;
        }
        model = json_model;
        ui->sheetBox->hide();
        QStringList arrays = json_model->arrayList();
        if (arrays.size() < 2)
        {
            ui->arrayBox->hide();
        }
        else
        {
            ui->arrayBox->show();
            QString current_array = json_model->currentArray();   // before ui->arrayName triggers a value change
            ui->arrayName->addItems(arrays);
            ui->arrayName->setCurrentText(current_array);
        }
    }
    else
    {
        qCritical() << tr("Unknown File Extension") << filename;
        return false;
    }
    ui->dataFilename->setText(filename);

    // Remember the data directory
    settings.setValue(DATA_DIRECTORY_PARAM, QFileInfo(filename).absolutePath());

    // Switch to the data directory, in case we need to load images.
    QDir::setCurrent(QFileInfo(filename).absolutePath());

    derived_columns->setSourceModel(model);

    //qDebug() << "Model size: " << model->rowCount() << "rows and" << model->columnCount() << "columns";
    ui->generateButton->setEnabled(rw_structure.categories.size() > 0);
    TopicKey::setModel(model);

    setWindowModified(false);

    return true;
}


void MainWindow::on_loadDataButton_pressed()
{
    QSettings settings;

    if (!discardChanges(tr("Discard changes and load new data?"))) return;

    // Prompt use to select a data file
    QString selected_filter = settings.value(DATA_EXTENSION_PARAM).toString();
    QString filename = QFileDialog::getOpenFileName(this,
                                                    /*caption*/ tr("Data File"),
                                                    /*dir*/ settings.value(DATA_DIRECTORY_PARAM).toString(),
                                                    /*template*/ tr("CSV Files (*.csv);;Excel Workbook (*.xlsx);;YAML (*.yaml);;JSON (*.json)"),
                                                    /*selectedFilter*/ &selected_filter);
    qDebug() << "load data: selected filter =" << selected_filter;
    if (filename.isEmpty()) return;
    if (load_data(filename)) settings.setValue(DATA_EXTENSION_PARAM, selected_filter);
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

    setWindowModified(false);

    return true;
}

void MainWindow::on_loadStructureButton_pressed()
{
    QSettings settings;

    if (!discardChanges(tr("Discard changes and load new structure?"))) return;

    // Prompt use to select a data file
    QString filename = QFileDialog::getOpenFileName(this, tr("Structure File"), /*dir*/ settings.value(STRUCTURE_DIRECTORY_PARAM).toString(), /*template*/ tr("Realm Works® Structure Files (*.rwstructure)"));
    if (filename.isEmpty()) return;
    load_structure(filename);
}

void MainWindow::on_categoryComboBox_currentTextChanged(const QString &selection)
{
    //qDebug() << "MainWindow::on_categoryComboBox_currentIndexChanged:" << selection;
    if (selection.isEmpty()) return;

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


bool MainWindow::discardChanges(const QString &msg)
{
    if (!isWindowModified()) return true;

    return QMessageBox::critical
            (this,
             /*title*/ tr("Project not Saved"),
             /*text*/ tr("The project has not been saved.\n%1").arg(msg),
            /*buttons*/ QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (discardChanges("Are you sure that you want to quit?"))
    {
        event->accept();
    }
    else
    {
        event->ignore();
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
                                   "Copyright (C) 2017-2020 Martin Smith\n\n"
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

void MainWindow::on_actionForce_Format_3_toggled(bool checked)
{
    rw_structure.forceFormatVersion(checked ? 3 : 0);
}

void MainWindow::delete_relationship(RWRelationshipWidget *w_conn)
{
    ui->relationshipWidget->layout()->removeWidget(w_conn);
    relationships.removeAll(w_conn);
    current_topic->relationships.removeAll(w_conn->relationship());
    w_conn->relationship()->deleteLater();
    w_conn->deleteLater();
}

void MainWindow::on_saveColumns_clicked()
{
    QSettings settings;
    QString filename = QFileDialog::getSaveFileName(this, tr("RWImport Custom Columns File"),
                                                    /*dir*/ settings.value(CUSTOM_COLUMNS_DIRECTORY_PARAM).toString(),
                                                    /*template*/ tr("RWImport Custom Columns Files (*.custcol)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) return;

    QDataStream stream(&file);
    stream << *derived_columns;
    file.close();
}

void MainWindow::on_loadColumns_clicked()
{
    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(this, tr("RWImport Custom Columns File"),
                                                    /*dir*/ settings.value(CUSTOM_COLUMNS_DIRECTORY_PARAM).toString(),
                                                    /*template*/ tr("RWImport Custom Columns Files (*.custcol)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) return;

    QDataStream stream(&file);
    stream >> *derived_columns;
    file.close();
}
