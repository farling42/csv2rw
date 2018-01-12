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

    QUrl input = QFileDialog::getOpenFileUrl(0, "Choose file containing Obsidian Portal backup file", settings.value(IN_DIRECTORY_PARAM).toString(), "*.xml");
    if (input.isEmpty()) return;
    QString output_filename = QFileDialog::getSaveFileName(0, "Choose output file", settings.value(OUT_DIRECTORY_PARAM).toString(), "*.rwexport");
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
