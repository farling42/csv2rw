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

#include "csvmodel.h"
#include <QtCore/QDebug>
#include <QtCore/QFile>

CsvModel::CsvModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QVariant CsvModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal &&
            role == Qt::DisplayRole &&
            section >= 0 && section < headers.size())
    {
        return headers.at(section);
    }
    return QVariant();

}

QModelIndex CsvModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) return QModelIndex();
    return createIndex(row, column);
}

QModelIndex CsvModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int CsvModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return lines.count();
}

int CsvModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return headers.count();
}

QVariant CsvModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid())
        return QVariant();

    // FIXME: Implement me!
    if ((role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole) &&
            hasIndex(index.row(), index.column()))
    {
        // Reading in the CSV ensures that the row will have
        // the correct number of entries
        return lines.at(index.row()).at(index.column());
    }
    return QVariant();
}

void CsvModel::readCSV(QFile &file)
{
    QTextStream stream(&file);

    QString line;
    beginResetModel();

    // Erase old data
    headers.clear();
    lines.clear();

    // Get headers
    headers = parseCSV(stream);
    if (headers.size() == 0)
    {
        qWarning("No lines in source file");
    }
    else
    {
        while (!stream.atEnd())
        {
            QStringList fields = parseCSV(stream);
            // Ensure this row of the table is the same length as the header row
            if (fields.size() > 0)
            {
                while (fields.size() < headers.size()) fields.append(QString());
                lines.append(fields);
            }
        }
    }
    endResetModel();
}


/**
 * @brief MainWindow::parseCSV
 * Parses the supplied single line from a CSV file and returns the list of individual cells.
 *
 * @param string the contents of a full row from the CSV file
 * @return a list of strings for each field on the row
 */
QStringList CsvModel::parseCSV(QTextStream &stream)
{
    enum State {Normal, Quote} state = Normal;
    QStringList fields;
    QString value;
    QString buffer;
    bool done=false;

    int pos = 0;
    if (!stream.readLineInto(&buffer)) return QStringList();

    //qDebug() << "parseCSV";
    while (!done)
    {
        // Maybe read another line into the buffer (handle blank lines inside quotes)
        while (pos == buffer.size())
        {
            // We were at end-of-line, so if in Normal mode then stop reading the current record.
            if (state == Normal || stream.atEnd())
            {
                done = true;
                break;
            }

            // If we've already read some fields, then add a line-feed to the output
            value += QChar::LineFeed;
            if (!stream.readLineInto(&buffer))
            {
                done = true;
                break;
            }
            pos = 0;
        }
        if (done) break;

        QChar current = buffer[pos++];

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value);
                //qDebug() << "\tfield:" << value;
                value.clear();
            }

            // Double-quote
            else if (current == '"')
                state = Quote;

            // Other character
            else
                value += current;
        }

        // In-quote state
        else if (state == Quote)
        {
            if (current == '"')
            {
                // A double double-quote?
                if (pos < buffer.size() && buffer.at(pos) == '"')
                {
                    value += '"';
                    pos++;
                }
                else
                    state = Normal;
            }
            else
                // Other character
                value += current;
        }
    }
    if (!value.isEmpty())
    {
        fields.append(value);
        //qDebug() << "\tfield:" << value;
    }

    return fields;
}
