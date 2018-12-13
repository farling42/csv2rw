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

#include "rw_facet.h"
#include <QXmlStreamWriter>
#include <QDebug>
#include <QModelIndex>
#include <QMetaEnum>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCoreApplication>
#include "rw_domain.h"
#include "rw_snippet.h"

static QMetaEnum snip_type_enum  = QMetaEnum::fromType<RWFacet::SnippetType>();

RWFacet::RWFacet(QXmlStreamReader *stream, QObject *parent) :
    RWStructureItem(stream, parent),
    p_snippet_type(Multi_Line)
{
    static bool first = true;
    if (first)
    {
        qRegisterMetaType<SnippetType>();
        first = false;
    }
    const QXmlStreamAttributes attr = attributes();

    // Decode the 'type' attribute (if present)
    if (attr.hasAttribute("type"))
    {
        QString ftype = attributes().value("type").toString();
        bool ok = true;
        p_snippet_type = static_cast<SnippetType>(snip_type_enum.keyToValue(qPrintable(ftype), &ok));
        if (!ok) qWarning() << "Unknown SNIPPET type" << ftype;
    }
    else
        p_snippet_type = Multi_Line;
}

RWContentsItem *RWFacet::createContentsItem(RWContentsItem *parent)
{
    return new RWSnippet(this, parent);
}
