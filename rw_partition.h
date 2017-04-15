#ifndef RW_PARTITION_H
#define RW_PARTITION_H

#include "rw_base_item.h"

class QXmlStreamWriter;
class RWFacet;

class RWPartition : public RWBaseItem
{
    Q_OBJECT
public:
    RWPartition(QXmlStreamReader *stream, QObject *parent = 0);
    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);
};

#endif // RWPARTITION_H
