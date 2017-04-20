#ifndef RW_BASE_ITEM_H
#define RW_BASE_ITEM_H

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

class QModelIndex;
class QXmlStreamWriter;

class RWBaseItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int textColumn   READ modelColumnForText   WRITE setModelColumnForText)
    Q_PROPERTY(int tagColumn    READ modelColumnForTag    WRITE setModelColumnForTag)
    Q_PROPERTY(QString elementName READ elementName)
    Q_PROPERTY(bool revealed          READ isRevealed        WRITE setIsRevealed)
    Q_PROPERTY(bool ignoreForContents READ ignoreForContents WRITE setIgnoreForContents)

public:
    RWBaseItem(QXmlStreamReader *stream, QObject *parent = 0, bool ignore_for_contents = false);

public Q_SLOTS:
    void setModelColumnForText(int column);
    void setModelColumnForTag(int column);
    void setIsRevealed(bool is_revealed) { p_revealed = is_revealed; }

public:
    QString name() const { return p_name; }
    QString id() const { return p_id; }
    bool global() const { return p_global; }
    QString uuid() const { return p_uuid; }
    QString signature() const { return p_signature; }
    virtual bool canBeGenerated() const;

    virtual void writeToStructure(QXmlStreamWriter*);
    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);
    virtual void postLoad(void) {}

    int  modelColumnForText() const;
    QString modelValueForText(const QModelIndex &index) const;

    int  modelColumnForTag() const;
    QString modelValueForTag(const QModelIndex &index) const;

    bool isRevealed() const { return p_revealed; }
    QString isRevealedString() const { return p_revealed ? "true" : "false"; }

    QString elementName() const { return p_element_name; }

    QString p_text;

    QString namespaceUri() const { return p_namespace_uri; }
    const QXmlStreamAttributes &attributes() const { return p_attributes; }

    static QString xmlParagraph(const QString &text);
    static QString xmlSpan(const QString &text, bool bold = false);

    void setIgnoreForContents(bool flag) { p_ignore_for_contents = flag; }
    bool ignoreForContents() const { return p_ignore_for_contents; }

    template<typename T>
    inline QList<T> childItems() const { return findChildren<T>(QString(), Qt::FindDirectChildrenOnly); }

    RWBaseItem *childElement(const QString &element_name) const;
    void writeExportTag(QXmlStreamWriter *writer);

protected:
    virtual void writeChildrenToStructure(QXmlStreamWriter *writer);
    virtual void writeChildrenToContents(QXmlStreamWriter *writer, const QModelIndex &index);

private:
    QXmlStreamAttributes p_attributes;
    bool p_revealed;
    QString p_namespace_uri;
    QString p_element_name;
    QString p_name;
    QString p_id;
    bool p_global;
    QString p_uuid;       // global_uuid or original_uuid
    QString p_signature;  // only when global == false
    int p_model_column_for_text;
    int p_model_column_for_tag;
    bool p_ignore_for_contents;
    friend QDebug operator<<(QDebug stream, const RWBaseItem&);
};

QDebug operator<<(QDebug stream, const RWBaseItem&);

#endif // RW_BASE_ITEM_H
