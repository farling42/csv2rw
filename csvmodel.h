#ifndef CSVMODEL_H
#define CSVMODEL_H

/*
CSV2RW
Copyright (C) 2017 Martin Smith

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

#include <QAbstractItemModel>

class QFile;
class QTextStream;

class CsvModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QChar csvSeparator READ fieldSeparator WRITE setSeparator)

public:
    explicit CsvModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void readCSV(QFile &);
    QChar fieldSeparator() const { return is_regional ? QChar() : p_csv_separator; }

public slots:
    void setSeparator(const QChar&);

private:
    QStringList readRowFromCSV(QTextStream &);
    QChar p_csv_separator;
    QStringList headers;
    QList<QStringList> lines;
    bool is_regional;
};

#endif // CSVMODEL_H
