/*
RWImporter
Copyright (C) 2018 Martin Smith

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

#include "excel_xlsxmodel.h"
#include "xlsxdocument.h"
#include "xlsxworksheet.h"
#include "xlsxrichstring.h"
#include "xlsxdrawing_p.h"
#include "xlsxdrawinganchor_p.h"
#include "xlsxmediafile_p.h"

#include <QDebug>
#include <QImage>

#define ALLOW_FORMATTING

struct PrivateData
{
    PrivateData(const QString &filename) : doc(filename) {}
    QXlsx::Document doc;
    QVector<QVector<QVariant>> data;
};


ExcelXlsxModel::ExcelXlsxModel(const QString &filename, QObject *parent)
    : QAbstractTableModel(parent),
    p(new PrivateData(filename))
{
    QStringList sheets = p->doc.sheetNames();
#if 0
    qDebug() << "sheet: firstRow" << p->doc.dimension().firstRow() <<
                ", lastRow" << p->doc.dimension().lastRow() <<
                ", firstColumn" << p->doc.dimension().firstColumn() <<
                ", lastColumn" << p->doc.dimension().lastColumn();
#endif
    //if (p->doc.sheet(name)->sheetType() != QXlsx::AbstractSheet::ST_WorkSheet) continue;
    loadData();
}

QVariant ExcelXlsxModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        int r = /*header row*/ p->doc.dimension().firstRow();
        int c = p->doc.dimension().firstColumn() + section;
        QXlsx::Cell *cell = p->doc.cellAt(r, c);
        //qDebug() << "headerData: cell " << r << "," << c << "=" << (cell ? cell->value() : "<null>");
        if (cell) return cell->value();
        else qDebug() << "data: cell " << r << "," << c << "= no data";
    }
    // FIXME: Implement me!
    return QVariant();
}

int ExcelXlsxModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // First row of excel spreadsheet = headers, so -1 to get data rows
    return p->doc.dimension().lastRow() - p->doc.dimension().firstRow();
}

int ExcelXlsxModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return p->doc.dimension().lastColumn() - p->doc.dimension().firstColumn() + 1;
}


QVariant ExcelXlsxModel::valueOfCell(int row, int column) const
{
    // excel row 0 = headers
    // excel row 1 = first row of data (model row 0)
    int r = p->doc.dimension().firstRow() + row + 1;
    int c = p->doc.dimension().firstColumn() + column;
    QXlsx::Cell *cell = p->doc.cellAt(r, c);
    //if (cell) qDebug() << "data: cell " << r << "," << c << "=" << cell->value().type();
    if (!cell) return QVariant();

#ifndef ALLOW_FORMATTING
    return cell->value();
#else
    if (!cell->isRichString()) return cell->value();

    // If the entire cell is HTML, then don't process it
    QString text = cell->value().toString();
    if (text.startsWith('<') && text.endsWith('>')) return text;

    // Convert rich string to HTML
    const QXlsx::RichString rich = cell->richString();
    //qDebug() << "data: cell " << r << "," << c << "has a rich string";

    // as rich.toPlainString()
    if (rich.isEmtpy()) return QString();

    // Start with the CELL's format, in case we ever get an invalid format from the RichString fragment.
    QXlsx::Format format = cell->format();
    QString result;
    for (int i=0; i<rich.fragmentCount(); i++)
    {
        QXlsx::Format newformat = rich.fragmentFormat(i);
        if (!newformat.isEmpty()) format = newformat;

#if 0
        qDebug() << index << ", fragment" << i << "=" << rich.fragmentText(i);
        QString prefix = QString("r%1, c%2, f%3").arg(r).arg(c).arg(i+1);
        qDebug() << prefix << "font  :" << format.fontName(); // QFont -- TBD
        qDebug() << prefix << "size  :" << format.fontSize(); // int -- TBD
        qDebug() << prefix << "bold  :" << format.fontBold(); // bool
        qDebug() << prefix << "italic:" << format.fontItalic(); // bool
        qDebug() << prefix << "under :" << format.fontUnderline();  // != FontUnderlineNone
        qDebug() << prefix << "strike:" << format.fontStrikeOut(); // bool

        qDebug() << prefix << "colour:" << format.fontColor(); // QColor -- TBD
        qDebug() << prefix << "fontNm:" << format.fontName(); // QString -- TBD
        qDebug() << prefix << "outlin:" << format.fontOutline(); // bool -- TBD
        qDebug() << prefix << "script:" << format.fontScript(); // FontScript -- TBD
#endif
        // Encode into the final Realm Works "RWSnippet" class of span.
        // RWContentsItem::xmlSpan will detect this formatting and not apply any itself.

        QStringList decorations;
        if (format.fontUnderline()) decorations.append("underline");
        if (format.fontStrikeOut()) decorations.append("line-through");
        // Styles - semi-colon separated list
        QStringList styles;
        if (!decorations.isEmpty()) styles.append("text-decoration:" + decorations.join(' '));
        if (format.fontBold()) styles.append("font-weight:bold");
        if (format.fontItalic()) styles.append("font-style:italic");

        QString style;
        if (!styles.isEmpty())
        {
            style = " style=\"" + styles.join(';') + "\"";
        }

        // Escape any "<" that might be in the cell, to avoid interpreting it as markup.
        // How do we allow HTML to be imported from the CELL?
        QString escaped = rich.fragmentText(i).toHtmlEscaped();

        // Handle multiple paragraphs in the cell, the tool always double line-break so that users
        // don't have to manually remove line breaks
        bool first=true;
        for (auto para : escaped.split("\n\n"))
        {
            // Keep double line-break for the xmlPara/xmlSpan processing
            if (first)
                first = false;
            else
                result.append("\n\n");

            result.append(QString("<span class=\"RWSnippet\"%1>%2</span>").arg(style).arg(para));
        }
    }
    //qDebug() << "    " << result;
    return result;
#endif
}

QVariant ExcelXlsxModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole)
    {
        //return valueOfCell(index.row(), index.column());
        int r = index.row();
        int c = index.column();
        if (r < 0 || c < 0 ||
            r >= p->data.length() ||
            c >= p->data[r].length())
            return QVariant();

        return p->data[index.row()][index.column()];
    }
    else if (role == Qt::UserRole && index.isValid())
    {
        return QString("topic_%1").arg(index.row()+1);
    }
    return QVariant();
}

QStringList ExcelXlsxModel::sheetNames() const
{
    return p->doc.sheetNames();
}

QString ExcelXlsxModel::currentSheetName() const
{
    return p->doc.currentSheet()->sheetName();
}

void ExcelXlsxModel::loadData()
{
    // Process all cells immediately, rather than doing it every time that ::data is called.
    int rc = rowCount();
    int cc = columnCount();
    p->data.resize(rc);
    for (int row=0; row<rc; row++)
    {
        p->data[row].resize(cc);
        for (int col=0; col<cc; col++)
        {
            p->data[row][col] = valueOfCell(row,col);
        }
    }

    // Maybe it contains an image.
    QXlsx::Drawing *drawing = p->doc.currentSheet()->drawing();
    if (drawing)
    {
        qDebug() << "File has " << drawing->anchors.size() << " images";
        // TODO
        for (int j=0; j<drawing->anchors.count(); j++)
        {
            QXlsx::DrawingAnchor *base_anchor=drawing->anchors.at(j);
            if (base_anchor->objectType() != QXlsx::DrawingAnchor::Picture)
            {
                qDebug() << "Ignoring image of type" << base_anchor->objectType();
                continue;
            }
            QXlsx::XlsxMarker pos;
            if (QXlsx::DrawingOneCellAnchor *oneanchor = dynamic_cast<QXlsx::DrawingOneCellAnchor*>(base_anchor))
                pos = oneanchor->from;
            else if (QXlsx::DrawingTwoCellAnchor *twoanchor = dynamic_cast<QXlsx::DrawingTwoCellAnchor*>(base_anchor))
                pos = twoanchor->from;
            else {
                qDebug() << "Anchor is NOT OneCellAnchor or TwoCellAnchor";
                continue;
            }

            qDebug() << "Image at EXCEL row " << pos.row() << ", col " << pos.col();

            // Convert from EXCEL row,column to MODEL row column
            // For some reason DrawingAnchor has 0 based positions, while EXCEL cells have 1 based positions.
            int r = pos.row() +1 - p->doc.dimension().firstRow() - 1;
            int c = pos.col() +1 - p->doc.dimension().firstColumn();

            qDebug() << "EXCEL first row" << p->doc.dimension().firstRow() << ", col" << p->doc.dimension().firstColumn();
            qDebug() << "Image at MODEL row " << r << ", col " << c;

            QImage image = QImage::fromData(base_anchor->pictureFile().data()->contents());
            if (image.isNull())
            {
                qDebug() << "Failed to load image for cell";
            }
            else if (r < 0 || c < 0 || r >= p->data.length() || c >= p->data[r].length())
            {
                qDebug() << "IMAGE: location out of range: row" << r << ", col" << c;
            }
            else
            {
                qDebug() << "Loaded image with size" << image.size();
                p->data[r][c] = image;
            }
        }
    }

    qDebug() << "loadData: finished reading all cells from spreadsheet: rc " << rc << ", cc " << cc;
}

void ExcelXlsxModel::selectSheet(const QString &sheetname)
{
    beginResetModel();
    p->doc.selectSheet(sheetname);
    loadData();
    endResetModel();
}
