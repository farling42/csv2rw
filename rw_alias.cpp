#include "rw_alias.h"

#include <QMetaEnum>
#include <QXmlStreamWriter>

static QMetaEnum case_matching_enum   = QMetaEnum::fromType<RWAlias::CaseMatching>();
static QMetaEnum match_priority_enum  = QMetaEnum::fromType<RWAlias::MatchPriority>();

static int g_alias_id = 1;

RWAlias::RWAlias(QObject *parent) : QObject(parent),
    p_is_auto_accept(false), p_case_matching(Ignore), p_match_priority(Normal),
    p_is_show_nav_pane(true), p_is_true_name(false), p_is_revealed(false)
{

}

void RWAlias::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    QString name = p_name_field.valueString(index);
    if (!name.isEmpty())
    {
        writer->writeStartElement("alias");
        writer->writeAttribute("alias_id", "Alias_" + QString::number(g_alias_id++));
        writer->writeAttribute("name", name);
        if (p_is_auto_accept) writer->writeAttribute("is_auto_accept", "true");
        if (p_case_matching  != Ignore) writer->writeAttribute("case_matching",   case_matching_enum.valueToKey(p_case_matching));
        if (p_match_priority != Normal) writer->writeAttribute("match_priority", match_priority_enum.valueToKey(p_match_priority));
        if (!p_is_show_nav_pane) writer->writeAttribute("is_show_nav_pane", "false");
        if (p_is_true_name) writer->writeAttribute("is_true_name", "true");
        if (p_is_revealed)  writer->writeAttribute("is_revealed",  "true");
        writer->writeEndElement();
    }
}
