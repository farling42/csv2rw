#ifndef RW_FACET_H
#define RW_FACET_H

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

#include "rw_base_item.h"

class QXmlStreamWriter;

class RWFacet : public RWBaseItem
{
    Q_OBJECT
    Q_PROPERTY(SnippetType  snippetType  READ snippetType)
    Q_PROPERTY(SnippetStyle snippetStyle READ snippetStyle WRITE setSnippetStyle)

public:
    // SnippetType values are taken from the XSD v2
    enum SnippetType { Multi_Line, Labeled_Text, Numeric, Date_Game, Date_Range,
                       Tag_Standard, Tag_Multi_Domain, Hybrid_Tag, Foreign,
                       Statblock, Portfolio, Picture, Rich_Text, PDF,
                       Audio, Video, HTML, Smart_Image };
    enum SnippetStyle { Normal, Read_Aloud, Handout, Flavor, Callout };

    Q_ENUM(SnippetType)
    Q_ENUM(SnippetStyle)

    RWFacet(QXmlStreamReader *stream, QObject *parent = 0);
    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);

    SnippetType  snippetType()  const { return p_snippet_type;  }
    SnippetStyle snippetStyle() const { return p_snippet_style; }
    DataField &tags()      { return p_tags; }
    DataField &gmDirections() { return p_gm_directions; }
    DataField &labelText() { return p_label_text; }
    DataField &filename() { return p_filename; }

public slots:
    void setSnippetStyle(SnippetStyle style) { p_snippet_style = style; }

private:
    void write_asset(QXmlStreamWriter *writer, const QString &filename);
    void write_ext_object(QXmlStreamWriter *writer, const QString &exttype, const QString &filename);
    void write_smart_image(QXmlStreamWriter *writer, const QString &filename);
    DataField p_tags;
    DataField p_gm_directions;
    SnippetType p_snippet_type;
    SnippetStyle p_snippet_style;
    DataField p_label_text;   // for Labeled_Text fields
    DataField p_filename;
};

#endif // RWFACET_H
