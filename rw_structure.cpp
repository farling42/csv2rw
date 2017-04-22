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

#include "rw_structure.h"

/**
 * @brief RWStructure::RWStructure
 * @param stream
 * @param parent
 *
 * Whilst this class might look empty, it is a separate sub-class so that
 * findChild and findChildren can quickly locate it amongst all the other
 * RWBaseItems in a child hierarchy.
 */

RWStructure::RWStructure(QXmlStreamReader *stream, QObject *parent) :
    RWBaseItem(stream, parent)
{
}
