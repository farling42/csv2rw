#-------------------------------------------------
#
# Project created by QtCreator 2017-03-03T23:56:58
#
#-------------------------------------------------

QT       += core gui network xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RealmWorksImport
TEMPLATE = app

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
    fieldmultilineedit.cpp \
    performxsltranslation.cpp \
    rw_alias.cpp \
    parentcategorywidget.cpp

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
    fieldmultilineedit.h \
    regexp.h \
    performxsltranslation.h \
    rw_alias.h \
    parentcategorywidget.h

FORMS    += mainwindow.ui \
    Installer/packages/com.amusingtime.csv2rw/meta/page.ui

TRANSLATIONS += csv2rw_en.ts

macx {
MACDEPLOYQT_OPTIONS = -verbose 3 -dmg
CONFIG += macdeployqt
}

win32 {
WINDEPLOYQT_OPTIONS = -verbose 3
CONFIG += windeployqt

COMPANY = com.amusingtime.csv2rw

DESTDIR = packages/$${COMPANY}/data

# The DISTFILE appears in the "Other files" section of Qt Creator
DISTFILES += \
    Installer/config/config.xml \
    Installer/packages/com.amusingtime.csv2rw/meta/package.xml \
    Installer/packages/com.amusingtime.csv2rw/meta/installscript.qs \
    Installer/packages/com.amusingtime.csv2rw/meta/LICENSE.txt

# Put binarycreator packages files in the correct place
bincre.path=$${OUT_PWD}
bincre.files=Installer/packages
INSTALLS += bincre

# Add new rule that will run the binarycreator (needs adding in Projects/Build Steps)
#QMAKE_EXTRA_TARGETS += binarycreator
#binarycreator.target = binarycreator
#binarycreator.depends = $${PWD}/Installer/config/config.xml packages install
#binarycreator.commands = D:\Qt\QtIFW-3.0.1\bin\binarycreator.exe --offline-only \
#    -c $$shell_path($${PWD}/Installer/config/config.xml) \
#    -p packages CSV2RW.exe

INSTALLERS = Installer/config/config.xml

binarycreator.input = INSTALLERS
binarycreator.name = binarycreator
binarycreator.depends = $$INSTALLERS install install_bincre
binarycreator.output = CSV2RW.exe
binarycreator.variable_out = HEADERS
binarycreator.commands = D:\Qt\QtIFW-3.0.1\bin\binarycreator.exe --offline-only \
    -c ${QMAKE_FILE_IN} -p packages ${QMAKE_FILE_OUT}

QMAKE_EXTRA_COMPILERS += binarycreator
}

DISTFILES += \
    LICENSE.txt \
    RELEASE_NOTES.txt \
    TODO.txt \
    csv2rw_en.ts \
    ob2rw-xsl1.xslt
