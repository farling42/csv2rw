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

static ValueList flatten_object(const QJsonObject &parent);
static ValueList flatten_value(const QString &key, const QJsonValue &value);

static ValueList flatten_array(const QJsonArray &array)
{
    ValueList result;
    int count = 0;
    for (QJsonValue value : array)
    {
        result.append(flatten_value(QString::number(++count), value));
    }
    return result;
}

static ValueList flatten_value(const QString &key, const QJsonValue &value)
{
    ValueList result;

    switch(value.type())
    {
    case QJsonValue::Object:
        result = flatten_object(value.toObject());
        for (ValuePair &value: result)
        {
            value.column.prepend(key + '/');
        }
        break;

    case QJsonValue::Array:
        result = flatten_array(value.toArray());
        for (ValuePair &value: result)
        {
            value.column.prepend(key + '/');
        }
        break;

    case QJsonValue::Bool:
        result.append({ key, value.toBool() ? "true" : "false" });
        break;

    case QJsonValue::Double:
        result.append({ key, QString::number(value.toDouble()) });
        break;

    case QJsonValue::String:
        result.append({ key, value.toString() });
        break;

    case QJsonValue::Null:
    case QJsonValue::Undefined:
        break;
    }

    return result;
}

static ValueList flatten_object(const QJsonObject &parent)
{
    ValueList result;

    for(QJsonObject::const_iterator it=parent.constBegin(); it != parent.constEnd(); ++it)
    {
        result.append(flatten_value(it.key(), it.value()));
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
        data = flatten_object(doc.object());
    }
    else if (doc.isArray())
    {
        qDebug() << "Decoding top-level object =" << doc.array().size();
        data = flatten_array(doc.array());
    }

    if (data.isEmpty())
    {
        qDebug() << "Data is empty";
        return false;
    }

    // Remove the top-level column from each entry, and transfer to the "row" field.
    for (ValuePair &value : data)
    {
        int pos = value.column.indexOf('/');
        if (pos < 0) continue;
        value.row = value.column.mid(0,pos).toInt();
        value.column.remove(0, pos+1);
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
