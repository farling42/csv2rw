#ifndef RW_FACET_H
#define RW_FACET_H

#include "rw_base_item.h"

class QXmlStreamWriter;

class RWFacet : public RWBaseItem
{
    Q_OBJECT
public:
    // SnippetType values are taken from the XSD v2
    enum SnippetType { Multi_Line, Labeled_Text, Numeric, Date_Game, Date_Range,
                       Tag_Standard, Tag_Multi_Domain, Hybrid_Tag, Foreign,
                       Statblock, Portfolio, Picture, Rich_Text, PDF,
                       Audio, Video, HTML, Smart_Image };
    enum SnippetStyle { Flavor, Callout, Handout, Normal, Read_Aloud };

    Q_ENUM(SnippetType)
    Q_ENUM(SnippetStyle)

    RWFacet(QXmlStreamReader *stream, QObject *parent = 0);
    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);

    SnippetType  snippetType()  const { return p_snippet_type;  }
    SnippetStyle snippetStyle() const { return p_snippet_style; }

private:
    SnippetType p_snippet_type;
    SnippetStyle p_snippet_style;
};

#endif // RWFACET_H
