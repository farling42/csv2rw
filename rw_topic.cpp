/*
RWImporter
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

#include "rw_alias.h"
#include "rw_category.h"
#include "rw_section.h"
#include "rw_snippet.h"
#include "rw_topic.h"
#include "rw_partition.h"
#include "rw_relationship.h"
#include "errordialog.h"
#include "realmworksstructure.h"

#include <QXmlStreamWriter>
#include <QMetaEnum>
#include <QModelIndex>
#include <QDebug>
#include <QDataStream>

#include "rw_contents_item.h"   // TODO - remove this?

static int base_topic_id = 1000;
static QString g_default_name = "no-name";
static QSet<QString> generated_topics;

RWTopic::RWTopic(RWCategory *item, RWContentsItem *parent) :
    RWContentsItem(item, parent),
    category(item),
    p_key_column(-1)
{
}

/**
 * @brief RWTopic::setBaseTopicId
 * @param value
 * This should be called before each new export.
 * Sets the base topic id to be used for topics which are not directly created from a row in the source model.
 */
void RWTopic::initBeforeExport(int model_row_count)
{
    generated_topics.clear();
    base_topic_id = model_row_count + 10;
    ErrorDialog::theInstance()->clear();
}

/**
 * @brief RWTopic::canBeGenerated
 * @return true if a model index has been set on the modelValueForName
 */
bool RWTopic::canBeGenerated() const
{
    // Don't check children, since only the name is needed in a topic.
    return p_public_name.namefield().modelColumn() >= 0;
    //return p_name.namefield().modelColumn() >= 0 && RWBaseItem::canBeGenerated();
}

void RWTopic::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index, bool use_index_topic_id) const
{
    // Don't put topics into the file if they don't match the filter
    if (keyColumn() < 0 || index.sibling(index.row(), keyColumn()).data().toString() == keyValue())
    {
        writeStartToContents(writer, index, use_index_topic_id);
        writer->writeEndElement();  // </topic>
    }
}


void RWTopic::writeStartToContents(QXmlStreamWriter *writer, const QModelIndex &index, bool use_index_topic_id) const
{
    writer->writeStartElement("topic");
    {
        // Use model row for an explicit topic, otherwise allocate a "random" topic id
        QString topic_id;
        if (use_index_topic_id && p_public_name.namefield().modelColumn() >= 0)
        {
            topic_id = index.data(Qt::UserRole).toString();
        }
        else
            topic_id = QString("topic_%1").arg(base_topic_id++);

        QString public_name = p_public_name.namefield().valueString(index);

        if (public_name.isEmpty())
            public_name = g_default_name;
        else if (generated_topics.contains(topic_id))
            // Report the duplicate name
            ErrorDialog::theInstance()->addMessage(tr("Topic '%1' appears in output more than once (the import will fail).").arg(public_name));
        else
            generated_topics.insert(topic_id);

        writer->writeAttribute("topic_id", topic_id);
        if (!category->id().isEmpty()) writer->writeAttribute("category_id", category->id());
        writer->writeAttribute("public_name", public_name);
        QString prefix = p_prefix.valueString(index);
        if (!prefix.isEmpty()) writer->writeAttribute("prefix", prefix);
        QString suffix = p_suffix.valueString(index);
        if (!suffix.isEmpty()) writer->writeAttribute("suffix", suffix);
        p_public_name.writeAttributes(writer, index);
        if (isRevealed()) writer->writeAttribute("is_revealed", "true");

        // Add alias snippets as the first entry within the topic,
        // but only generate aliases for those entries that have a non-empty string.
        // This allows a user to specify several different aliases in the GUI each using a different set of attributes,
        // and then use a different CSV column for each particular alias.
        // (Ensure that all aliases are different, and also different from the topic's main title
        QStringList known_names(public_name);
        for (auto alias: aliases)
        {
            QString name = alias->namefield().valueString(index);
            if (name.isEmpty()) continue;
            if (!known_names.contains(name))
            {
                alias->writeToContents(writer, index);
                known_names.append(name);
            }
            else if (name == public_name)
                ErrorDialog::theInstance()->addMessage(tr("Can't create alias with same name as topic: '%1'").arg(name));
            else
                ErrorDialog::theInstance()->addMessage(tr("Same alias '%1' appears more than once in topic '%2'").arg(name).arg(public_name));
        }

        // Children in the following order:
        //   X x 'alias'
        //   X x 'section'
        //   X x 'tag_assign'
        //   X x connection / dconnection
        //   X x 'topic'
        writeChildrenToContents(writer, index);   // this will do sections

        // Relevant export tag on every topic
        writeExportTag(writer);

        // All possible relationships
        for (auto relationship: relationships)
            relationship->writeToContents(writer, index);

        // No actual TEXT for this element (only children)
        //if (!text().valueString(index).isEmpty()) writer->writeCharacters(text().valueString(index));
    }
}


void RWTopic::setDefaultName(const QString &name)
{
    g_default_name = name;
}

static const QString END_MARKER("--end--");

QDataStream& operator<<(QDataStream &stream, const RWTopic &topic)
{
    //qDebug() << "RWTopic<<" << topic.structure->name();
    // write base class items
    stream << *dynamic_cast<const RWContentsItem*>(&topic);
    // write this class items
    stream << topic.p_public_name.namefield();
    stream << topic.p_prefix;
    stream << topic.p_suffix;
    stream << topic.p_key_column;
    stream << topic.p_key_value;
    // write children
    for (auto elem : topic.findChildren<RWContentsItem*>())
    {
        stream << elem->structure->name();
        stream << QString(elem->metaObject()->className());

        // Use the correct outputter
        if (RWSection *section = qobject_cast<RWSection*>(elem))
            stream << *section;
        else if (RWSnippet *snippet = qobject_cast<RWSnippet*>(elem))
            stream << *snippet;
        else
            stream << *elem;
    }
    stream << END_MARKER;
    // write relationships
    stream << topic.relationships.count();
    for (auto relationship: topic.relationships)
        stream << *relationship;

    // write parents (they won't have children!)
    stream << topic.parents.count();
    for (auto parent : topic.parents)
    {
        stream << parent->structure->name();
        stream << *dynamic_cast<const RWContentsItem*>(parent);
        // write this class items
        stream << parent->p_public_name.namefield();
        stream << parent->p_prefix;
        stream << parent->p_suffix;
        stream << parent->p_key_column;
        stream << parent->p_key_value;
    }
    return stream;
}

QDataStream& operator>>(QDataStream &stream, RWTopic &topic)
{
    //qDebug() << "RWTopic>>" << topic.structure->name();
    // Collect the structure->name() of each child into a look-up table
    QMap<QString,RWContentsItem*> contents;
    for (auto child: topic.findChildren<RWContentsItem*>())
    {
        QString mapname{QString("%1:%2").arg(child->metaObject()->className()).arg(child->structure->name())};
        contents.insert(mapname, child);
    }

    // read base class items
    stream >> *dynamic_cast<RWContentsItem*>(&topic);
    // read this class items
    stream >> topic.p_public_name.namefield();
    stream >> topic.p_prefix;
    stream >> topic.p_suffix;
    stream >> topic.p_key_column;
    stream >> topic.p_key_value;

    // read contents for children
    while (true)
    {
        QString name, classname;
        stream >> name;
        if (name == END_MARKER) break;
        stream >> classname;

        // Find the named child
        RWContentsItem* elem = contents.value(classname + ":" + name, nullptr);
        if (elem == nullptr)
        {
            qWarning() << "RWTopic>> failed to find RWContentsItem for" << classname + ":" + name;
            stream.setStatus(QDataStream::ReadCorruptData);
            return stream;
        }

        // Use correct reader
        if (RWSection *section = qobject_cast<RWSection*>(elem))
            stream >> *section;
        else if (RWSnippet *snippet = qobject_cast<RWSnippet*>(elem))
            stream >> *snippet;
        else
            stream >> *elem;
    }

    // read relationships
    int count;
    stream >> count;
    while (count--)
    {
        RWRelationship *relationship = new RWRelationship;
        stream >> *relationship;
        topic.relationships.append(relationship);
    }

    // read parents (they won't have children!)
    stream >> count;
    while (count--)
    {
        //RWTopic *parent = new RWTopic;
        QString category_name;
        stream >> category_name;

        RWCategory *new_category = nullptr;
        for (auto category: RealmWorksStructure::theInstance()->categories)
        {
            if (category->name() == category_name)
            {
                new_category = category;
                break;
            }
        }
        qDebug() << "reading parent" << category_name << ", found" << new_category;
        if (new_category == nullptr) return stream;
        RWTopic *parent = qobject_cast<RWTopic*>(new_category->createContentsTree());

        stream >> *dynamic_cast<RWContentsItem*>(parent);
        // read this class items
        stream >> parent->p_public_name.namefield();
        stream >> parent->p_prefix;
        stream >> parent->p_suffix;
        stream >> parent->p_key_column;
        stream >> parent->p_key_value;
        topic.parents.append(parent);
    }
    return stream;
}
