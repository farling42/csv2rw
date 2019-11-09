#ifndef REALMWORKSSTRUCTURE_H
#define REALMWORKSSTRUCTURE_H

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

#include <QObject>
#include <QDataStream>
#include <QXmlStreamReader>
#include <QAbstractItemModel>

#include "rw_domain.h"
#include "rw_category.h"
#include "rw_facet.h"
#include "rw_partition.h"
#include "rw_structure.h"

class QProgressDialog;
class QDataStream;

class RealmWorksStructure : public QObject
{
    Q_OBJECT

public:
    explicit RealmWorksStructure(QObject *parent = nullptr);

public Q_SLOTS:
    void loadFile(QIODevice*);
    void writeExportFile(QIODevice*,
                         const QList<RWTopic*> &body_topics,
                         const QAbstractItemModel *model);

    void saveState(QDataStream&);
    void loadState(QDataStream&);

public:
    static RealmWorksStructure *theInstance();

    int format_version;
    int game_system_id;
    RWStructureItem *export_element{nullptr};
    QList<RWCategory*> categories;
    QList<RWDomain*> domains;

    QString details_name;
    //QString details_import_tag_id;  // not easily user-specified
    QString details_version;
    QString details_abbrev;

    QString details_summary;
    QString details_description;
    QString details_requirements;
    QString details_credits;
    QString details_legal;
    QString details_other_notes;

    int formatVersion() const;
    void forceFormatVersion(int version);

signals:
    void modificationDone();

private:
    QString namespace_uri;
    int force_format_version{-1};
    int orig_format_version{-1};
    RWStructureItem *read_element(QXmlStreamReader *reader, RWStructureItem *parent);
    void writeParentToStructure(QProgressDialog &progress, QXmlStreamWriter *writer,
                                const RWTopic* body_topic,
                                const QAbstractItemModel *model,
                                const QList<RWTopic*> &parent_topics);
};

#endif // REALMWORKSSTRUCTURE_H
