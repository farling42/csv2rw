/*
RWImporter
Copyright (C) 2018 Martin Smith

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

#include "filedetails.h"
#include "realmworksstructure.h"
#include "ui_filedetails.h"

#include <QDebug>

FileDetails::FileDetails(RealmWorksStructure *structure, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fileDetails),
    rw_structure(structure)
{
    ui->setupUi(this);
}

FileDetails::~FileDetails()
{
    delete ui;
}

void FileDetails::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    ui->name->setText(rw_structure->details_name);
    ui->version->setText(rw_structure->details_version);
    ui->abbreviation->setText(rw_structure->details_abbrev);

    ui->summary->setPlainText(rw_structure->details_summary);
    ui->description->setPlainText(rw_structure->details_description);
    ui->requirements->setPlainText(rw_structure->details_requirements);
    ui->credits->setPlainText(rw_structure->details_requirements);
    ui->legal->setPlainText(rw_structure->details_legal);
    ui->otherNotes->setPlainText(rw_structure->details_other_notes);
}

void FileDetails::on_fileDetails_accepted()
{
    rw_structure->details_name = ui->name->text();
    rw_structure->details_version = ui->version->text();
    rw_structure->details_abbrev = ui->abbreviation->text();

    rw_structure->details_summary = ui->summary->toPlainText();
    rw_structure->details_description = ui->description->toPlainText();
    rw_structure->details_requirements = ui->requirements->toPlainText();
    rw_structure->details_credits = ui->credits->toPlainText();
    rw_structure->details_legal = ui->legal->toPlainText();
    rw_structure->details_other_notes = ui->otherNotes->toPlainText();
}
