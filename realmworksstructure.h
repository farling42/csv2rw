#ifndef REALMWORKSSTRUCTURE_H
#define REALMWORKSSTRUCTURE_H

/*
CSV2RW
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

#include <QObject>
#include <QFile>
#include <QXmlStreamReader>
#include <QAbstractItemModel>

#include "rw_domain.h"
#include "rw_category.h"
#include "rw_facet.h"
#include "rw_partition.h"
#include "rw_structure.h"

class QProgressDialog;

class RealmWorksStructure
{
public:
    RealmWorksStructure();

public Q_SLOTS:
    void loadFile(QIODevice*);
    void writeExportFile(QIODevice*, RWCategory *category, QAbstractItemModel *model,
                         RWCategory *parent_category);

    int format_version;
    int game_system_id;
    RWBaseItem *export_element;
    QList<RWCategory*> categories;
    QList<RWDomain*> domains;

private:
    QString namespace_uri;
    RWBaseItem *read_element(QXmlStreamReader *reader, RWBaseItem *parent);
    void writeTopics(QProgressDialog &progress, QXmlStreamWriter *writer, RWCategory *category, QAbstractItemModel *model);
    void writeParentToStructure(QProgressDialog &progress, QXmlStreamWriter *writer, RWCategory *category, QAbstractItemModel *model, RWCategory *parent_category);
};

#endif // REALMWORKSSTRUCTURE_H
