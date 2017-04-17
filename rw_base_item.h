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
    Q_PROPERTY(int nameColumn READ modelColumnForName WRITE setModelColumnForName)
    Q_PROPERTY(int prefixColumn READ modelColumnForPrefix WRITE setModelColumnForPrefix)
    Q_PROPERTY(int suffixColumn READ modelColumnForSuffix WRITE setModelColumnForSuffix)
    Q_PROPERTY(int textColumn READ modelColumnForText WRITE setModelColumnForText)
    Q_PROPERTY(int tagColumn READ modelColumnForTag WRITE setModelColumnForTag)
    Q_PROPERTY(bool revealed READ isRevealed WRITE setIsRevealed)
    Q_PROPERTY(bool gmOnly READ isGMonly WRITE setIsGMonly)
    Q_PROPERTY(QString elementName READ elementName)
    Q_PROPERTY(bool ignoreForContents READ ignoreForContents WRITE setIgnoreForContents)

public:
    RWBaseItem(QXmlStreamReader *stream, QObject *parent = 0, bool ignore_for_contents = false);

public Q_SLOTS:
    void setModelColumnForName(int column);
    void setModelColumnForPrefix(int column);
    void setModelColumnForSuffix(int column);
    void setModelColumnForText(int column);
    void setModelColumnForTag(int column);
    void setIsRevealed(bool is_revealed) { p_revealed = is_revealed; }
    void setIsGMonly(bool flag) { p_gm_only = flag; }

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

    int  modelColumnForName() const;
    QString modelValueForName(const QModelIndex &index) const;

    int  modelColumnForPrefix() const;
    QString modelValueForPrefix(const QModelIndex &index) const;

    int  modelColumnForSuffix() const;
    QString modelValueForSuffix(const QModelIndex &index) const;

    int  modelColumnForText() const;
    QString modelValueForText(const QModelIndex &index) const;

    int  modelColumnForTag() const;
    QString modelValueForTag(const QModelIndex &index) const;

    bool isRevealed() const { return p_revealed; }
    QString isRevealedString() const { return p_revealed ? "true" : "false"; }
    bool isGMonly() const { return p_gm_only; }

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

protected:
    virtual void writeChildrenToStructure(QXmlStreamWriter *writer);
    virtual void writeChildrenToContents(QXmlStreamWriter *writer, const QModelIndex &index);

private:
    QXmlStreamAttributes p_attributes;
    bool p_revealed;
    bool p_gm_only;
    QString p_namespace_uri;
    QString p_element_name;
    QString p_name;
    QString p_id;
    bool p_global;
    QString p_uuid;       // global_uuid or original_uuid
    QString p_signature;  // only when global == false
    int p_model_column_for_name;
    int p_model_column_for_prefix;
    int p_model_column_for_suffix;
    int p_model_column_for_text;
    int p_model_column_for_tag;
    bool p_ignore_for_contents;
    friend QDebug operator<<(QDebug stream, const RWBaseItem&);
};

QDebug operator<<(QDebug stream, const RWBaseItem&);

#endif // RW_BASE_ITEM_H
