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

#include "rw_contents_item.h"   // TODO - remove later

class QModelIndex;
class QXmlStreamWriter;
class RWContentsItem;

class RWStructureItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString structureElement READ structureElement)
    Q_PROPERTY(bool ignoreForContents READ ignoreForContents WRITE setIgnoreForContents)

public:
    RWStructureItem(QXmlStreamReader *stream, QObject *parent = 0, bool ignore_for_contents = false);

public:
    // Attributes from the structure definition
    QString name() const { return p_name; }
    QString id() const { return p_id; }
    bool global() const { return p_global; }
    QString uuid() const { return p_uuid; }
    QString signature() const { return p_signature; }

    void setIgnoreForContents(bool flag) { p_ignore_for_contents = flag; }
    bool ignoreForContents() const { return p_ignore_for_contents; }

    // More information
    virtual bool canBeGenerated() const;
    QString structureText() { return p_structure_text; }
    void setStructureText(const QString &text) { p_structure_text = text; }

    virtual void writeToStructure(QXmlStreamWriter*);
    virtual void postLoad(void) {}

    QString structureElement() const { return p_structure_element; }

    QString namespaceUri() const { return p_namespace_uri; }
    const QXmlStreamAttributes &attributes() const { return p_attributes; }

    template<typename T>
    inline QList<T> childItems() const { return findChildren<T>(QString(), Qt::FindDirectChildrenOnly); }

    RWStructureItem *childElement(const QString &element_name) const;
    RWContentsItem *createContentsTree(RWContentsItem *parent = 0);

protected:
    virtual void writeChildrenToStructure(QXmlStreamWriter *writer);
    virtual RWContentsItem *createContentsItem(RWContentsItem *parent);

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
