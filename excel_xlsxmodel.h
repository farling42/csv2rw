#ifndef EXCELXLSXMODEL_H
#define EXCELXLSXMODEL_H

#include <QAbstractTableModel>

class ExcelXlsxModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ExcelXlsxModel(const QString &filename, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    struct PrivateData *p;
};

#endif // EXCELXLSXMODEL_H
