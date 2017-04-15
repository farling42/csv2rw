#include "rw_domain.h"
#include <QXmlStreamWriter>
#include <QDebug>

static QMap<QString,RWDomain*> all_domains_by_id;
static QMap<QString,RWDomain*> all_domains_by_name;

RWDomain::RWDomain(QXmlStreamReader *stream, QObject *parent) :
    RWBaseItem(stream, parent)
{
    all_domains_by_id.insert(id(), this);
    all_domains_by_name.insert(name(), this);
}


RWDomain *RWDomain::getDomainById(const QString &domain_id)
{
    return all_domains_by_id.value(domain_id);
}


RWDomain *RWDomain::getDomainByName(const QString &domain_id)
{
    return all_domains_by_name.value(domain_id);
}


QStringList RWDomain::tagNames() const
{
    QStringList result;
    QList<RWBaseItem*> tags = childItems<RWBaseItem*>();
    foreach (RWBaseItem *tag, tags)
    {
        if (tag->elementName().startsWith("tag"))
        {
            result.append(tag->name());
        }
    }
    return result;
}

QString RWDomain::tagId(const QString &tag_name) const
{
    QList<RWBaseItem*> tags = childItems<RWBaseItem*>();
    foreach (RWBaseItem *tag, tags)
    {
        if (tag->elementName().startsWith("tag") && tag->name().compare(tag_name, Qt::CaseInsensitive) == 0)
        {
            return tag->id();
        }
    }
    return QString();
}
