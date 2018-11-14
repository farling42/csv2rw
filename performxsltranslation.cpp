/*
CSV2RW
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

#include "performxsltranslation.h"

#include <QXmlQuery>
#include <QFile>
#include <QFileDialog>
#include <QSettings>

void translate_obsidian_portal()
{
    QSettings settings;
    const QString IN_DIRECTORY_PARAM("obInDirectory");
    const QString OUT_DIRECTORY_PARAM("obOutDirectory");

    QUrl input = QFileDialog::getOpenFileUrl(nullptr, "Choose file containing Obsidian Portal backup file", settings.value(IN_DIRECTORY_PARAM).toString(), "*.xml");
    if (input.isEmpty()) return;
    QString output_filename = QFileDialog::getSaveFileName(nullptr, "Choose output file", settings.value(OUT_DIRECTORY_PARAM).toString(), "*.rwexport");
    if (output_filename.isEmpty()) return;

    settings.setValue(IN_DIRECTORY_PARAM, QFileInfo(input.fileName()).absolutePath());
    settings.setValue(OUT_DIRECTORY_PARAM, QFileInfo(output_filename).absolutePath());

    QXmlQuery query(QXmlQuery::XSLT20);
    query.setFocus(input);
    // TODO, include this file in the application's resources,
    // and find a proper XSL translator!!!
    query.setQuery(QUrl("ob2rw.xslt"));

    QFile output_file(output_filename);
    if (output_file.open(QFile::WriteOnly))
        query.evaluateTo(&output_file);
}
