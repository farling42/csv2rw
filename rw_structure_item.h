#ifndef RW_STRUCTURE_ITEM_H
#define RW_STRUCTURE_ITEM_H

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
#include <QXmlStreamReader>
#include "datafield.h"

class QModelIndex;
class QXmlStreamWriter;

class RWStructureItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString structureElement READ structureElement)
    Q_PROPERTY(bool ignoreForContents READ ignoreForContents WRITE setIgnoreForContents)

    Q_PROPERTY(bool revealed          READ isRevealed        WRITE setIsRevealed)
    Q_PROPERTY(SnippetStyle snippetStyle READ snippetStyle WRITE setSnippetStyle)
    // SnippetStyle should be in RWFacet, but it is used by RWPartition

public:
    RWStructureItem(QXmlStreamReader *stream, QObject *parent = 0, bool ignore_for_contents = false);

    enum SnippetStyle { Normal, Read_Aloud, Handout, Flavor, Callout };
    Q_ENUM(SnippetStyle)
    enum SnippetVeracity { Truth, Partial, Lie };
    Q_ENUM(SnippetVeracity)
    enum SnippetPurpose { Story_Only, Directions_Only, Both };
    Q_ENUM(SnippetPurpose)

public Q_SLOTS:
    void setIsRevealed(bool is_revealed) { p_revealed = is_revealed; }
    void setSnippetStyleInt(int style) { p_snippet_style = (SnippetStyle)style; }
    void setSnippetStyle(SnippetStyle style) { p_snippet_style = style; }
    void setSnippetVeracity(SnippetVeracity veracity) { p_snippet_veracity = veracity; }
    void setSnippetPurpose(SnippetPurpose purpose) { p_snippet_purpose = purpose; }

public:
    // Attributes from the structure definition
    QString name() const { return p_name; }
    QString id() const { return p_id; }
    bool global() const { return p_global; }
    QString uuid() const { return p_uuid; }
    QString signature() const { return p_signature; }

    void setIgnoreForContents(bool flag) { p_ignore_for_contents = flag; }
    bool ignoreForContents() const { return p_ignore_for_contents; }

    SnippetStyle snippetStyle() const { return p_snippet_style; }
    SnippetVeracity snippetVeracity () const { return p_snippet_veracity; }
    SnippetPurpose snippetPurpose() const { return p_snippet_purpose; }

    // More information
    virtual bool canBeGenerated() const;
    DataField &contentsText() { return p_contents_text; }
    QString structureText() { return p_structure_text; }
    void setStructureText(const QString &text) { p_structure_text = text; }

    virtual void writeToStructure(QXmlStreamWriter*);
    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);
    virtual void postLoad(void) {}

    bool isRevealed() const { return p_revealed; }
    QString isRevealedString() const { return p_revealed ? "true" : "false"; }

    QString structureElement() const { return p_structure_element; }

    QString namespaceUri() const { return p_namespace_uri; }
    const QXmlStreamAttributes &attributes() const { return p_attributes; }

    enum TextClass { RWDefault, RWSnippet, RWEnumerated };
    static QString xmlParagraph(const QString &contentsText, TextClass text_class = RWDefault, int margin = 0);
    static QString xmlSpan(const QString &contentsText,
                           bool bold = false, bool italic = false,
                           bool line_through = false, bool underline = false);

    template<typename T>
    inline QList<T> childItems() const { return findChildren<T>(QString(), Qt::FindDirectChildrenOnly); }

    RWStructureItem *childElement(const QString &element_name) const;
    void writeExportTag(QXmlStreamWriter *writer);

protected:
    virtual void writeChildrenToStructure(QXmlStreamWriter *writer);
    virtual void writeChildrenToContents(QXmlStreamWriter *writer, const QModelIndex &index);
    SnippetStyle p_snippet_style;
    SnippetVeracity p_snippet_veracity;
    SnippetPurpose p_snippet_purpose;

private:
    QXmlStreamAttributes p_attributes;
    DataField p_contents_text;
    bool p_revealed;
    QString p_namespace_uri;
    QString p_structure_element;
    QString p_structure_text;
    QString p_name;
    QString p_id;
    bool p_global;
    QString p_uuid;       // global_uuid or original_uuid
    QString p_signature;  // only when global == false
    bool p_ignore_for_contents;
    friend QDebug operator<<(QDebug stream, const RWStructureItem&);
};

QDebug operator<<(QDebug stream, const RWStructureItem&);

#endif // RW_BASE_ITEM_H
