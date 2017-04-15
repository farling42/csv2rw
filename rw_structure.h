#ifndef RW_STRUCTURE_H
#define RW_STRUCTURE_H

#include "rw_base_item.h"

class RWStructure : public RWBaseItem
{
    Q_OBJECT
public:
    RWStructure(QXmlStreamReader *stream, QObject *parent = 0);
};

#endif // RWSTRUCTURE_H
