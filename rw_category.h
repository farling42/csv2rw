#ifndef RW_CATEGORY_H
#define RW_CATEGORY_H

#include "rw_base_item.h"

class QXmlStreamWriter;
class RWPartition;

class RWCategory : public RWBaseItem
{
    Q_OBJECT
public:
    RWCategory(QXmlStreamReader *stream, QObject *parent = 0);

    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);
    virtual void postLoad();
};

#endif // RWCATEGORY_H
