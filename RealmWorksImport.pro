#-------------------------------------------------
#
# Project created by QtCreator 2017-03-03T23:56:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RealmWorksImport
TEMPLATE = app

CONFIG += windeployqt

SOURCES += main.cpp\
        mainwindow.cpp \
    csvmodel.cpp \
    realmworksstructure.cpp \
    rw_domain.cpp \
    rw_base_item.cpp \
    rw_category.cpp \
    rw_partition.cpp \
    rw_facet.cpp \
    rwcategorywidget.cpp \
    fieldlineedit.cpp \
    rw_structure.cpp \
    fieldcombobox.cpp \
    fieldmultilineedit.cpp

HEADERS  += mainwindow.h \
    csvmodel.h \
    realmworksstructure.h \
    rw_domain.h \
    rw_base_item.h \
    rw_category.h \
    rw_partition.h \
    rw_facet.h \
    rwcategorywidget.h \
    fieldlineedit.h \
    rw_structure.h \
    fieldcombobox.h \
    datafield.h \
    fieldmultilineedit.h

FORMS    += mainwindow.ui

TRANSLATIONS += csv2rw_en.ts

COMPANY = com.amusingtime.csv2rw

PKGSRC=Installer
PKGDIR=packages

#DISTFILES += $${PWD}/$${PKGDIR}/

INST_DIR_DATA = $${PKGDIR}/$${COMPANY}/data
INST_DIR_META = $${PKGDIR}/$${COMPANY}/meta

WINDEPLOYQT_OPTIONS = -verbose 3

DESTDIR = $${INST_DIR_DATA}

QMAKE_POST_LINK += $$quote(\$(COPY_DIR) ..\\RealmWorksImport\\Installer\\packages packages$$escape_expand(\\n\\t))

DISTFILES += \
    LICENSE.txt \
    RELEASE_NOTES.txt \
    TODO.txt \
    Installer/config/config.xml \
    Installer/packages/com.amusingtime.csv2rw/meta/package.xml \
    Installer/packages/com.amusingtime.csv2rw/meta/installscript.qs \
    Installer/packages/com.amusingtime.csv2rw/meta/LICENSE.txt \
    csv2rw_en.ts

#
# Create install file
#
#INSTALLER = CSV2RW.exe
#INPUT = $$PWD/$${PKGSRC}/config/config.xml
#qtinstall.input = INPUT
#qtinstall.name = Generates the Windows Install Program
#qtinstall.output = $$INSTALLER
#qtinstall.depends = $${INST_DIR_DATA}/$$TARGET
#qtinstall.commands = D:/Qt58/QtIFW2.0.5/bin/binarycreator --offline-only -c $$PWD/$${PKGSRC}/config/config.xml -p $$OUT_PWD/$${PKGDIR} ${QMAKE_FILE_OUT}
#qtinstall.CONFIG += no_link combine explicit_dependencies
#QMAKE_EXTRA_COMPILERS += qtinstall
