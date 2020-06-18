#ifndef DERIVEDCOLUMNSPROXYMODEL_H
#define DERIVEDCOLUMNSPROXYMODEL_H

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
#include <QIdentityProxyModel>
#include <QJSValue>

class DerivedColumnsProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    explicit DerivedColumnsProxyModel(QObject *parent = nullptr);
    ~DerivedColumnsProxyModel();

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Manage derived columns
    bool setColumn(const QString &name, const QString &js_expression);
    bool deleteColumn(const QString &name);
    QString expression(const QString &name) const;
    QStringList columnNames() const;
    void clearColumns();

private:
    Q_DISABLE_COPY(DerivedColumnsProxyModel)
    typedef QIdentityProxyModel SuperClass;
    struct PrivateData;
    PrivateData *p;
    friend QDataStream& operator>>(QDataStream&, DerivedColumnsProxyModel&);
    friend QDataStream& operator<<(QDataStream&, DerivedColumnsProxyModel&);
};


extern QDataStream& operator>>(QDataStream&, DerivedColumnsProxyModel&);
extern QDataStream& operator<<(QDataStream&, DerivedColumnsProxyModel&);


class ColumnReader : public QObject
{
    Q_OBJECT
public:
    ColumnReader(QObject *parent = nullptr) : QObject(parent) {};

    void setModel(QAbstractItemModel *themodel)
    {
        p_model = themodel;
        resetColumns();
    }
    void resetColumns()
    {
        columns.clear();
        if (p_model == nullptr) return;
        int count=p_model->columnCount();
        for (int column = 0; column < count; column++)
        {
            columns.insert(p_model->headerData(column, Qt::Horizontal).toString(), column);
        }
    }

    inline QAbstractItemModel *model()const { return p_model; }

    ///
    /// \brief setRow
    /// Sets the current row for use by the column() function
    /// \param row
    ///
    inline void setRow(int row)
    {
        therow = row;
    }
    inline int row() const { return therow; }
    ///
    /// \brief column
    /// Invoked from a JS script to read the current value of a column.
    /// \param name The name of the column whose value is required.
    /// \return The string of the specified column (in the current row).
    ///
    Q_INVOKABLE QJSValue column(const QString &name)
    {
        if (p_model == nullptr) return "";
        int col = columns.value(name, -1);
        if (col == -1) return QJSValue();
        return p_model->index(therow, col).data().toString();
    }
    ///
    /// \brief hasColumn
    /// Invoked from a JS script to detect the presence of the named column.
    /// \param name the name of the column whose existence is being tested.
    /// \return true if the named column exists in the model.
    ///
    Q_INVOKABLE QJSValue hasColumn(const QString &name)
    {
        return columns.contains(name);
    }
private:
    QAbstractItemModel *p_model{nullptr};
    QMap<QString,int> columns;
    int therow{-1};
};

#endif // DERIVEDCOLUMNSPROXYMODEL_H
