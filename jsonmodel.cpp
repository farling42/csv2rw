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

    // FIXME: Implement me!
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

bool JsonModel::readFile(QFile &file)
{
    // Tell users that the model is about to change.
    beginResetModel();

    // Delete all the old data
    headers.clear();
    lines.clear();

    qDebug() << "About to start reading JSON from" << file.fileName();

    QByteArray fulldata = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fulldata);
    qDebug() << "Finished reading data from JSON file";

    // Flatten the data
    ValueList data;
    if (doc.isObject())
    {
        qDebug() << "Decoding top-level object =" << doc.object().size();
        data = flatten_object(doc.object(), /*istop*/ true);
    }
    else if (doc.isArray())
    {
        qDebug() << "Decoding top-level array =" << doc.array().size();
        data = flatten_array(doc.array());
    }

    if (data.isEmpty())
    {
        qDebug() << "Data is empty";
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

    for (auto &value: data)
    {
        qDebug() << "row" << value.row << ":" << value.column << ":=" << value.value;
    }

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
        lines[value.row-1][headers.indexOf(value.column)] = value.value;
    }

    // Parse the tree into a simple table.
    endResetModel();
    return true;
}
