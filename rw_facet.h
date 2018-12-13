#ifndef RW_FACET_H
#define RW_FACET_H

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

#include "rw_structure_item.h"

class QXmlStreamWriter;
class RWSnippet;    // TODO - remove

class RWFacet : public RWStructureItem
{
    Q_OBJECT
    Q_PROPERTY(SnippetType  snippetType  READ snippetType)

public:
    // SnippetType values are taken from the XSD v2
    enum SnippetType { Multi_Line, Labeled_Text, Numeric, Date_Game, Date_Range,
                       Tag_Standard, Tag_Multi_Domain, Hybrid_Tag, Foreign,
                       Statblock, Portfolio, Picture, Rich_Text, PDF,
                       Audio, Video, HTML, Smart_Image };

    Q_ENUM(SnippetType)

    RWFacet(QXmlStreamReader *stream, QObject *parent = nullptr);

    SnippetType  snippetType()  const { return p_snippet_type;  }

protected:
    virtual RWContentsItem *createContentsItem(RWContentsItem *parent);

public slots:

private:
    SnippetType p_snippet_type;
};

#endif // RW_FACET_H
