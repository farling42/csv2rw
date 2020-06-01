#include "jsonmodel.h"

#include <QCollator>
#include <QDebug>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

typedef QAbstractItemModel SuperClass;


struct ValuePair
{
    QString column;
    QString value;
    int row = 0;
};

typedef QList<ValuePair> ValueList;

/**
 * @brief JsonModel::JsonModel
 *
 * This model reads in a JSON file and presents the data as a flat 2D table.
 */
JsonModel::JsonModel(QObject *parent) : QAbstractItemModel(parent)
{

}

QVariant JsonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal &&
            role == Qt::DisplayRole &&
            section >= 0 && section < headers.size())
    {
        return headers.at(section);
    }
    return QVariant();
}

QModelIndex JsonModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) return QModelIndex();
    return createIndex(row, column);
}

QModelIndex JsonModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int JsonModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return lines.count();
}

int JsonModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return headers.count();
}

QVariant JsonModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid())
        return QVariant();

    if ((role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole) &&
            hasIndex(index.row(), index.column()))
    {
        return lines.at(index.row()).at(index.column());
    }
    else if (role == Qt::UserRole && index.isValid())
    {
        return QString("topic_%1").arg(index.row()+1);
    }
    return QVariant();
}

static ValueList flatten_object(const QJsonObject &parent, bool istop=false);
static ValueList flatten_value(const QString &key, const QJsonValue &value, bool istop=false);

static ValueList flatten_array(const QJsonArray &array)
{
    ValueList result;
    int count = 0;
    for (QJsonValue value : array)
    {
        //qDebug() << "flatten_array" << count+1;
        result.append(flatten_value(QString::number(++count), value));
    }
    return result;
}

static ValueList flatten_value(const QString &key, const QJsonValue &value, bool istop)
{
    ValueList result;

    switch(value.type())
    {
    case QJsonValue::Object:
        //qDebug() << "flatten_value Object" << key;
        result = flatten_object(value.toObject(), istop);
        //qDebug() << "flatten_value Object" << key << "marking";
        if (!istop) // If top item in JSON File is an object, don't add it to the name.
        {
            for (ValuePair &value: result)
            {
                value.column.prepend(key + '/');
            }
        }
        //qDebug() << "flatten_value Object" << key << "done";
        break;

    case QJsonValue::Array:
        //qDebug() << "flatten_value Array " << key;
        result = flatten_array(value.toArray());
        //qDebug() << "flatten_value Array " << key << "marking";
        if (!istop)
        {
            for (ValuePair &value: result)
            {
                value.column.prepend(key + '/');
            }
        }
        //qDebug() << "flatten_value Array " << key << "done";
        break;

    case QJsonValue::Bool:
        //qDebug() << "flatten_value Bool  " << key << ":=" << value.toBool();
        result.append({ key, value.toBool() ? "true" : "false" });
        break;

    case QJsonValue::Double:
        //qDebug() << "flatten_value Double" << key << ":=" << value.toDouble();
        result.append({ key, QString::number(value.toDouble()) });
        break;

    case QJsonValue::String:
        //qDebug() << "flatten_value String" << key << ":=" << value.toString();
        result.append({ key, value.toString() });
        break;

    case QJsonValue::Null:
    case QJsonValue::Undefined:
        //qDebug() << "flatten_value Null/Undefined" << key;
        break;
    }

    return result;
}

static ValueList flatten_object(const QJsonObject &parent, bool istop)
{
    ValueList result;

    //int count=0;
    for(QJsonObject::const_iterator it=parent.constBegin(); it != parent.constEnd(); ++it)
    {
        //qDebug() << "flatten_object" << ++count;
        result.append(flatten_value(it.key(), it.value(), istop));
    }

    return result;
}

void JsonModel::clear_data()
{
    beginResetModel();
    headers.clear();
    lines.clear();
    array_names.clear();
    endResetModel();
}

bool JsonModel::load_array(const QJsonArray &array)
{
    // Tell users that the model is about to change.
    beginResetModel();

    // Delete all the old data
    headers.clear();
    lines.clear();

    ValueList data = flatten_array(array);

    if (data.isEmpty())
    {
        qDebug() << "Data is empty";
        endResetModel();
        return false;
    }

    qDebug() << "Flattening finished...";
    // Remove the top-level column from each entry, and transfer to the "row" field.
    for (ValuePair &value : data)
    {
        int pos = value.column.indexOf('/');
        if (pos < 0) continue;
        value.row = value.column.mid(0,pos).toInt();
        value.column.remove(0, pos+1);
    }

#ifdef DEBUG
    for (auto &value: data)
    {
        qDebug() << "row" << value.row << ":" << value.column << ":=" << value.value;
    }
#endif

    // Collect all the column names: We don't want a sequence number at the top-level.
    for (const ValuePair &value : data)
        headers.append(value.column);

    // Use QCollator so that numeric entries are sorted sensibly.
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(headers.begin(), headers.end(), collator);
    headers.removeDuplicates();

    // Set lines to the correct 2D size
    lines.resize(data.last().row);
    for (QVector<QString> &line : lines)
        line.resize(headers.size());

    // Transfer data to the relevant cells in the table.
    for (const ValuePair &value: data)
    {
        if (value.row <= 0 || value.row > lines.count())
        {
            qCritical() << "Invalid row" << value.row << ", max" << lines.count() << "for value" << value.value;
            continue;
        }

        int col = headers.indexOf(value.column);
        if (col < 0)
        {
            qCritical() << "Failed to find column" << value.column << "for value" << value.value;
            continue;
        }
        if (col >= lines[value.row-1].size())
        {
            qCritical() << "col" << col << "won't fit in array of size" << lines[value.row-1].size() << "for column" << value.column << "for value" << value.value;
            continue;
        }

        lines[value.row-1][headers.indexOf(value.column)] = value.value;
    }

    // Parse the tree into a simple table.
    endResetModel();
    return true;
}


bool JsonModel::readFile(QFile &file)
{
    qDebug() << "About to start reading JSON from" << file.fileName();

    clear_data();

    QByteArray fulldata = file.readAll();
    json_doc = QJsonDocument::fromJson(fulldata);
    qDebug() << "Finished reading data from JSON file";

    // Flatten the data
    if (json_doc.isArray())
    {
        qDebug() << "Decoding top-level array =" << json_doc.array().size();
        current_array.clear();
        return load_array(json_doc.array());
    }

    if (json_doc.isObject())
    {
        qDebug() << "Decoding top-level object =" << json_doc.object().size();
        //
        // Find biggest array child of the parent object
        //
        const QJsonObject &parent = json_doc.object();
        int largest_size = 0;
        QString largest_array;
        for(QJsonObject::const_iterator it=parent.constBegin(); it != parent.constEnd(); ++it)
        {
            //qDebug() << "flatten_object" << ++count;
            if (it.value().isArray())
            {
                array_names.append(it.key());

                QJsonArray this_array = it.value().toArray();
                qDebug() << "Array" << it.key() << "has size" << this_array.size();

                if (this_array.size() > largest_size)
                {
                    largest_size = this_array.size();
                    largest_array = it.key();
                    qDebug() << "Largest array" << largest_array << "of size" << largest_size;
                }
            }
        }

        if (largest_size == 0)
        {
            qCritical() << "No array found in data";
            return false;
        }

        return setArray(largest_array);
    }

    qCritical() << "File does not have top-level array or object";
    return false;
}

///
/// \brief JsonModel::arrayList
/// \return The list of arrays available in the currently selected file
///
QStringList JsonModel::arrayList() const
{
    return array_names;
}

///
/// \brief JsonModel::setArray
/// Loads the specified array from the current file.
///
/// \param array_name
/// \return
///
bool JsonModel::setArray(const QString &array_name)
{
    const QJsonObject &parent = json_doc.object();
    for(QJsonObject::const_iterator it=parent.constBegin(); it != parent.constEnd(); ++it)
    {
        if (it.key() == array_name && it.value().isArray())
        {
            qDebug() << "setArray is loading" << it.key();
            current_array = array_name;
            return load_array(it.value().toArray());
        }
    }

    // Failed to find the named array
    return false;
}

QString JsonModel::currentArray() const
{
    return current_array;
}
