#ifndef YAMLMODEL_H
#define YAMLMODEL_H

#include <QAbstractItemModel>
#include <QFile>

class YamlModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit YamlModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool readFile(const QString &filename);

private:
    QStringList headers;
    QVector<QVector<QString>> lines;
    bool is_regional;
};

#endif // YAMLMODEL_H
