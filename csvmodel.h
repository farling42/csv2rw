#ifndef CSVMODEL_H
#define CSVMODEL_H

#include <QAbstractItemModel>

class QTextStream;

class CsvModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CsvModel(QObject *parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void readCSV(QTextStream &);

private:
    QStringList parseCSV(const QString &string);
    QStringList headers;
    QList<QStringList> lines;
};

#endif // CSVMODEL_H
