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
    fieldcombobox.cpp

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
    fieldcombobox.h

FORMS    += mainwindow.ui

COMPANY = com.amusingtime.csv2rw

#DISTFILES += $${PWD}/install/

INST_DIR_DATA = install/$${COMPANY}/data
INST_DIR_META = install/$${COMPANY}/meta

WINDEPLOYQT_OPTIONS = -verbose 3

DESTDIR = $${INST_DIR_DATA}

QMAKE_POST_LINK += $$quote(\$(COPY_DIR) ..\\RealmWorksImport\\installer\\$${COMPANY} install\\$${COMPANY}$$escape_expand(\\n\\t))
