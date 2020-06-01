#ifndef JSONMODEL_H
#define JSONMODEL_H

#include <QAbstractItemModel>
#include <QFile>
#include <QJsonDocument>

class JsonModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentArray READ currentArray WRITE setArray)

public:
    explicit JsonModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool readFile(QFile &file);

    QStringList arrayList() const;
    bool setArray(const QString&);
    QString currentArray() const;

private:
    QStringList headers;
    QVector<QVector<QString>> lines;
    QJsonDocument json_doc;
    QStringList array_names;
    QString current_array;
    bool is_regional;
    bool load_array(const QJsonArray &array);
    void clear_data();
};

#endif // JSONMODEL_H
