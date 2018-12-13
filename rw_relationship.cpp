/*
RWImporter
Copyright (C) 2018 Martin Smith

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

#include "rw_relationship.h"
#include <QMetaEnum>
#include <QXmlStreamWriter>
#include <QDataStream>
#include <QPointer>
#include <QAbstractProxyModel>
#include "rw_domain.h"
#include "rw_topic.h"
#include "datafield.h"

static const QMap<QString,QString> attitudeId{
    { "Hostile", "-3" },
    { "Angry", "-2" },
    { "Annoyed", "-1" },
    { "Neutral", "0" },
    { "Pleased", "1" },
    { "Friendly", "2" },
    { "Gracious", "3" }
};

static QMetaEnum nature_enum   = QMetaEnum::fromType<RWRelationship::Nature>();
static QMetaEnum attitude_enum = QMetaEnum::fromType<RWRelationship::Attitude>();

// If a new structure file is loaded, then the following two pointers should get reset to nullptr automatically
static QPointer<RWDomain> comprises_domain;
static QPointer<RWDomain> generic_domain;


RWRelationship::RWRelationship(QObject *parent) : QObject(parent)
{
}


void RWRelationship::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index) const
{
    if (p_this_link.modelColumn() < 0 || p_other_link.modelColumn() < 0) return;

    // Get actual value from the current ROW
    QString value_to_match = p_this_link.valueString(index);
    if (value_to_match.isEmpty()) return;

    // Find the base model
    const QAbstractItemModel *model = index.model();
    while (const QAbstractProxyModel *proxy = qobject_cast<const QAbstractProxyModel*>(model))
        model = proxy->sourceModel();

    // Create one connection for each matching topic
    const QModelIndexList topics = model->match(model->index(0, p_other_link.modelColumn()), Qt::DisplayRole, value_to_match, /*hits*/ -1, Qt::MatchExactly);
    for (auto other_index : topics)
    {
        writer->writeStartElement("connection");
        writer->writeAttribute("target_id", other_index.data(Qt::UserRole).toString());
        writer->writeAttribute("nature", nature_enum.valueToKey(nature));

        switch (nature)
        {
        case Master_To_Minion:
        //case Minion_To_Master:
            // Requires tag from "Comprises Relationship Types" domain
            if (comprises_domain.isNull()) comprises_domain = RWDomain::getDomainByName("Comprises Relationship Types");
            writer->writeAttribute("qualifier_tag_id", comprises_domain->tagId(qualifier_tag_name));
            writer->writeAttribute("qualifier", qualifier_tag_name);
            break;

        case Generic:
            // Requires tag from "Generic Relationship Types" domain
            if (generic_domain.isNull()) generic_domain = RWDomain::getDomainByName("Generic Relationship Types");
            writer->writeAttribute("qualifier_tag_id", generic_domain->tagId(qualifier_tag_name));
            writer->writeAttribute("qualifier", qualifier_tag_name);
            break;

        case Public_Attitude_Towards:
        case Private_Attitude_Towards:
            writer->writeAttribute("rating", attitudeId.value(qualifier_tag_name));
            writer->writeAttribute("attitude", qualifier_tag_name);
            break;

        case Parent_To_Offspring:
        //case Offspring_To_Parent:
        case Arbitrary:
        case Union:
            // No additional attributes
            break;
        }

        if (is_revealed) writer->writeAttribute("is_revealed", "true");
        writer->writeEndElement();
    }
}


extern QDataStream& operator<<(QDataStream &stream, const RWRelationship &r)
{
    stream << int(r.nature);
    stream << int(r.attitude);
    stream << r.qualifier_tag_name;
    stream << r.is_revealed;
    stream << r.p_this_link;
    stream << r.p_other_link;
    return stream;
}


extern QDataStream& operator>>(QDataStream &stream, RWRelationship &r)
{
    int n, a;
    stream >> n;
    stream >> a;
    r.nature = RWRelationship::Nature(n);
    r.attitude = RWRelationship::Attitude(a);
    stream >> r.qualifier_tag_name;
    stream >> r.is_revealed;
    stream >> r.p_this_link;
    stream >> r.p_other_link;
    return stream;
}
