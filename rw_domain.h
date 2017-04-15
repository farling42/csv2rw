#ifndef RW_DOMAIN_H
#define RW_DOMAIN_H

#include "rw_base_item.h"

class QXmlStreamWriter;

typedef RWBaseItem DomainTag;

class RWDomain : public RWBaseItem
{
    Q_OBJECT
public:
    RWDomain(QXmlStreamReader *stream, QObject *parent = 0);
    static RWDomain *getDomainById(const QString &domain_id);
    static RWDomain *getDomainByName(const QString &domain_id);
    QStringList tagNames() const;
    QString tagId(const QString &tag_name) const;
};

#endif // RW_DOMAIN_H
