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

#include "rw_domain.h"
#include <QXmlStreamWriter>
#include <QDebug>

static QMap<QString,RWDomain*> all_domains_by_id;
static QMap<QString,RWDomain*> all_domains_by_name;

RWDomain::RWDomain(QXmlStreamReader *stream, QObject *parent) :
    RWStructureItem(stream, parent)
{
    all_domains_by_id.insert(id(), this);
    all_domains_by_name.insert(name(), this);
}


RWDomain *RWDomain::getDomainById(const QString &domain_id)
{
    if (domain_id.isEmpty()) return 0;
    return all_domains_by_id.value(domain_id);
}


RWDomain *RWDomain::getDomainByName(const QString &domain_id)
{
    return all_domains_by_name.value(domain_id);
}


QStringList RWDomain::tagNames() const
{
    QStringList result;
    for (auto tag: childItems<RWStructureItem*>())
    {
        if (tag->structureElement().startsWith("tag"))
        {
            result.append(tag->name());
        }
    }
    return result;
}

QString RWDomain::tagId(const QString &tag_name) const
{
    for (auto tag: childItems<RWStructureItem*>())
    {
        if (tag->structureElement().startsWith("tag") &&
                tag->name().compare(tag_name, Qt::CaseInsensitive) == 0)
        {
            return tag->id();
        }
    }
    return QString();
}

RWContentsItem *RWDomain::createContentsItem(RWContentsItem *parent)
{
    return new RWContentsItem(this, parent);
}
