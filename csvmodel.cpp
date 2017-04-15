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
