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
#include <QtCore/QTextStream>

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
    if ((role == Qt::DisplayRole || role == Qt::EditRole) &&
            hasIndex(index.row(), index.column()))
    {
        // Reading in the CSV ensures that the row will have
        // the correct number of entries
        return lines.at(index.row()).at(index.column());
    }
    return QVariant();
}

void CsvModel::readCSV(QTextStream &stream)
{
    QString line;
    beginResetModel();

    // Erase old data
    headers.clear();
    lines.clear();

    // Get headers
    if (stream.readLineInto(&line))
    {
        headers = parseCSV(line);
        while (stream.readLineInto(&line))
        {
            QStringList fields = parseCSV(line);
            // Ensure this row of the table is the same length as the header row
            while (fields.size() < headers.size()) fields.append(QString());
            lines.append(fields);
        }
    }
    else
        qWarning("No lines in source file");
    endResetModel();
}


/**
 * @brief MainWindow::parseCSV
 * Parses the supplied single line from a CSV file and returns the list of individual cells.
 *
 * @param string the contents of a full row from the CSV file
 * @return a list of strings for each field on the row
 */
QStringList CsvModel::parseCSV(const QString &string)
{
    // from http://stackoverflow.com/questions/27318631/parsing-through-a-csv-file-in-qt
    enum State {Normal, Quote} state = Normal;
    QStringList fields;
    QString value;

    for (int i = 0; i < string.size(); i++)
    {
        QChar current = string.at(i);

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value);
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
            // Another double-quote
            if (current == '"')
            {
                if (i+1 < string.size())
                {
                    QChar next = string.at(i+1);

                    // A double double-quote?
                    if (next == '"')
                    {
                        value += '"';
                        i++;
                    }
                    else
                        state = Normal;
                }
            }

            // Other character
            else
                value += current;
        }
    }
    if (!value.isEmpty())
        fields.append(value);

    return fields;
}
