#ifndef REALMWORKSSTRUCTURE_H
#define REALMWORKSSTRUCTURE_H

#include <QObject>
#include <QFile>
#include <QXmlStreamReader>
#include <QAbstractItemModel>

#include "rw_domain.h"
#include "rw_category.h"
#include "rw_facet.h"
#include "rw_partition.h"
#include "rw_structure.h"

class RealmWorksStructure
{
public:
    RealmWorksStructure();

public Q_SLOTS:
    void loadFile(QIODevice*);
    void writeExportFile(QIODevice*, RWCategory *category, QAbstractItemModel *model);

    int format_version;
    int game_system_id;
    RWBaseItem *export_element;
    QList<RWCategory*> categories;
    QList<RWDomain*> domains;

private:
    QString namespace_uri;
    RWBaseItem *read_element(QXmlStreamReader *reader, RWBaseItem *parent);
};

#endif // REALMWORKSSTRUCTURE_H
