#-------------------------------------------------
#
# Project created by QtCreator 2017-03-03T23:56:58
#
#-------------------------------------------------

VERSION = 2.14

# Found at https://github.com/dbzhang800/QtXlsxWriter
# (with one tweak to XlsCell to add a richString() method)
include (3rdparty/QtXlsxWriter/src/xlsx/qtxlsx.pri)
include (3rdparty/yaml-cpp/yaml-cpp.pri)

QT       += core gui network xmlpatterns qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = RealmWorksImport
TEMPLATE = app

INCLUDEPATH += 3rdparty/yaml-cpp/include/

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051200

SOURCES += main.cpp \
    addcolumndialog.cpp \
    columnnamemodel.cpp \
    derivedcolumnsproxymodel.cpp \
    jsonmodel.cpp \
    jsontreemodel.cpp \
        mainwindow.cpp \
    csvmodel.cpp \
    realmworksstructure.cpp \
    rw_domain.cpp \
    rw_category.cpp \
    rw_partition.cpp \
    rw_facet.cpp \
    fieldlineedit.cpp \
    rw_structure.cpp \
    fieldcombobox.cpp \
    fieldmultilineedit.cpp \
    performxsltranslation.cpp \
    rw_alias.cpp \
    parentcategorywidget.cpp \
    filedetails.cpp \
    rw_structure_item.cpp \
    rw_contents_item.cpp \
    rw_section.cpp \
    rw_snippet.cpp \
    rw_topic.cpp \
    rw_topic_widget.cpp \
    topickey.cpp \
    datafield.cpp \
    excel_xlsxmodel.cpp \
    htmlitemdelegate.cpp \
    rw_relationship.cpp \
    rw_relationship_widget.cpp \
    errordialog.cpp \
    yamlmodel.cpp

HEADERS  += mainwindow.h \
    addcolumndialog.h \
    columnnamemodel.h \
    csvmodel.h \
    derivedcolumnsproxymodel.h \
    jsonmodel.h \
    jsontreemodel.h \
    realmworksstructure.h \
    rw_domain.h \
    rw_category.h \
    rw_partition.h \
    rw_facet.h \
    fieldlineedit.h \
    rw_structure.h \
    fieldcombobox.h \
    datafield.h \
    fieldmultilineedit.h \
    regexp.h \
    performxsltranslation.h \
    rw_alias.h \
    parentcategorywidget.h \
    filedetails.h \
    rw_structure_item.h \
    rw_contents_item.h \
    rw_section.h \
    rw_snippet.h \
    rw_topic.h \
    rw_topic_widget.h \
    topickey.h \
    excel_xlsxmodel.h \
    htmlitemdelegate.h \
    rw_relationship.h \
    rw_relationship_widget.h \
    errordialog.h \
    yamlmodel.h

FORMS    += mainwindow.ui \
    addcolumndialog.ui \
    filedetails.ui \
    topickey.ui \
    errordialog.ui

TRANSLATIONS += rwImporter_en.ts

macx {
MACDEPLOYQT_OPTIONS = -verbose 3 -dmg
CONFIG += macdeployqt
}

win32 {
WINDEPLOYQT_OPTIONS = -verbose 3
CONFIG += windeployqt
}

COMPANY = com.amusingtime.rwImporter

DESTDIR = install

# Installation on Windows is performed using NSIS (see http://nsis.sourceforge.net/)
# On Project page, on the "Run" tag, under "Deployment"
# 1 - add a "Make" step with the following:
#     Make arguments = "-f Makefile.%{CurrentBuild:Name} windeployqt"
# 2 - add a "Custom" step with the following:
#     Command   = "C:\Program Files (x86)\NSIS\makeNSIS.exe"
#     Arguments = "/nocd %{CurrentProject:NativePath}\config.nsi"
#     Working Directory = "%{buildDir}"

DISTFILES += \
    LICENSE.txt \
    RELEASE_NOTES.txt \
    rwImporter_en.ts \
    ob2rw-xsl1.xslt \
    config.nsi

RESOURCES += \
    rwimport.qrc
