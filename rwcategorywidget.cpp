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

#include "rwcategorywidget.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QLabel>
#include <QFrame>
#include <QBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QTextStream>
#include "fieldcombobox.h"
#include "fieldlineedit.h"
#include "fieldmultilineedit.h"

#include "rw_category.h"
#include "rw_domain.h"
#include "rw_facet.h"
#include "rw_partition.h"

static inline QString column_name(QAbstractItemModel *model, int column)
{
    return model->index(column, 0).data().toString();
}

/**
 * @brief RWCategoryWidget::RWCategoryWidget
 *
 * Creates the widget which shows the entire structure of this particular category.
 *
 * @param category
 * @param parent
 */
RWCategoryWidget::RWCategoryWidget(RWCategory *category, QAbstractItemModel *columns, QWidget *parent) :
    QFrame(parent),
    p_category(category)
{
    RWBaseItem *description;
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    // Start with the title (+ prefix + suffix)
    QRadioButton  *reveal = new QRadioButton(QString());
    FieldLineEdit *name   = new FieldLineEdit(category->namefield());
    FieldLineEdit *prefix = new FieldLineEdit(category->prefix());
    FieldLineEdit *suffix = new FieldLineEdit(category->suffix());

    // Should the category be marked as revealed
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    connect(reveal, &QRadioButton::toggled, category, &RWCategory::setIsRevealed);

    name->setPlaceholderText("<name>");
    description = category->childElement("description");
    name->setToolTip(description ? description->structureText() : category->name());
    if (category->namefield().modelColumn() >= 0)
        name->setText(column_name(columns, category->namefield().modelColumn()));

    prefix->setPlaceholderText("<prefix>");
    prefix->setToolTip("prefix");
    if (category->prefix().modelColumn() >= 0)
        prefix->setText(column_name(columns, category->prefix().modelColumn()));

    suffix->setPlaceholderText("<suffix>");
    suffix->setToolTip("suffix");
    if (category->suffix().modelColumn() >= 0)
        suffix->setText(column_name(columns, category->suffix().modelColumn()));

    QBoxLayout *title = new QHBoxLayout;
    title->addWidget(reveal, 0);
    title->addWidget(name, 2);
    title->addWidget(prefix, 1);
    title->addWidget(suffix, 1);
    layout->addLayout(title);

    // Now deal with the sections
    QList<int> sections;
    sections.append(1);
    QList<RWPartition*> child_items = category->childItems<RWPartition*>();
    foreach (RWPartition *child, child_items)
    {
        layout->addWidget(add_partition(sections, columns, child));
        sections.last()++;
    }

    layout->addStretch(2);
    setLayout(layout);
}

void RWCategoryWidget::do_insert()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button == 0) return;
    QBoxLayout *layout = button->property("where").value<QBoxLayout*>();
    if (layout == 0) return;

    // Add an additional contents field
    FieldLineEdit *edit = new FieldLineEdit(p_category->contentsText());
    edit->setToolTip("contents");
    edit->setPlaceholderText("<generic>");
    //edit->setText(column_name(columns, p_category->modelColumnForText()));

    // Insert immediately before the INSERT button
    layout->insertWidget(layout->indexOf(button), edit);

}


static QString section_string(QList<int> sections)
{
    QString result;
    QTextStream stream(&result);
    for (int level = 0; level < sections.size(); level++)
    {
        stream << sections[level] << '.';
    }
    return result;
}

QWidget *RWCategoryWidget::add_partition(QList<int> sections, QAbstractItemModel *columns, RWPartition *partition)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,/*top*/9,0,0);

    QLabel *title = new QLabel;
    title->setFrameStyle(QFrame::Panel | QFrame::Raised);
    title->setLineWidth(2);
    title->setMargin(3);
    title->setText(QString("%1  %2").arg(section_string(sections)).arg(partition->name()));
    RWBaseItem *description = partition->childElement("description");
    title->setToolTip(description ? description->structureText() : partition->id());
    layout->addWidget(title);

    QFont bold_font = title->font();
    bold_font.setBold(true);
    title->setFont(bold_font);

    QList<RWFacet*> child_facets = partition->childItems<RWFacet*>();
    foreach (RWFacet *facet, child_facets)
    {
        layout->addWidget(add_facet(columns, facet));
    }

    // Finally a generic text box (the contents + reveal)
    QRadioButton *reveal = new QRadioButton(QString());
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    reveal->setChecked(partition->isRevealed());
    connect(reveal, &QRadioButton::toggled, partition, &RWFacet::setIsRevealed);

    FieldMultiLineEdit *edit = new FieldMultiLineEdit(partition->contentsText());
    edit->setToolTip("contents");
    RWBaseItem *purpose = partition->childElement("purpose");
    edit->setPlaceholderText(purpose ? purpose->structureText() : "<purpose>");
    if (partition->contentsText().modelColumn() >= 0)
        edit->setText(column_name(columns, partition->contentsText().modelColumn()));

    QHBoxLayout *textlayout = new QHBoxLayout;
    textlayout->addWidget(reveal);
    textlayout->addWidget(edit);
    layout->addLayout(textlayout);

#ifdef ADD_SNIPPET
    // Add a button to allow a second <contents> section to be added.
    QPushButton *insert_button = new QPushButton("+");
    connect(insert_button, &QPushButton::clicked, this, &RWCategoryWidget::do_insert);
    insert_button->setProperty("where", QVariant::fromValue(layout));
    layout->addWidget(insert_button);
#endif

    // Then display the sub-partitions
    QWidget *first_sub = 0;
    sections.append(1);
    QList<RWPartition*> child_partitions = partition->childItems<RWPartition*>();
    foreach (RWPartition *child, child_partitions)
    {
        QWidget *sub_part = add_partition (sections, columns, child);
        if (first_sub == 0) first_sub = sub_part;
        layout->addWidget(sub_part);
        sections.last()++;
    }
    sections.removeLast();

    QFrame *frame = new QFrame;
    frame->setLayout(layout);
    return frame;
}

QWidget *RWCategoryWidget::add_facet(QAbstractItemModel *columns, RWFacet *facet)
{
    QRadioButton *reveal = 0;
    QLabel *label = 0;
    FieldLineEdit *filename = 0;
    FieldComboBox *combo = 0;
    QWidget *edit_widget = 0;

    // Every snippet is revealable
    reveal = new QRadioButton(QString());
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    connect(reveal, &QRadioButton::toggled, facet, &RWFacet::setIsRevealed);

    if (facet->snippetType() == RWFacet::Labeled_Text || facet->snippetType() == RWFacet::Hybrid_Tag)
    {
        label = new QLabel;
        label->setText(facet->name() + ":");
        if (facet->attributes().value("label_style").toString() == "Bold")
        {
            QFont bold_font = label->font();
            bold_font.setBold(true);
            label->setFont(bold_font);
        }
        label->setToolTip(facet->id());
    }
//Multi_Line, Hybrid_Tag, Labeled_Text, Tag_Standard, Picture

    // Maybe a tag selector
    if (facet->snippetType() == RWFacet::Hybrid_Tag)
    {
        combo = new FieldComboBox(facet->tags(), RWDomain::getDomainById(facet->attributes().value("domain_id").toString()));
        if (facet->tags().modelColumn() >= 0)
            combo->setIndexString(column_name(columns, facet->tags().modelColumn()));
    }

    if (facet->snippetType() == RWFacet::Picture ||
        facet->snippetType() == RWFacet::Smart_Image)
    {
        // A field in which to choose the field for the IMAGE file to be loaded
        filename = new FieldLineEdit(facet->filename());
        filename->setPlaceholderText(facet->name());
        filename->setToolTip(tr("File containing asset"));
        // Change the background colour
        QPalette p = filename->palette();
        p.setBrush(QPalette::Base, p.button());
        filename->setPalette(p);
        filename->setBackgroundRole(QPalette::Button);
        filename->setMaximumWidth(100);
        if (facet->contentsText().modelColumn() >= 0)
        {
            filename->setText(column_name(columns, facet->filename().modelColumn()));
        }
    }

    if (facet->snippetType() == RWFacet::Multi_Line)
    {
        FieldMultiLineEdit *edit = new FieldMultiLineEdit(facet->contentsText());
        edit_widget = edit;

        // Use the <description> child as a tool tip, if available
        RWBaseItem *description = facet->childElement("description");
        edit->setToolTip(description ? description->structureText() : facet->uuid());
        if (facet->snippetType() == RWFacet::Hybrid_Tag)
        {
            edit->setPlaceholderText("Enter annotation here");
        }
        else
        {
            edit->setPlaceholderText(facet->name());
        }
        if (facet->contentsText().modelColumn() >= 0)
        {
            edit->setText(column_name(columns, facet->contentsText().modelColumn()));
        }
    }
    else
    {
        FieldLineEdit *edit = new FieldLineEdit(facet->contentsText());
        edit_widget = edit;

        // Use the <description> child as a tool tip, if available
        RWBaseItem *description = facet->childElement("description");
        edit->setToolTip(description ? description->structureText() : facet->uuid());
        if (facet->snippetType() == RWFacet::Hybrid_Tag ||
                facet->snippetType() == RWFacet::Picture ||
                facet->snippetType() == RWFacet::Smart_Image)
        {
            edit->setPlaceholderText("Enter annotation here");
        }
        else
        {
            edit->setPlaceholderText(facet->name());
        }
        if (facet->contentsText().modelColumn() >= 0)
        {
            edit->setText(column_name(columns, facet->contentsText().modelColumn()));
        }
    }

    // Create a row containing all these widgets
    QHBoxLayout *boxl = new QHBoxLayout;
    boxl->setContentsMargins(0,0,0,0);
    if (reveal) boxl->addWidget(reveal);
    if (label) boxl->addWidget(label);
    if (filename) boxl->addWidget(filename);
    if (combo) boxl->addWidget(combo);
    if (edit_widget) boxl->addWidget(edit_widget);

    QWidget *box = new QWidget;
    box->setProperty("facet",QVariant::fromValue((void*)facet));
    box->setLayout(boxl);

    return box;
}
