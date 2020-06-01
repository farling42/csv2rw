/*
RWImporter
Copyright (C) 2020 Martin Smith

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

#include "derivedcolumnsproxymodel.h"

#include <QJSEngine>
#include <QBrush>
#include <QtDebug>

struct OneColumn
{
    QString columnName;         // The name of the column.
    QString jsExpression;       // The javascript expression which will be used to generate the values.
    QVector<QVariant> values;   // The values for each row in the column, calculated using jsExpression.

    bool recalculate(QJSEngine *engine, ColumnReader *helper)
    {
        if (!helper->model()) return false;
        bool result = true;
        int size = helper->model()->rowCount();
        values.resize(size);
        for (int row=0; row<size; row++)
        {
            helper->setRow(row);
            QJSValue value = engine->evaluate(jsExpression).toString();
            if (value.isError())
            {
                result = false;
                values[row] = "???";
            }
            else
                values[row] = value.toString();
        }
        return result;
    }
};

struct DerivedColumnsProxyModel::PrivateData
{
    QJSEngine jsEngine;                 // The Javascript Engine to be used for calculating dynamic values.
    QVector<OneColumn> derivedColumns;  // All the added derived-value columns.
    ColumnReader helper;
};

///
/// \brief DerivedColumnsProxyModel::DerivedColumnsProxyModel
/// Within the JS expression, access the value of column using:
///     row.column('column name')
///
/// \param parent
///
DerivedColumnsProxyModel::DerivedColumnsProxyModel(QObject *parent) :
    SuperClass(parent),
    p(new PrivateData)
{
    p->jsEngine.globalObject().setProperty("row", p->jsEngine.newQObject(&p->helper));
    p->helper.setModel(this);
}

DerivedColumnsProxyModel::~DerivedColumnsProxyModel()
{
    delete p;
}

void DerivedColumnsProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    SuperClass::setSourceModel(sourceModel);
    //p->helper.setModel(sourceModel);
    p->helper.resetColumns();

    if (!p->derivedColumns.isEmpty())
    {
        // Now re-calculate all the derived values.
        int firstColumn = sourceModel->columnCount();
        for (OneColumn &col : p->derivedColumns)
        {
            col.recalculate(&p->jsEngine, &p->helper);
        }

        emit dataChanged(index(0, firstColumn),
                         index(sourceModel->rowCount()-1, firstColumn+p->derivedColumns.count()-1));
    }
}

QVariant DerivedColumnsProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //qDebug() << "headerData" << section << orientation << role;

    int column = section - sourceModel()->columnCount();
    if (orientation == Qt::Horizontal && column >= 0 && column < p->derivedColumns.size())
    {
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return p->derivedColumns[column].columnName;
        case Qt::ForegroundRole:
            return QBrush(Qt::darkGreen);
        default:
            return QVariant();
        }
    }
    else
    {
        return SuperClass::headerData(section, orientation, role);
    }
}

QModelIndex DerivedColumnsProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column >= sourceModel()->columnCount())
    {
        QModelIndex result = SuperClass::index(row, 0, parent);
        return createIndex(row, column, result.internalPointer());
    }
    return SuperClass::index(row, column, parent);
}

int DerivedColumnsProxyModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return SuperClass::columnCount(parent) + p->derivedColumns.count();
}

QVariant DerivedColumnsProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int colnum = index.column() - sourceModel()->columnCount();
    if (colnum < 0)
    {
        return SuperClass::data(index, role);
    }
    if (colnum >= p->derivedColumns.count())
        return QVariant();

    OneColumn &column = p->derivedColumns[colnum];
    if (index.row() < 0 || index.row() >= column.values.size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return column.values[index.row()];
    default:
        return QVariant();
    }
}

///
/// \brief DerivedColumnsProxyModel::insertColumns
/// Add a new column to the end of the model (or modifies the rule for an existing column).
/// Initialise the javascript for this column to return an empty string.
/// \param name The name of the new or existing column
/// \param js_expression The JS expression to be used for the new/existing column.
/// \return true if the expression was successfully used on all rows.
///
bool DerivedColumnsProxyModel::setColumn(const QString &name, const QString &js_expression)
{
    // See if we are modifying an existing column.
    int colnumber=0;
    bool result = true;
    for (OneColumn &col : p->derivedColumns)
    {
        if (col.columnName == name)
        {
            col.jsExpression = js_expression;
            result = col.recalculate(&p->jsEngine, &p->helper);
            int fullcolumn = sourceModel()->columnCount() + colnumber;
            emit dataChanged(index(0, fullcolumn), index(rowCount()-1, fullcolumn));
            qDebug() << "setColumn - modified existing column";
            return result;
        }
        colnumber++;
    }
    // Add a new column
    OneColumn newcol;
    newcol.columnName = name;
    newcol.jsExpression = js_expression;
    result = newcol.recalculate(&p->jsEngine, &p->helper);
    //qDebug() << "setColumn - created new column";

    int fullcolumn = sourceModel()->columnCount() + p->derivedColumns.count();
    beginInsertColumns(QModelIndex(), fullcolumn, fullcolumn);
    p->derivedColumns.append(newcol);
    endInsertColumns();
    return result;
}

///
/// \brief DerivedColumnsProxyModel::removeColumns
/// Removes the indicated column from the model.
/// \param name The name of the column to be removed from the model.
/// \return true if the named column was found and removed from the model.
///
bool DerivedColumnsProxyModel::deleteColumn(const QString &name)
{
    int colnum = 0;
    for (OneColumn &col : p->derivedColumns)
    {
        if (col.columnName == name)
        {
            int fullcolumn = sourceModel()->columnCount() + colnum;
            beginRemoveColumns(QModelIndex(), fullcolumn, fullcolumn);
            p->derivedColumns.removeAt(colnum);
            endRemoveColumns();
            return true;
        }
        colnum++;
    }
    return false;
}

QString DerivedColumnsProxyModel::expression(const QString &name) const
{
    for (OneColumn &col : p->derivedColumns)
    {
        if (col.columnName == name)
        {
            return col.jsExpression;
        }
    }
    return "";
}

QStringList DerivedColumnsProxyModel::columnNames() const
{
    QStringList result;
    for (OneColumn &col : p->derivedColumns)
    {
        result.append(col.columnName);
    }
    return result;
}

void DerivedColumnsProxyModel::clearColumns()
{
    beginRemoveColumns(QModelIndex(), sourceModel()->columnCount(), sourceModel()->columnCount() + p->derivedColumns.count());
    p->derivedColumns.clear();
    endRemoveColumns();
}

// Save the configured derived columns to the specified stream
extern QDataStream& operator<<(QDataStream &stream, DerivedColumnsProxyModel &model)
{
    stream << (int)model.p->derivedColumns.count();
    for (auto col : model.p->derivedColumns)
    {
        stream << col.columnName;
        stream << col.jsExpression;
    }
    return stream;
}

extern QDataStream& operator>>(QDataStream &stream, DerivedColumnsProxyModel &model)
{
    int count;
    stream >> count;

    model.beginResetModel();
    model.p->derivedColumns.clear();
    for (int c=0; c<count; c++)
    {
        OneColumn col;
        stream >> col.columnName;
        stream >> col.jsExpression;
        col.recalculate(&model.p->jsEngine, &model.p->helper);
        model.p->derivedColumns.append(col);
    }
    model.endResetModel();
    return stream;
}
