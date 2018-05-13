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

static QMetaEnum snip_type_enum  = QMetaEnum::fromType<RWFacet::SnippetType>();
static QMetaEnum snip_style_enum = QMetaEnum::fromType<RWFacet::SnippetStyle>();

const int NAME_TYPE_LENGTH = 50;

RWFacet::RWFacet(QXmlStreamReader *stream, QObject *parent) :
    RWBaseItem(stream, parent),
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
        p_snippet_type = (SnippetType) snip_type_enum.keyToValue(qPrintable(ftype), &ok);
        if (!ok) qWarning() << "Unknown SNIPPET type" << ftype;
    }
    else
        p_snippet_type = Multi_Line;
}


static QString to_gregorian(const QString &from)
{
    // TODO - Realm Works does not like "gregorian" fields in this format!

    /* Must be [Y]YYYY-MM-DD hh:mm:ss[ BCE]
     * year limit is 20000 */
    if (from.length() >= 19) return from;
    // If no time in the field, then simply append midnight time */
    if (from.length() == 10 || from.length() == 11) return from + " 00:00:00";
    qWarning() << "INVALID DATE FORMAT:" << from << "(should be [Y]YYYY-MM-DD HH:MM:SS)";
    return from;
}


void RWFacet::writeToContents(QXmlStreamWriter *writer, const QModelIndex &index)
{
    Q_UNUSED(index);
    bool bold = false;

    // Ignore date snippets if no data available
    const QString start_date  = startDate().valueString(index);
    if ((p_snippet_type == Date_Game || p_snippet_type == Date_Range) && start_date.isEmpty())
    {
        return;
    }

    writer->writeStartElement("snippet");
    {
        const QString user_text = contentsText().valueString(index);
        const QString gm_dir    = gmDirections().valueString(index);
        const QString asset     = filename().valueString(index);
        const QString finish_date = finishDate().valueString(index);

        if (!id().isEmpty()) writer->writeAttribute("facet_id", id());
        writer->writeAttribute("type", snip_type_enum.valueToKey(p_snippet_type));
        if (p_snippet_style != Normal) writer->writeAttribute("style", snip_style_enum.valueToKey(p_snippet_style));
        if (isRevealed()) writer->writeAttribute("is_revealed", "true");
        if (!gm_dir.isEmpty()) writer->writeAttribute("purpose", user_text.isEmpty() ? "Directions_Only" : "Both");

#if 0
        QString label_text = p_label_text.valueString(index);
        if (id().isEmpty() && p_snippet_type == Labeled_Text && !label_text.isEmpty())
        {
            // For locally added snippets of the Labeled_Text variety
            writer->writeAttribute("label", label_text);
        }
#endif

        //
        // CHILDREN have to be in a specific order:
        //  contents | smart_image | ext_object | game_date | date_range
        //  annotation
        //  gm_directions
        //  X x (link | dlink)
        //  X x tag_assign

        // Maybe an ANNOTATION or CONTENTS
        if (!user_text.isEmpty() || !asset.isEmpty() || !start_date.isEmpty())
        {
            switch (p_snippet_type)
            {
            case Multi_Line:
            case Labeled_Text:
            {
                QString text;
                foreach (const QString &para, user_text.split("\n\n"))
                    text.append(xmlParagraph(xmlSpan(para, bold)));
                writer->writeTextElement("contents", text);
                break;
            }
            case Tag_Standard:
            case Hybrid_Tag:
                writer->writeTextElement("annotation", xmlParagraph(xmlSpan(user_text, bold)));
                break;

                // There are a lot of snippet types which have an ext_object child (which has an asset child)
            case Foreign:
                write_ext_object(writer, "Foreign", asset);
                break;
            case Statblock: // this might require an .rtf file?
                write_ext_object(writer, "Statblock", asset);
                break;
            case Portfolio: // requires a HeroLab portfolio
                write_ext_object(writer, "Portfolio", asset);
                break;
            case Picture:
                write_ext_object(writer, "Picture", asset);
                break;
            case Rich_Text:
                write_ext_object(writer, "Rich_Text", asset);
                break;
            case PDF:
                write_ext_object(writer, "PDF", asset);
                break;
            case Audio:
                write_ext_object(writer, "Audio", asset);
                break;
            case Video:
                write_ext_object(writer, "Video", asset);
                break;
            case HTML:
                write_ext_object(writer, "HTML", asset);
                break;

            case Smart_Image:
                // Slightly different format since it has a smart_image child (which has an asset child)
                write_smart_image(writer, asset);
                break;

            case Date_Game:
                writer->writeStartElement("game_date");
                //writer->writeAttribute("canonical", start_date);
                writer->writeAttribute("gregorian", to_gregorian(start_date));
                //writer->writeAttribute("display", start_date); // XML v3?
                writer->writeEndElement();
                if (!user_text.isEmpty())
                {
                    writer->writeTextElement("annotation", xmlParagraph(xmlSpan(user_text, bold)));
                }
                break;

            case Date_Range:
                writer->writeStartElement("date_range");
                //writer->writeAttribute("canonical_start", start_date);
                writer->writeAttribute("gregorian_start", to_gregorian(start_date));
                //writer->writeAttribute("display_start", start_date);
                //writer->writeAttribute("canonical_end",   finish_date);
                writer->writeAttribute("gregorian_end",   to_gregorian(finish_date));
                //writer->writeAttribute("display_end",   finish_date);
                writer->writeEndElement();
                if (!user_text.isEmpty())
                {
                    writer->writeTextElement("annotation", xmlParagraph(xmlSpan(user_text, bold)));
                }
                break;

            case Numeric:
            case Tag_Multi_Domain:
                qFatal("RWFacet::writeToContents: invalid snippet type: %d", p_snippet_type);
                break;
            }
        }

        // Maybe some GM directions
        if (!gm_dir.isEmpty())
        {
            writer->writeTextElement("gm_directions", xmlParagraph(xmlSpan(user_text, /*bold*/ false)));
        }

        // Maybe one or more TAG_ASSIGN (to be entered AFTER the contents/annotation)
        QString tag_names = p_tags.valueString(index);
        if (!tag_names.isEmpty())
        {
            // Find the domain to use
            QString domain_id = attributes().value("domain_id").toString();
            RWDomain *domain = RWDomain::getDomainById(domain_id);
            if (domain)
            {
                foreach (QString tag_name, tag_names.split(","))
                {
                    QString tag_id = domain->tagId(tag_name.trimmed());
                    if (!tag_id.isEmpty())
                    {
                        writer->writeStartElement("tag_assign");
                        writer->writeAttribute("tag_id", tag_id);
                        writer->writeAttribute("type","Indirect");
                        writer->writeAttribute("is_auto_assign", "true");
                        writer->writeEndElement();
                    }
                    else
                        qWarning() << "No TAG defined for" << tag_name.trimmed() << "in DOMAIN" << domain->name();
                }
            }
            else if (!domain_id.isEmpty())
                qWarning() << "DOMAIN not found for" << domain_id << "on FACET" << id();
            else
                qWarning() << "domain_id does not exist on FACET" << id();
        }

    }
    writer->writeEndElement();  // snippet
}

void RWFacet::write_asset(QXmlStreamWriter *writer, const QString &asset)
{
    QFile file(asset);
    QUrl url(asset);
    const int FILENAME_TYPE_LENGTH = 200;
    if (file.open(QFile::ReadOnly))
    {
        QFileInfo info(file);

        writer->writeStartElement("asset");
        writer->writeAttribute("filename", info.fileName().right(FILENAME_TYPE_LENGTH));
        //writer->writeAttribute("thumbnail_size", info.fileName());

        QByteArray contents = file.readAll();
        writer->writeTextElement("contents", contents.toBase64());

        //writer->writeTextElement("thumbnail", thumbnail.toBase64());
        //writer->writeTextElement("summary", thing.toBase64());
        //writer->writeTextElement("url", filename);

        writer->writeEndElement();   // asset
    }
    else if (url.isValid())
    {
        static QNetworkAccessManager *nam = 0;
        if (nam == 0)
        {
            nam = new QNetworkAccessManager;
            nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        }
        QNetworkRequest request(url);
        QNetworkReply *reply = nam->get(request);
        while (!reply->isFinished())
        {
            qApp->processEvents(QEventLoop::WaitForMoreEvents);
        }
        if (reply->error() != QNetworkReply::NoError)
        {
            qWarning() << "Failed to locate URL:" << asset;
        }
        // A redirect has ContentType of "text/html; charset=UTF-8, image/png"
        // which is an ordered comma-separated list of types.
        // So we need to check the LAST type which will be for readAll()
        else if (reply->header(QNetworkRequest::ContentTypeHeader).toString().split(',').last().trimmed().startsWith("image/"))
        {
            QString tempname = QFileInfo(url.path()).baseName() + '.' + reply->header(QNetworkRequest::ContentTypeHeader).toString().split("/").last();
            writer->writeStartElement("asset");
            writer->writeAttribute("filename", tempname.right(FILENAME_TYPE_LENGTH));

            QByteArray contents = reply->readAll();
            writer->writeTextElement("contents", contents.toBase64());
            //writer->writeTextElement("url", filename);

            writer->writeEndElement();   // asset
        }
        else
        {
            // A redirect has QPair("Content-Type","text/html; charset=UTF-8, image/png")
            // note the comma-separated list of content types (legal?)
            // the body of the message is actually PNG binary data.
            // QPair("Server","Microsoft-IIS/8.5, Microsoft-IIS/8.5") so maybe ISS sent wrong content type

            qWarning() << "Only URLs to images are supported (not" << reply->header(QNetworkRequest::ContentTypeHeader) << ")! Check source at" << asset;
            //if (reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith("text/"))
            //    qWarning() << "Body =" << reply->readAll();
            //qWarning() << "Raw Header List =" << reply->rawHeaderPairs();
        }
        reply->deleteLater();
    }
    else
    {
        QString message = "File/URL does not exist: " + asset;

        static QMessageBox *warning = 0;
        if (warning == 0)
        {
            warning = new QMessageBox;
            warning->setText("Issues encountered during GENERATE:\n");
        }
        warning->setText(warning->text() + '\n' + message);
        warning->show();
    }
}

/**
 * @brief RWFacet::write_ext_object
 * @param writer
 * @param exttype one of Foreign, Statblock, Portfolio, Picture, Rich_Text, PDF, Audio, Video, HTML
 * @param filename
 */
void RWFacet::write_ext_object(QXmlStreamWriter *writer, const QString &exttype, const QString &asset)
{
    writer->writeStartElement("ext_object");
    writer->writeAttribute("name", QFileInfo(asset).fileName().right(NAME_TYPE_LENGTH));
    writer->writeAttribute("type", exttype);
    write_asset(writer, asset);
    writer->writeEndElement();
}

void RWFacet::write_smart_image(QXmlStreamWriter *writer, const QString &asset)
{
    writer->writeStartElement("smart_image");
    writer->writeAttribute("name", QFileInfo(asset).fileName().right(NAME_TYPE_LENGTH));
    write_asset(writer, asset);
    // write_overlay (0-1)
    // write_subset_mask (0-1)
    // write_superset_mask (0-1)
    // write_map_pan (0+)
    writer->writeEndElement();
}
