#include "columnnamemodel.h"

ColumnNameModel::ColumnNameModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

void ColumnNameModel::setSourceModel(QAbstractItemModel *model)
{
    beginResetModel();
    if (source_model)
    {
        disconnect(source_model, &QAbstractItemModel::modelReset,               this, &ColumnNameModel::modelReset);
        disconnect(source_model, &QAbstractItemModel::columnsAboutToBeInserted, this, &ColumnNameModel::rowsAboutToBeInserted);
        disconnect(source_model, &QAbstractItemModel::columnsInserted,          this, &ColumnNameModel::rowsInserted);
        disconnect(source_model, &QAbstractItemModel::columnsAboutToBeRemoved,  this, &ColumnNameModel::rowsAboutToBeRemoved);
        disconnect(source_model, &QAbstractItemModel::columnsRemoved,           this, &ColumnNameModel::rowsRemoved);
    }

    source_model = model;

    if (source_model)
    {
        connect(source_model, &QAbstractItemModel::modelReset,               this, &ColumnNameModel::modelReset);
        connect(source_model, &QAbstractItemModel::columnsAboutToBeInserted, this, &ColumnNameModel::rowsAboutToBeInserted);
        connect(source_model, &QAbstractItemModel::columnsInserted,          this, &ColumnNameModel::rowsInserted);
        connect(source_model, &QAbstractItemModel::columnsAboutToBeRemoved,  this, &ColumnNameModel::rowsAboutToBeRemoved);
        connect(source_model, &QAbstractItemModel::columnsRemoved,           this, &ColumnNameModel::rowsRemoved);
    }

    endResetModel();
}

QModelIndex ColumnNameModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) return QModelIndex();
    return createIndex(row, column);
}

QModelIndex ColumnNameModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

///
/// \brief ColumnNameModel::flags
/// Adds ItemIsDragEnabled so that column name can be dragged onto the topic snippets.
/// \param index
/// \return
///
Qt::ItemFlags ColumnNameModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = QAbstractItemModel::flags(index);
    if (index.isValid())
    {
        result |= Qt::ItemIsDragEnabled;
    }
    return result;
}

int ColumnNameModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return source_model ? source_model->columnCount() : 0;
}

int ColumnNameModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() || source_model == nullptr)
        return 0;
    else
        return 1;
}

QVariant ColumnNameModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != 0 || source_model == nullptr)
        return QVariant();
    else
        return source_model->headerData(index.row(), Qt::Horizontal, role);
}


void ColumnNameModel::columns_added(const QModelIndex &parent, int first, int last)
{
    beginInsertColumns(parent, first, last);
    endInsertColumns();
}

void ColumnNameModel::columns_removed(const QModelIndex &parent, int first, int last)
{
    beginRemoveColumns(parent, first, last);
    endRemoveColumns();
}
