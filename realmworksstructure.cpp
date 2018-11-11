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

#include "realmworksstructure.h"
#include <QXmlStreamReader>
#include <QDebug>
#include <QAbstractItemModel>
#include <QProgressBar>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QSortFilterProxyModel>

#include "rw_topic.h"

#undef DUMP_ON_LOAD

#ifdef DUMP_ON_LOAD
static void dump_tree(int indent, RWStructureItem *parent)
{
    QString indentation(indent, QChar(QChar::Space));

    qDebug().noquote().nospace() << indentation << *parent;
    for (auto child: parent->childItems<RWStructureItem*>())
    {
        dump_tree(indent +3, child);
    }
}
#endif

/**
 * @brief RealmWorksStructure::loadFile
 * @param file
 */
void RealmWorksStructure::loadFile(QIODevice *device)
{
    QXmlStreamReader reader;
    reader.setDevice(device);

    // Move to the start of the first element
    if (reader.readNextStartElement())
    {
        export_element = read_element(&reader, nullptr);
    }

    if (reader.hasError())
    {
        qWarning() << "Failed to parse XML in structure file: line" <<
                      reader.lineNumber() << ", column" <<
                      reader.columnNumber() << "error:" <<
                      reader.errorString();
        return;
    }
#ifdef DUMP_ON_LOAD
    dump_tree (0, export_element);
#endif

    // Now find the partitions and domains in the structure
    RWStructureItem *main_structure = export_element->findChild<RWStructure*>(QString(), Qt::FindDirectChildrenOnly);
    categories = main_structure->findChildren<RWCategory*>();   // not simply childItems, since some might be sub-categories
    domains = main_structure->childItems<RWDomain*>();
    //qDebug() << "File has" << categories.count() << "categories and" << domains.count() << "domains";
}

RWStructureItem *RealmWorksStructure::read_element(QXmlStreamReader *reader, RWStructureItem *parent)
{
    RWStructureItem *element = nullptr;
    if (reader->name().startsWith("structure"))
        element = new RWStructure(reader, parent);
    else if (reader->name().startsWith("category"))
        element = new RWCategory(reader, parent);
    else if (reader->name().startsWith("domain"))
        element = new RWDomain(reader, parent);
    else if (reader->name().startsWith("facet"))
        element = new RWFacet(reader, parent);
    else if (reader->name().startsWith("partition"))
        element = new RWPartition(reader, parent);
    else if (reader->name().startsWith("text_override") ||
             reader->name().startsWith("overlay"))
        element = new RWStructureItem(reader, parent, /*ignore_for_contents*/ true);
    else if (reader->name().startsWith("export") ||
             reader->name().startsWith("tag") ||
             reader->name().startsWith("definition") ||
             reader->name().startsWith("details") ||
             reader->name().startsWith("requirements") ||
             reader->name().startsWith("legal") ||
             reader->name().startsWith("content_summary") ||
             //reader->name().startsWith("text_override") ||
             reader->name().startsWith("contents") ||
             reader->name().startsWith("description") ||
             reader->name().startsWith("credits") ||
             reader->name().startsWith("summary") ||
             reader->name().startsWith("purpose") )
        element = new RWStructureItem(reader, parent);
    else
    {
        qWarning() << "read_element: unknown element type:" << reader->name();
        element = new RWStructureItem(reader, parent);
    }

    // Now read the rest of this element
    while (!reader->atEnd())
    {
        switch (reader->readNext())
        {
        case QXmlStreamReader::StartElement:
            read_element(reader, element);
            break;

        case QXmlStreamReader::EndElement:
            element->setStructureText(element->structureText().trimmed());
            // Maybe some post-processing?
            element->postLoad();
            return element;

        case QXmlStreamReader::Characters:
            // Add the characters to the end of the text for this element.
            element->setStructureText(element->structureText().append(reader->text()));
            break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::DTD:
            // Skip everything else
            break;
        }
    }

    // Maybe some post-processing?
    element->postLoad();

    return element;
}

/**
 * @brief RealmWorksStructure::writeExportFile
 * @param file
 * @param category
 * @param model
 */

void RealmWorksStructure::writeExportFile(QIODevice *device,
                                          const QList<RWTopic*> &body_topics,
                                          const QAbstractItemModel *model)
{
    QProgressDialog progress;
    progress.setModal(true);
    progress.setWindowTitle("Progress");
    progress.setLabelText("Generating topics/articles...");
    progress.setCancelButton(nullptr);  // hide cancel button
    progress.show();

    QXmlStreamWriter *writer = new QXmlStreamWriter(device);
    // Write out the basics to the file.
    writer->setAutoFormatting(true);
    writer->writeStartDocument();
    writer->writeStartElement("export");
    {
        writer->writeDefaultNamespace(export_element->namespaceUri());
        writer->writeNamespace("http://www.w3.org/2001/XMLSchema-instance", "xsi");
        // Write out some specific attributes.
        writer->writeAttribute("format_version", export_element->attributes().value("format_version").toString());
        writer->writeAttribute("game_system_id", export_element->attributes().value("game_system_id").toString());
        writer->writeAttribute("is_structure_only", "false");

        RWStructure *main_structure = export_element->findChild<RWStructure*>(QString(), Qt::FindDirectChildrenOnly);

        // Write out the correct project definition
        writer->writeStartElement("definition");
        {
            // Replace details with our own details
            writer->writeStartElement("details");
            {
                writer->writeAttribute("name", details_name);
                writer->writeAttribute("import_tag_id", "Tag_1");   // can't be user specified!
                if (!details_version.isEmpty()) writer->writeAttribute("version", details_version);
                if (!details_abbrev.isEmpty()) writer->writeAttribute("abbrev", details_abbrev);

                if (!details_summary.isEmpty()) writer->writeTextElement("summary", details_summary);
                if (!details_description.isEmpty()) writer->writeTextElement("description", details_description);
                if (!details_requirements.isEmpty()) writer->writeTextElement("requirements", details_requirements);
                if (!details_credits.isEmpty()) writer->writeTextElement("credits", details_credits);
                if (!details_legal.isEmpty()) writer->writeTextElement("legal", details_legal);
                if (!details_other_notes.isEmpty()) writer->writeTextElement("other_notes", details_other_notes);
            }
            writer->writeEndElement();  // details

            writer->writeStartElement("content_summary");
            {
                writer->writeAttribute("max_domain_count", QString::number(domains.count()));
                writer->writeAttribute("max_category_count", QString::number(categories.count()));
                writer->writeAttribute("plot_count", "0");

                // Get count of number of topics which will be generated
                int topic_count = 0;
                for (auto topic: body_topics)
                {
                    if (topic->publicName().namefield().modelColumn() >= 0)
                    {
                        if (topic->keyColumn() < 0)
                            topic_count += model->rowCount(); // all entries will be added
                        else
                            topic_count += model->match(/*start*/ model->index(0,topic->keyColumn()),
                                                  /*role*/  Qt::DisplayRole,
                                                  /*value*/ topic->keyValue(),
                                                  /*hits*/  -1,
                                                  /*match*/ Qt::MatchFixedString|Qt::MatchCaseSensitive).size();
                    }
                }
                writer->writeAttribute("topic_count", QString::number(topic_count));
            }
            writer->writeEndElement();  // content_summary
        }
        writer->writeEndElement();  // definition

        // Write out the loaded structure
        main_structure->writeToStructure(writer);

        // Process the CSV to write out the entire RWEXPORT file.
        writer->writeStartElement("contents");

        // Progress is across all rows of the base model
        progress.setMaximum(model->rowCount());

        for (auto topic: body_topics)
        {
            if (topic->publicName().namefield().modelColumn() >= 0)
            {
                if (topic->keyColumn() >= 0)
                {
                    QSortFilterProxyModel proxy;
                    proxy.setSourceModel(const_cast<QAbstractItemModel*>(model));
                    proxy.setFilterKeyColumn(topic->keyColumn());
                    // TODO: need to ignore all possible "regexp" special characters from topic->keyValue
                    proxy.setFilterFixedString(topic->keyValue());
                    writeParentToStructure(progress, writer, topic, &proxy, topic->parents);
                }
                else
                {
                    writeParentToStructure(progress, writer, topic, model, topic->parents);
                }
            }
        }

        writer->writeEndElement(); // contents
    }
    writer->writeEndElement(); // export

    writer->writeEndDocument();
    delete writer;
}

void RealmWorksStructure::saveState(QDataStream &stream)
{
    //qDebug() << "RealmWorksStructure::saveState";
    // Save DETAILS
    stream << details_name;
    stream << details_version;
    stream << details_abbrev;
    stream << details_summary;
    stream << details_description;
    stream << details_requirements;
    stream << details_credits;
    stream << details_legal;
    stream << details_other_notes;
}

void RealmWorksStructure::loadState(QDataStream &stream)
{
    //qDebug() << "RealmWorksStructure::loadState";
    // Load DETAILS
    stream >> details_name;
    stream >> details_version;
    stream >> details_abbrev;
    stream >> details_summary;
    stream >> details_description;
    stream >> details_requirements;
    stream >> details_credits;
    stream >> details_legal;
    stream >> details_other_notes;
}

/**
 * @brief RealmWorksStructure::writeParentToStructure
 * Write out the parent topics, with the subject topics as children of the appropriate lowest parent.
 *
 * @param progress
 * @param writer
 * @param topic_category
 * @param model
 * @param parent_category
 */
void RealmWorksStructure::writeParentToStructure(QProgressDialog &progress,
                                                 QXmlStreamWriter *writer,
                                                 const RWTopic* body_topic,
                                                 const QAbstractItemModel *model,
                                                 const QList<RWTopic*> &parent_topics)
{
    if (parent_topics.isEmpty())
    {
        // No parent topic - so write out the table as individual topics
        int maxrow = model->rowCount();
        progress.setLabelText(body_topic->structure->name());
        for (int row = 0 ; row < maxrow ; row++)
        {
            progress.setValue(row);
            qApp->processEvents();  // for progress dialog

            body_topic->writeToContents(writer, model->index(row, 0));
        }
    }
    else if (parent_topics.first()->publicName().namefield().modelColumn() < 0)
    {
        // The parent has a FIXED STRING
        parent_topics.first()->writeStartToContents(writer, model->index(0,0));
        // Maybe more children to write
        writeParentToStructure(progress, writer, body_topic, model, parent_topics.mid(1));
        writer->writeEndElement();
    }
    else
    {
        // The parent identifies a COLUMN to use to generate a parent for each unique entry
        // in that column.
        QSet<QString> parent_set;
        int parent_column = parent_topics.first()->publicName().namefield().modelColumn();
        for (int row=0; row<model->rowCount(); row++)
        {
            QString name = model->index(row, parent_column).data().toString();
            if (name.isEmpty())
            {
                qDebug() << "row" << row << "has no name";
            }
            parent_set.insert(name);
        }
        // Always put the parents in a predicable (i.e. alphabetical) order
        QList<QString> parent_names = parent_set.toList();
        std::sort(parent_names.begin(), parent_names.end());

        QSortFilterProxyModel proxy;
        proxy.setSourceModel(const_cast<QAbstractItemModel*>(model));
        proxy.setFilterKeyColumn(parent_column);

        // TODO - iterate across unique values in the column
        for (auto name: parent_names)
        {
            // Find the rows which match the parent's name
            // (ignore regexp characters in the name)
            proxy.setFilterFixedString(name);

            parent_topics.first()->writeStartToContents(writer, proxy.index(0,0));
            writeParentToStructure(progress, writer, body_topic, &proxy, parent_topics.mid(1));
            writer->writeEndElement();
        }
    }
}
