#ifndef DATAFIELD_H
#define DATAFIELD_H

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

#include <QObject>
#include <QModelIndex>

class DataField : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int     modelColumn READ modelColumn WRITE setModelColumn)
    Q_PROPERTY(QString fixedText   READ fixedText   WRITE setFixedText)

public:
    explicit DataField(QObject *parent = nullptr) : QObject(parent) {}

    static void setColumnOffset(int offset);

    int  modelColumn() const;
    QString fixedText() const;
    QString valueString(const QModelIndex &index = QModelIndex()) const;
    bool isDefined() const;

public slots:
    void setModelColumn(int column);
    void setFixedText(const QString &text);

private:
    int p_model_column{-1};
    QString p_fixed_text;
    static int p_column_offset;
    friend QDataStream& operator<<(QDataStream&,const DataField&);
    friend QDataStream& operator>>(QDataStream&,DataField&);
};

extern QDataStream& operator<<(QDataStream&,const DataField&);
extern QDataStream& operator>>(QDataStream&,DataField&);

#endif // DATAFIELD_H
