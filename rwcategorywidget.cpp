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
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QFrame>
#include <QBoxLayout>
#include <QMetaEnum>
#include <QRadioButton>
#include <QPushButton>
#include <QTextStream>

#include "fieldcombobox.h"
#include "fieldlineedit.h"
#include "fieldmultilineedit.h"

#include "rw_alias.h"
#include "rw_category.h"
#include "rw_domain.h"
#include "rw_facet.h"
#include "rw_partition.h"

static QMetaEnum case_matching_enum   = QMetaEnum::fromType<RWAlias::CaseMatching>();
static QMetaEnum match_priority_enum  = QMetaEnum::fromType<RWAlias::MatchPriority>();


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
RWCategoryWidget::RWCategoryWidget(RWCategory *category, QAbstractItemModel *columns, bool include_sections, QWidget *parent) :
    QFrame(parent),
    p_columns(columns),
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
    QPushButton   *addName = new QPushButton("+Name");

    addName->setToolTip("Adds a True Name/Other Name\n"
                        "Any name whose CSV cell is empty will not be created in RealmWorks.\n"
                        "This allows different sets of attributes to set for different alternative names.");

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

    connect(addName, &QPushButton::clicked, this, &RWCategoryWidget::add_name);

    QBoxLayout *title = new QHBoxLayout;
    title->addWidget(reveal,  0);
    title->addWidget(name,    2);
    title->addWidget(prefix,  1);
    title->addWidget(suffix,  1);
    title->addWidget(addName, 0);
    layout->addLayout(title);

    // For a parent category, we don't want to allow sections to be defined.
    if (include_sections)
    {
        // Now deal with the sections
        QList<int> sections;
        sections.append(1);
        QList<RWPartition*> child_items = category->childItems<RWPartition*>();
        p_first_section = 0;
        foreach (RWPartition *child, child_items)
        {
            QWidget *sec = add_partition(sections, columns, child);
            if (p_first_section == 0) p_first_section = sec;
            layout->addWidget(sec);
            sections.last()++;
        }
        layout->addStretch(2);
    }
    setLayout(layout);

    // Maybe they've switched to a category with some name entries already present.
    foreach (RWAlias *alias, p_category->aliases)
        add_rwalias(alias);
}


void RWCategoryWidget::add_rwalias(RWAlias *alias)
{
    bool is_true_name = (p_category->aliases.first() == alias);

    alias->setIsTrueName(is_true_name);

    QLabel        *label          = new QLabel(is_true_name ? "True Name :" : "Other Name:");
    QRadioButton  *revealed       = new QRadioButton;
    FieldLineEdit *name           = new FieldLineEdit(alias->namefield());
    QCheckBox     *auto_accept    = new QCheckBox;
    QCheckBox     *show_in_nav    = new QCheckBox;
    QComboBox     *case_matching  = new QComboBox;
    QComboBox     *match_priority = new QComboBox;
    QPushButton   *delete_name    = new QPushButton("X");

#if 0
    // XML uses "Correction" whereas RW uses "Auto Correct"
    for (int key=0; key<case_matching_enum.keyCount(); key++)
        case_matching->addItem(case_matching_enum.key(key));
#else
    case_matching->addItem("Ignore");
    case_matching->addItem("Sensitive");
    case_matching->addItem("Auto Correct"); // XML uses "Correction"
#endif

    for (int key=0; key<match_priority_enum.keyCount(); key++)
        match_priority->addItem(match_priority_enum.key(key));

    connect(revealed,       &QRadioButton::toggled, alias, &RWAlias::setRevealed);
    connect(auto_accept,    &QCheckBox::toggled,    alias, &RWAlias::setAutoAccept);
    connect(show_in_nav,    &QCheckBox::toggled,    alias, &RWAlias::setShowInNav);
    connect(case_matching,  QOverload<int>::of(&QComboBox::activated), alias, &RWAlias::setCaseMatchingInt);
    connect(match_priority, QOverload<int>::of(&QComboBox::activated), alias, &RWAlias::setMatchPriorityInt);
    connect(delete_name,    &QPushButton::clicked,  this,  &RWCategoryWidget::remove_name);
    // Don't allow deleting a True Name until all Other Names have been removed.
    if (is_true_name) delete_name->setEnabled(false);

    name->setPlaceholderText(is_true_name ? "Enter True Name" : "Enter New Alias");
    revealed->setToolTip("Revealed");
    auto_accept->setToolTip("Auto Accept");
    show_in_nav->setToolTip("Shown In Nav");
    case_matching->setToolTip("Case Matching");
    match_priority->setToolTip("Priority");
    delete_name->setToolTip(is_true_name ? "Unable to delete True Name" : "Delete this name");

    revealed->setChecked(alias->isRevealed());
    auto_accept->setChecked(alias->isAutoAccept());
    show_in_nav->setChecked(alias->isShowNavPane());
    case_matching->setCurrentIndex(alias->caseMatching());
    match_priority->setCurrentIndex(alias->matchPriority());

    if (alias->namefield().modelColumn() >= 0)
        name->setText(column_name(p_columns, alias->namefield().modelColumn()));

    // Add everything to the layout
    QHBoxLayout *row = new QHBoxLayout;
    row->addWidget(revealed, 0);
    row->addWidget(label,    0);
    row->addWidget(name,     1);
    row->addWidget(case_matching,  0);
    row->addWidget(auto_accept,    0);
    row->addWidget(show_in_nav,    0);
    row->addWidget(match_priority, 0);
    row->addWidget(delete_name,    0);

    // Set properties on the delete button to allow it to do it's work.
    delete_name->setProperty("alias",  QVariant::fromValue(alias));
    delete_name->setProperty("layout", QVariant::fromValue(row));

    // Insert new name immediately before the FIRST section
    QVBoxLayout *top_layout = qobject_cast<QVBoxLayout*>(layout());
    if (p_first_section)
        top_layout->insertLayout(top_layout->indexOf(p_first_section), row);
    else
        top_layout->addLayout(row);
}


void RWCategoryWidget::add_name()
{
    RWAlias *alias = new RWAlias;
    p_category->aliases.append(alias);
    add_rwalias(alias);
}


void RWCategoryWidget::remove_name()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button == 0) return;

    QBoxLayout *row   = button->property("layout").value<QBoxLayout*>();
    RWAlias    *alias = button->property("alias").value<RWAlias*>();
    if (row == 0 || alias == 0) return;

    // Remove alias from the structure
    p_category->aliases.removeAll(alias);
    delete alias;

    // Remove widget from the window
    QVBoxLayout *top_layout = qobject_cast<QVBoxLayout*>(layout());
    top_layout->removeItem(row);
    QLayoutItem *child;
    while ((child = row->takeAt(0)) != 0)
    {
        if (child->widget())
        {
            delete child->widget();
        }
        delete child;
    }
    delete row;
    top_layout->invalidate();
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

    QComboBox *snippet_style = new QComboBox;
    snippet_style->setToolTip("Snippet Style");
    snippet_style->addItem("Normal",     RWFacet::Normal);
    snippet_style->addItem("Read Aloud", RWFacet::Read_Aloud);
    snippet_style->addItem("Message",    RWFacet::Handout);
    snippet_style->addItem("Flavor",     RWFacet::Flavor);
    snippet_style->addItem("Callout",    RWFacet::Callout);
    snippet_style->setCurrentIndex(partition->snippetStyle());
    connect(snippet_style, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] { set_style(snippet_style, partition); } );

    QHBoxLayout *textlayout = new QHBoxLayout;
    textlayout->addWidget(reveal);
    textlayout->addWidget(edit);
    textlayout->addWidget(snippet_style);
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

    // Maybe a field-name to start the line
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

    // Maybe a FILENAME entry field
    if (facet->snippetType() == RWFacet::Picture ||
            facet->snippetType() == RWFacet::Smart_Image ||
            facet->snippetType() == RWFacet::Statblock ||
            facet->snippetType() == RWFacet::Portfolio ||
            facet->snippetType() == RWFacet::Rich_Text ||
            facet->snippetType() == RWFacet::PDF ||
            facet->snippetType() == RWFacet::Audio ||
            facet->snippetType() == RWFacet::Video ||
            facet->snippetType() == RWFacet::HTML ||
            facet->snippetType() == RWFacet::Foreign)
    {
        // A field in which to choose the field for the data (e.g. image) file to be loaded
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

    // Now the text entry field
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
                facet->snippetType() == RWFacet::Smart_Image ||
                facet->snippetType() == RWFacet::Statblock ||
                facet->snippetType() == RWFacet::Portfolio ||
                facet->snippetType() == RWFacet::Picture ||
                facet->snippetType() == RWFacet::Rich_Text ||
                facet->snippetType() == RWFacet::PDF ||
                facet->snippetType() == RWFacet::Audio ||
                facet->snippetType() == RWFacet::Video ||
                facet->snippetType() == RWFacet::HTML)
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
    QComboBox *snippet_style = new QComboBox;
    snippet_style->setToolTip("Snippet Style");
    snippet_style->addItem("Normal",     RWFacet::Normal);
    snippet_style->addItem("Read Aloud", RWFacet::Read_Aloud);
    snippet_style->addItem("Message",    RWFacet::Handout);
    snippet_style->addItem("Flavor",     RWFacet::Flavor);
    snippet_style->addItem("Callout",    RWFacet::Callout);
    snippet_style->setCurrentIndex(facet->snippetStyle());
    connect(snippet_style, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] { set_style(snippet_style, facet); } );

    // Create a row containing all these widgets
    QHBoxLayout *boxl = new QHBoxLayout;
    boxl->setContentsMargins(0,0,0,0);
    if (reveal) boxl->addWidget(reveal);
    if (label) boxl->addWidget(label);
    if (filename) boxl->addWidget(filename);
    if (combo) boxl->addWidget(combo);
    if (edit_widget) boxl->addWidget(edit_widget);
    if (snippet_style) boxl->addWidget(snippet_style);

    QWidget *box = new QWidget;
    box->setProperty("facet", QVariant::fromValue((void*)facet));
    box->setLayout(boxl);

    return box;
}


void RWCategoryWidget::set_style(QComboBox *combo, RWBaseItem *facet)
{
    facet->setSnippetStyle(combo->currentData().value<RWFacet::SnippetStyle>());
}
