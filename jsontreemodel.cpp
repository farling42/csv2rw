#include "jsontreemodel.h"

#include <QCollator>
#include <QDebug>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief JsonTreeModel::JsonTreeModel
 *
 * This model reads in a JSON file and presents the data as a flat 2D table.
 */
JsonTreeModel::JsonTreeModel(QObject *parent) : SuperClass(parent)
{

}

#if 0
QVariant JsonTreeModel::data(const QModelIndex &index, int role) const
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
#endif

static void translate_object(QStandardItem *parent_item, const QJsonObject &parent);
static void translate_array(QStandardItem *parent_item, const QJsonArray &array);

static QStandardItem *translate_value(const QJsonValue &value)
{
    QStandardItem *result = new QStandardItem;

    switch(value.type())
    {
    case QJsonValue::Object:
        //qDebug() << "flatten_value Object" << key;
        translate_object(result, value.toObject());
        break;

    case QJsonValue::Array:
        //qDebug() << "flatten_value Array " << key;
        translate_array(result, value.toArray());
        break;

    case QJsonValue::Bool:
        //qDebug() << "flatten_value Bool  " << key << ":=" << value.toBool();
        result->setText(value.toBool() ? "true" : "false");
        break;

    case QJsonValue::Double:
        //qDebug() << "flatten_value Double" << key << ":=" << value.toDouble();
        result->setText(QString::number(value.toDouble()));
        break;

    case QJsonValue::String:
        //qDebug() << "flatten_value String" << key << ":=" << value.toString();
        result->setText(value.toString());
        break;

    case QJsonValue::Null:
    case QJsonValue::Undefined:
        //qDebug() << "flatten_value Null/Undefined" << key;
        break;
    }

    return result;
}

static void translate_array(QStandardItem *parent_item, const QJsonArray &array)
{
    int count = 0;
    for (QJsonValue value : array)
    {
        parent_item->setChild(0, 0, new QStandardItem(QString::number(++count)));
        parent_item->setChild(0, 1, translate_value(value));
    }
}

static void translate_object(QStandardItem *parent_item, const QJsonObject &parent)
{
    for(QJsonObject::const_iterator it=parent.constBegin(); it != parent.constEnd(); ++it)
    {
        parent_item->setChild(0, 0, new QStandardItem(it.key()));
        parent_item->setChild(0, 1, translate_value(it.value()));
    }
}

bool JsonTreeModel::readFile(QFile &file)
{
    // Tell users that the model is about to change.
    beginResetModel();

    // Delete all the old data
    clear();

    qDebug() << "About to start reading JSON from" << file.fileName();

    QByteArray fulldata = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fulldata);
    qDebug() << "Finished reading data from JSON file";

    // Flatten the data
    //ValueList data;
    if (doc.isObject())
    {
        qDebug() << "Decoding top-level object =" << doc.object().size();
        translate_object(invisibleRootItem(), doc.object());
    }
    else if (doc.isArray())
    {
        qDebug() << "Decoding top-level array =" << doc.array().size();
        translate_array(invisibleRootItem(), doc.array());
    }

    if (rowCount() == 0)
    {
        qDebug() << "Data is empty";
        return false;
    }

#if 0
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
#else
    // TODO
#endif

    // Parse the tree into a simple table.
    endResetModel();
    return true;
}
