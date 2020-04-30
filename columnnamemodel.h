#ifndef COLUMNNAMEMODEL_H
#define COLUMNNAMEMODEL_H

#include <QAbstractItemModel>

class ColumnNameModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ColumnNameModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *source_model);

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private slots:
    void columns_added(const QModelIndex &parent, int first, int last);
    void columns_removed(const QModelIndex &parent, int first, int last);
private:
    QAbstractItemModel *source_model{nullptr};
};

#endif // COLUMNNAMEMODEL_H
