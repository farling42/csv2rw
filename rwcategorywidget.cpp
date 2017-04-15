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
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    // Start with the title (+ prefix + suffix)
    FieldLineEdit *name = new FieldLineEdit;
    FieldLineEdit *prefix = new FieldLineEdit;
    FieldLineEdit *suffix = new FieldLineEdit;

    connect(name,   &FieldLineEdit::modelColumnSelected, category, &RWBaseItem::setModelColumnForName);
    connect(prefix, &FieldLineEdit::modelColumnSelected, category, &RWBaseItem::setModelColumnForPrefix);
    connect(suffix, &FieldLineEdit::modelColumnSelected, category, &RWBaseItem::setModelColumnForSuffix);

    name->setPlaceholderText("<name>");
    name->setToolTip(category->name());
    if (category->modelColumnForName() >= 0)
        name->setText(column_name(columns, category->modelColumnForName()));

    prefix->setPlaceholderText("<prefix>");
    prefix->setToolTip("prefix");
    if (category->modelColumnForPrefix() >= 0)
        prefix->setText(column_name(columns, category->modelColumnForPrefix()));

    suffix->setPlaceholderText("<suffix>");
    suffix->setToolTip("suffix");
    if (category->modelColumnForSuffix() >= 0)
        suffix->setText(column_name(columns, category->modelColumnForSuffix()));

    QBoxLayout *title = new QHBoxLayout;
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

    layout->addStretch();
    setLayout(layout);
}

void RWCategoryWidget::do_insert()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button == 0) return;
    QBoxLayout *layout = button->property("where").value<QBoxLayout*>();
    if (layout == 0) return;

    // Add an additional contents field
    FieldLineEdit *edit = new FieldLineEdit;
    connect (edit, &FieldLineEdit::modelColumnSelected, p_category, &RWBaseItem::setModelColumnForText);
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

    QFont bold_font;

    QLabel *title = new QLabel;
    title->setFrameStyle(QFrame::Panel | QFrame::Raised);
    title->setLineWidth(2);
    title->setMargin(3);
    title->setText(QString("%1  %2").arg(section_string(sections)).arg(partition->name()));
    title->setToolTip(partition->id());
    layout->addWidget(title);

    bold_font = title->font();
    bold_font.setBold(true);
    title->setFont(bold_font);

    QList<RWFacet*> child_facets = partition->childItems<RWFacet*>();
    foreach (RWFacet *child, child_facets)
    {
        QRadioButton *reveal = 0;
        QRadioButton *gm_only = 0;
        QLabel *label = 0;
        FieldComboBox *combo = 0;
        FieldLineEdit *edit = 0;

        //add_facet (facet, this);
        QWidget *box = new QWidget;
        box->setProperty("facet",QVariant::fromValue((void*)child));

        // Every snippet is revealable (or is GM only)
        reveal = new QRadioButton(QString());
        reveal->setAutoExclusive(false);
        reveal->setToolTip("revealed?");
        connect(reveal, &QRadioButton::toggled, child, &RWFacet::setIsRevealed);

        // Every snippet could be GM only
        gm_only = new QRadioButton(QString());
        gm_only->setAutoExclusive(false);
        gm_only->setToolTip("GM only?");
        connect(gm_only, &QRadioButton::toggled, child, &RWFacet::setIsGMonly);

        if (child->snippetType() == RWFacet::Labeled_Text || child->snippetType() == RWFacet::Hybrid_Tag)
        {
            label = new QLabel;
            label->setText(child->name() + ":");
            label->setFont(bold_font);
            label->setToolTip(child->id());
        }
 //Multi_Line, Hybrid_Tag, Labeled_Text, Tag_Standard, Picture

        // Maybe a tag selector
        if (child->snippetType() == RWFacet::Hybrid_Tag)
        {
            combo = new FieldComboBox;
            connect (combo, &FieldComboBox::modelColumnSelected, child, &RWBaseItem::setModelColumnForTag);
            QString domain_id = child->attributes().value("domain_id").toString();
            combo->setToolTip("Domain");
            if (!domain_id.isEmpty())
            {
                RWDomain *domain = RWDomain::getDomainById(domain_id);
                if (domain) combo->setToolTip(domain->name());
            }
            if (child->modelColumnForTag() >= 0)
                combo->setValue(column_name(columns, child->modelColumnForTag()));
        }

        edit = new FieldLineEdit;
        connect (edit, &FieldLineEdit::modelColumnSelected, child, &RWBaseItem::setModelColumnForText);
        edit->setToolTip(child->uuid());
        if (child->snippetType() == RWFacet::Hybrid_Tag)
        {
            edit->setPlaceholderText("Enter annotation here");
        }
        else
        {
            edit->setPlaceholderText(child->name());
        }
        if (child->modelColumnForText() >= 0)
            edit->setText(column_name(columns, child->modelColumnForText()));

        // Create a row containing all these widgets
        QHBoxLayout *boxl = new QHBoxLayout;
        boxl->setContentsMargins(0,0,0,0);
        if (reveal) boxl->addWidget(reveal);
        if (gm_only) boxl->addWidget(gm_only);
        if (label) boxl->addWidget(label);
        if (combo) boxl->addWidget(combo);
        if (edit) boxl->addWidget(edit);
        box->setLayout(boxl);

        layout->addWidget(box);
    }

    // Finally a generic text box (the contents + reveal)
    QRadioButton *reveal = new QRadioButton(QString());
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    reveal->setChecked(partition->isRevealed());
    connect(reveal, &QRadioButton::toggled, partition, &RWFacet::setIsRevealed);

    QRadioButton *gm_only = new QRadioButton(QString());
    gm_only->setAutoExclusive(false);
    gm_only->setToolTip("GM only?");
    gm_only->setChecked(partition->isGMonly());
    connect(gm_only, &QRadioButton::toggled, partition, &RWFacet::setIsGMonly);

    FieldLineEdit *edit = new FieldLineEdit;
    connect (edit, &FieldLineEdit::modelColumnSelected, partition, &RWBaseItem::setModelColumnForText);
    edit->setToolTip("contents");
    RWBaseItem *purpose = partition->childElement("purpose");
    edit->setPlaceholderText(purpose ? purpose->p_text : "<purpose>");
    if (partition->modelColumnForText() >= 0)
        edit->setText(column_name(columns, partition->modelColumnForText()));

    QHBoxLayout *textlayout = new QHBoxLayout;
    textlayout->addWidget(reveal);
    textlayout->addWidget(gm_only);
    textlayout->addWidget(edit);
    layout->addLayout(textlayout);

#if 0
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
