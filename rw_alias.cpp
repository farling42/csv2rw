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

#include "rw_alias.h"

#include <QMetaEnum>
#include <QDataStream>
#include <QXmlStreamWriter>

static QMetaEnum case_matching_enum   = QMetaEnum::fromType<RWAlias::CaseMatching>();
static QMetaEnum match_priority_enum  = QMetaEnum::fromType<RWAlias::MatchPriority>();

static int g_alias_id = 1;

RWAlias::RWAlias(QObject *parent) : QObject(parent),
    p_is_auto_accept(false), p_case_matching(Ignore), p_match_priority(Normal),
    p_is_show_nav_pane(true), p_is_true_name(false), p_is_revealed(false)
{

}

/**
 * @brief RWAlias::writeAttributes
 * Write out the attributes which are common to an Alias and a Topic
 * @param writer
 * @param index
 */
void RWAlias::writeAttributes(QXmlStreamWriter *writer, const QModelIndex &index) const
{
    Q_UNUSED(index)
    if (p_is_auto_accept) writer->writeAttribute("is_auto_accept", "true");
    if (p_case_matching  != Ignore) writer->writeAttribute("case_matching",   case_matching_enum.valueToKey(p_case_matching));
    if (p_match_priority != Normal) writer->writeAttribute("match_priority", match_priority_enum.valueToKey(p_match_priority));
    if (!p_is_show_nav_pane) writer->writeAttribute("is_show_nav_pane", "false");
}


void RWAlias::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index) const
{
    QString name = p_name_field.valueString(index);
    if (!name.isEmpty())
    {
        writer->writeStartElement("alias");
        writer->writeAttribute("alias_id", "Alias_" + QString::number(g_alias_id++));
        writer->writeAttribute("name", name);
        writeAttributes(writer, index);
        if (p_is_true_name) writer->writeAttribute("is_true_name", "true");
        if (p_is_revealed)  writer->writeAttribute("is_revealed",  "true");
        writer->writeEndElement();
    }
}

QDataStream& operator<<(QDataStream &stream, const RWAlias &alias)
{
    stream << alias.p_name_field;
    stream << alias.p_is_auto_accept;
    stream << (int)alias.p_case_matching;
    stream << (int)alias.p_match_priority;
    stream << alias.p_is_show_nav_pane;
    stream << alias.p_is_true_name;
    stream << alias.p_is_revealed;
    return stream;
}

QDataStream& operator>>(QDataStream &stream, RWAlias &alias)
{
    int enum_value;
    stream >> alias.p_name_field;
    stream >> alias.p_is_auto_accept;
    stream >> enum_value;
    alias.p_case_matching = RWAlias::CaseMatching(enum_value);
    stream >> enum_value;
    alias.p_match_priority = RWAlias::MatchPriority(enum_value);
    stream >> alias.p_is_show_nav_pane;
    stream >> alias.p_is_true_name;
    stream >> alias.p_is_revealed;
    return stream;
}
