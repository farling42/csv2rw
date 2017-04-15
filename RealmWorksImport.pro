#-------------------------------------------------
#
# Project created by QtCreator 2017-03-03T23:56:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RealmWorksImport
TEMPLATE = app

CONFIG+= static

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
