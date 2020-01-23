#include "yamlmodel.h"

#include "yaml-cpp/yaml.h"

#include <QCollator>
#include <QDebug>
#include <QSet>

typedef QAbstractItemModel SuperClass;


struct ValuePair
{
    QString column;
    QString value;
    int row = 0;
};

typedef QList<ValuePair> ValueList;

/**
 * @brief YamlModel::YamlModel
 *
 * This model uses the yaml-cpp library to parse YAML files.
 *
 * It converts the nested YAML file into a simple two-dimensional table.
 *
 * The top-level array in the YAML file is used to populate each row of the model.
 *
 * The sub-elements within all elements of the top array are used to determine the column names for the model.
 * This requires parsing the entire YAML data twice, once to determine the full list of column names,
 * and then a second parse to populate the now-complete set of known column names.
 */
YamlModel::YamlModel(QObject *parent) : QAbstractItemModel(parent)
{

}

QVariant YamlModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal &&
            role == Qt::DisplayRole &&
            section >= 0 && section < headers.size())
    {
        return headers.at(section);
    }
    return QVariant();
}

QModelIndex YamlModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) return QModelIndex();
    return createIndex(row, column);
}

QModelIndex YamlModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int YamlModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return lines.count();
}

int YamlModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return headers.count();
}

QVariant YamlModel::data(const QModelIndex &index, int role) const
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

static ValueList flatten_tree(YAML::Node parent);

static inline void get_one(ValueList &result, const QString &key, YAML::Node child)
{
    if (child.IsScalar())
    {
        // No maps/sequences as children, so just put in enough numbers to cater for each item.
        result.append({ key, QString::fromStdString(child.as<std::string>()) });
    }
    else
    {
        ValueList part = flatten_tree(child);
        for (ValuePair &value : part)
        {
            value.column.prepend(key + '/');
        }
        result.append(part);
    }
}

static ValueList flatten_tree(YAML::Node parent)
{
    ValueList result;
    int count;

    switch (parent.Type())
    {
    case YAML::NodeType::Null:
    case YAML::NodeType::Undefined:
    case YAML::NodeType::Scalar:
        break;

    case YAML::NodeType::Sequence:
        count = 0;
        // Simple iteration over all entries, numbering each occurrence
        for (YAML::Node child : parent)
        {
            get_one(result, QString::number(++count), child);
        }
        break;

    case YAML::NodeType::Map:
        // Access the Map using an iterator that allows access to each key
        for(YAML::const_iterator it=parent.begin(); it != parent.end(); ++it)
        {
            get_one(result, QString::fromStdString(it->first.as<std::string>()), it->second);
        }
        break;
    }
    return result;
}

bool YamlModel::readFile(const QString &filename)
{
    // Tell users that the model is about to change.
    beginResetModel();

    // Delete all the old data
    headers.clear();
    lines.clear();

    qDebug() << "About to start reading YAML from" << filename;
    YAML::Node config;
    try {
        config = YAML::LoadFile(qPrintable(filename));
    } catch (const std::exception &exc) {
        qCritical() << "Exception raised by YAML parser" << exc.what();
        return false;
    }
    qDebug() << "Finished reading data from YAML file";

    if (config.IsNull())
    {
        return false;
    }
    qDebug() << tr("File has %1 top-level nodes").arg(config.size());

    // Flatten the data
    ValueList data = flatten_tree(config);

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
