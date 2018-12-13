/*
RWImporter
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

#include "datafield.h"

#include <QDataStream>

int DataField::p_column_offset{0};

void DataField::setColumnOffset(int offset)
{
    p_column_offset = offset;
}

int  DataField::modelColumn() const
{
    return p_column_offset + p_model_column;
}

QString DataField::fixedText() const
{
    return p_fixed_text;
}

QString DataField::valueString(const QModelIndex &index) const
{
    if (p_model_column >= 0)
        return index.sibling(index.row(), modelColumn()).data().toString();
    else
        return p_fixed_text;
}

bool DataField::isDefined() const
{
    return p_model_column >= 0 || !p_fixed_text.isEmpty();
}

void DataField::setModelColumn(int column)
{
    p_model_column = column;
}

void DataField::setFixedText(const QString &text)
{
    p_fixed_text = text;
}

QDataStream& operator<<(QDataStream &stream, const DataField &item)
{
    stream << item.p_model_column;
    stream << item.p_fixed_text;
    return stream;
}

QDataStream& operator>>(QDataStream &stream, DataField &item)
{
    stream >> item.p_model_column;
    stream >> item.p_fixed_text;
    return stream;
}
