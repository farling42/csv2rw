#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

#include <QMainWindow>
#include "realmworksstructure.h"

namespace Ui {
class MainWindow;
}

class CsvModel;
class ExcelXlsxModel;
class FileDetails;
class QAbstractProxyModel;
class ColumnNameModel;
class ParentCategoryWidget;
class RWTopicWidget;
class RWRelationshipWidget;
class YamlModel;
class JsonModel;
class DerivedColumnsProxyModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &filename = QString(), QWidget *parent = nullptr);
    ~MainWindow() override;

    QStringList parseCSV(const QString &string);

public Q_SLOTS:
    void loadProject();
    void saveProject();
    void saveProjectAs();
    void on_loadDataButton_pressed();
    void on_loadStructureButton_pressed();
    void on_categoryComboBox_currentTextChanged(const QString&);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private slots:
    void on_generateButton_clicked();
    void showBriefHelp();
    void on_convertOP_triggered();
    void on_addParent_clicked();
    void delete_parent();
    void parent_topics_changed();
    void showAbout();

    void on_addRelationship_clicked();
    void on_actionForce_Format_3_toggled(bool checked);
    void delete_relationship(RWRelationshipWidget*);

    void on_saveColumns_clicked();

    void on_loadColumns_clicked();

private:
    Ui::MainWindow *ui{nullptr};
    QAbstractProxyModel *proxy{nullptr};
    CsvModel *csv_full_model{nullptr};
    ExcelXlsxModel *excel_full_model{nullptr};
    YamlModel *yaml_model{nullptr};
    JsonModel *json_model{nullptr};
    ColumnNameModel *header_model{nullptr};
    RealmWorksStructure rw_structure;
    RWTopicWidget *topic_widget{nullptr};
    RWTopic *current_topic{nullptr};
    QList<ParentCategoryWidget*> parents;
    QList<RWRelationshipWidget*> relationships;
    FileDetails *file_details{nullptr};
    DerivedColumnsProxyModel *derived_columns{nullptr};
    QMap<QString, RWTopic*> p_all_topics;
    QString base_window_title;
    QString project_name;
    bool load_project(const QString &filename);
    bool save_project(const QString &filename);
    void set_project_filename(const QString &filename);
    bool load_structure(const QString &filename);
    bool load_data(const QString &filename, const QString &worksheet = QString());
    void set_current_topic(const QString &selection);
    bool discardChanges(const QString &msg);
};

#endif // MAINWINDOW_H
