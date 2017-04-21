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
    explicit DataField(QObject *parent = 0) : QObject(parent), p_model_column(-1) {}
    int  modelColumn() const { return p_model_column; }
    QString fixedText() const { return p_fixed_text; }
    QString valueString(const QModelIndex &index = QModelIndex()) const
    {
        if (p_model_column >= 0)
            return index.sibling(index.row(), p_model_column).data().toString();
        else
            return p_fixed_text;
    }

public slots:
    void setModelColumn(int column) { p_model_column = column; }
    void setFixedText(const QString &text) { p_fixed_text = text; }

private:
    int p_model_column;
    QString p_fixed_text;
};

#endif // DATAFIELD_H
