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
#include "realmworksstructure.h"
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

QVariant DataField::value(const QModelIndex &index) const
{
    if (p_model_column >= 0)
        return index.sibling(index.row(), modelColumn()).data();
    else
        return p_fixed_text;
}

bool DataField::isDefined() const
{
    return p_model_column >= 0 || !p_fixed_text.isEmpty();
}

void DataField::setModelColumn(int column)
{
    if (p_model_column == column) return;

    p_model_column = column;
    p_fixed_text.clear();
    emit RealmWorksStructure::theInstance()->modificationDone();
}

void DataField::setFixedText(const QString &text)
{
    if (p_fixed_text == text) return;
    p_model_column = -1;
    p_fixed_text = text;
    emit RealmWorksStructure::theInstance()->modificationDone();
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
