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

#include "rw_topic_widget.h"

#include <QAbstractItemModel>
#include <QActionGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QIcon>
#include <QLabel>
#include <QFrame>
#include <QBoxLayout>
#include <QMenu>
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
#include "rw_section.h"
#include "rw_snippet.h"
#include "rw_topic.h"
#include "topickey.h"

static QMetaEnum case_matching_enum   = QMetaEnum::fromType<RWAlias::CaseMatching>();
static QMetaEnum match_priority_enum  = QMetaEnum::fromType<RWAlias::MatchPriority>();

static inline QString column_name(QAbstractItemModel *model, int column)
{
    return model->index(column, 0).data().toString();
}

/**
 * @brief RWTopicWidget::RWTopicWidget
 *
 * Creates the widget which shows the entire structure of this particular category.
 *
 * @param category
 * @param parent
 */
RWTopicWidget::RWTopicWidget(RWTopic *topic, QAbstractItemModel *columns, bool include_sections, QWidget *parent) :
    QFrame(parent),
    p_columns(columns),
    p_topic(topic),
    p_key(nullptr)
{
    RWStructureItem *description;
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    // Start with the title (+ prefix + suffix)
    QRadioButton  *reveal = new QRadioButton(QString());
    if (include_sections) p_key = new QPushButton(style()->standardIcon(QStyle::SP_MessageBoxQuestion), QString());
    FieldLineEdit *name   = new FieldLineEdit(topic->publicName().namefield());
    FieldLineEdit *prefix = new FieldLineEdit(topic->prefix());
    FieldLineEdit *suffix = new FieldLineEdit(topic->suffix());
    QPushButton   *addName = new QPushButton("+Name");

    addName->setToolTip("Adds a True Name/Other Name\n"
                        "Any name whose CSV cell is empty will not be created in RealmWorks.\n"
                        "This allows different sets of attributes to set for different alternative names.");

    // Should the category be marked as revealed
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    reveal->setChecked(topic->isRevealed());
    connect(reveal, &QRadioButton::toggled, topic, &RWContentsItem::setIsRevealed);

    if (p_key)
    {
        connect(p_key, &QPushButton::clicked, this, &RWTopicWidget::show_key);
        set_key_tooltip();
    }

    name->setPlaceholderText("<name>");
    description = topic->category->childElement("description");     // TODO - probably need to find the local version
    name->setToolTip(description ? description->structureText() : topic->category->name());
    if (topic->publicName().namefield().modelColumn() >= 0)
        name->setText(column_name(columns, topic->publicName().namefield().modelColumn()));

    prefix->setPlaceholderText("<prefix>");
    prefix->setToolTip("prefix");
    if (topic->prefix().modelColumn() >= 0)
        prefix->setText(column_name(columns, topic->prefix().modelColumn()));

    suffix->setPlaceholderText("<suffix>");
    suffix->setToolTip("suffix");
    if (topic->suffix().modelColumn() >= 0)
        suffix->setText(column_name(columns, topic->suffix().modelColumn()));

    connect(addName, &QPushButton::clicked, this, &RWTopicWidget::add_name);

    QBoxLayout *title = new QHBoxLayout;
    title->addWidget(reveal,  0);
    if (p_key) title->addWidget(p_key,   0);
    title->addWidget(name,    2);
    title->addWidget(prefix,  1);
    title->addWidget(suffix,  1);
    add_name_attributes(title, &topic->publicName());
    title->addWidget(addName, 0);
    layout->addLayout(title);

    // For a parent category, we don't want to allow sections to be defined.
    if (include_sections)
    {
        // Now deal with the sections
        QList<int> sections;
        sections.append(1);
        p_first_section = nullptr;
        for (auto section: topic->childItems<RWSection*>())
        {
            QWidget *sec = add_section(sections, columns, section);
            if (p_first_section == nullptr) p_first_section = sec;
            layout->addWidget(sec);
            sections.last()++;
        }
        layout->addStretch(2);
    }
    setLayout(layout);

    // Maybe they've switched to a category with some name entries already present.
    for (auto alias: p_topic->aliases)
        add_rwalias(alias);
}


void RWTopicWidget::add_name_attributes(QBoxLayout *layout, RWAlias *alias)
{
    QCheckBox     *auto_accept    = new QCheckBox;
    QCheckBox     *show_in_nav    = new QCheckBox;
    QComboBox     *case_matching  = new QComboBox;
    QComboBox     *match_priority = new QComboBox;

#if 0
    // XML uses "Correction" whereas RW uses "Correct"
    for (int key=0; key<case_matching_enum.keyCount(); key++)
        case_matching->addItem(case_matching_enum.key(key));
#else
    case_matching->addItem("Ignore");
    case_matching->addItem("Sensitive");
    case_matching->addItem("Correct"); // XML uses "Correction"
#endif

    for (int key=0; key<match_priority_enum.keyCount(); key++)
        match_priority->addItem(match_priority_enum.key(key));

    connect(auto_accept,    &QCheckBox::toggled,    alias, &RWAlias::setAutoAccept);
    connect(show_in_nav,    &QCheckBox::toggled,    alias, &RWAlias::setShowInNav);
    connect(case_matching,  QOverload<int>::of(&QComboBox::activated), alias, &RWAlias::setCaseMatchingInt);
    connect(match_priority, QOverload<int>::of(&QComboBox::activated), alias, &RWAlias::setMatchPriorityInt);

    auto_accept->setToolTip("Auto Accept");
    show_in_nav->setToolTip("Shown In Nav");
    case_matching->setToolTip("Case Matching");
    match_priority->setToolTip("Priority");

    auto_accept->setChecked(alias->isAutoAccept());
    show_in_nav->setChecked(alias->isShowNavPane());
    case_matching->setCurrentIndex(alias->caseMatching());
    match_priority->setCurrentIndex(alias->matchPriority());

    layout->addWidget(case_matching,  0);
    layout->addWidget(auto_accept,    0);
    layout->addWidget(show_in_nav,    0);
    layout->addWidget(match_priority, 0);
}


void RWTopicWidget::add_rwalias(RWAlias *alias)
{
    bool is_true_name = (p_topic->aliases.first() == alias);

    alias->setIsTrueName(is_true_name);

    QLabel        *label          = new QLabel(is_true_name ? "True Name :" : "Other Name:");
    QRadioButton  *reveal         = new QRadioButton(QString());
    FieldLineEdit *name           = new FieldLineEdit(alias->namefield());
    QPushButton   *delete_name    = new QPushButton(style()->standardIcon(QStyle::SP_LineEditClearButton), "");

    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");

    connect(reveal,         &QRadioButton::toggled, alias, &RWAlias::setRevealed);
    // Don't allow deleting a True Name until all Other Names have been removed.
    if (is_true_name) delete_name->setEnabled(false);

    name->setPlaceholderText(is_true_name ? "Enter True Name" : "Enter New Alias");
    reveal->setToolTip("Revealed");
    delete_name->setToolTip(is_true_name ? "Unable to delete True Name" : "Delete this name");

    reveal->setChecked(alias->isRevealed());

    if (alias->namefield().modelColumn() >= 0)
        name->setText(column_name(p_columns, alias->namefield().modelColumn()));

    // Add everything to the layout
    QHBoxLayout *row = new QHBoxLayout;
    row->addWidget(reveal, 0);
    row->addWidget(label,  0);
    row->addWidget(name,   1);  // the only stretched field
    add_name_attributes(row, alias);
    row->addWidget(delete_name, 0);

    // Connect the "remove alias" button with the correct parameters
    connect(delete_name, &QPushButton::clicked,  [=]() { remove_name(row, alias); });

    // Insert new name immediately before the FIRST section
    QVBoxLayout *top_layout = qobject_cast<QVBoxLayout*>(layout());
    if (p_first_section)
        top_layout->insertLayout(top_layout->indexOf(p_first_section), row);
    else
        top_layout->addLayout(row);
}

void RWTopicWidget::set_key_tooltip()
{
    if (p_key == nullptr) return;

    if (p_topic->keyColumn() < 0)
        p_key->setToolTip("Choose a subset of CSV rows.\nChoose which rows of the table will be used to create one of these topics");
    else
        p_key->setToolTip(QString("This topic will be created for rows where:\n%1 = '%2'").arg(p_columns->index(p_topic->keyColumn(), 0).data().toString()).arg(p_topic->keyValue()));
}

void RWTopicWidget::add_name()
{
    RWAlias *alias = new RWAlias;
    p_topic->aliases.append(alias);
    add_rwalias(alias);
}


void RWTopicWidget::remove_name(QHBoxLayout *row, RWAlias *alias)
{
    if (row == nullptr || alias == nullptr) return;

    // Remove alias from the structure
    p_topic->aliases.removeAll(alias);
    delete alias;

    // Remove widget from the window
    QVBoxLayout *top_layout = qobject_cast<QVBoxLayout*>(layout());
    top_layout->removeItem(row);
    while (QLayoutItem *child = row->takeAt(0))
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

void RWTopicWidget::show_key()
{
    static TopicKey *key_dialog = nullptr;
    if (key_dialog == nullptr)
    {
        key_dialog = new TopicKey(parentWidget());
    }
    key_dialog->setSelectedColumn(p_topic->keyColumn());
    key_dialog->setSelectedValue(p_topic->keyValue());
    if (key_dialog->exec() == QDialog::Accepted)
    {
        p_topic->setKeyColumn(key_dialog->selectedColumn());
        p_topic->setKeyValue(key_dialog->selectedValue());
        set_key_tooltip();
    }
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

QWidget *RWTopicWidget::add_section(QList<int> sections, QAbstractItemModel *columns, RWSection *section)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,/*top*/9,0,0);

    // Start with the title bar for the section
    QFrame *title_frame = new QFrame;
    title_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);

    QVBoxLayout *title_vlayout = new QVBoxLayout;
    title_vlayout->setContentsMargins(0,0,0,0);

    // First row is the section title
    QLabel *title_label = new QLabel;
    title_label->setLineWidth(2);
    title_label->setMargin(3);
    title_label->setText(QString("%1  %2").arg(section_string(sections)).arg(section->partition->name()));
    RWStructureItem *description = section->partition->childElement("description");
    title_label->setToolTip(description ? description->structureText() : section->partition->id());

    // Second row of the title starts hidden
    FieldLineEdit *first_multiple  = new FieldLineEdit(section->firstMultiple());
    FieldLineEdit *second_multiple = new FieldLineEdit(section->secondMultiple());
    FieldLineEdit *last_multiple   = new FieldLineEdit(section->lastMultiple());
    first_multiple->setPlaceholderText("First Section header");
    first_multiple->setToolTip("Select the CSV column to use for the first heading");
    second_multiple->setPlaceholderText("Second Section header");
    second_multiple->setToolTip("If defined, sets the CSV column for the second heading when creating multiple sections");
    last_multiple->setPlaceholderText("Last Section header");
    last_multiple->setToolTip("If defined, sets the last CSV column to be used when creating multiple sections");
    if (section->firstMultiple().modelColumn() >= 0)
        first_multiple->setText(column_name(columns, section->firstMultiple().modelColumn()));
    if (section->secondMultiple().modelColumn() >= 0)
        second_multiple->setText(column_name(columns, section->secondMultiple().modelColumn()));
    if (section->lastMultiple().modelColumn() >= 0)
        last_multiple->setText(column_name(columns, section->lastMultiple().modelColumn()));

    QWidget *multiple_subsection = new QWidget;
    multiple_subsection->setVisible(section->p_is_multiple);

    // A button to toggle the display of multiple_subsection
    QPushButton *multiple = new QPushButton("...");
    multiple->setCheckable(true);
    multiple->setFlat(true);
    multiple->setToolTip("Configure custom section header or multiple sections to be created");
    connect(multiple, &QPushButton::clicked, [=] {
        section->p_is_multiple = multiple->isChecked();
        multiple_subsection->setVisible(section->p_is_multiple);
    } );

    QHBoxLayout *title_hlayout1 = new QHBoxLayout;
    title_vlayout->setContentsMargins(0,0,0,0);
    title_hlayout1->addWidget(title_label, 1);
    title_hlayout1->addWidget(multiple, 0);
    title_hlayout1->addWidget(create_section_options(section), 0);

    QHBoxLayout *title_hlayout2 = new QHBoxLayout;
    title_hlayout2->setContentsMargins(0,0,0,0);
    title_hlayout2->addWidget(new QLabel("Multiple Section Columns:"));
    title_hlayout2->addWidget(first_multiple);
    title_hlayout2->addWidget(second_multiple);
    title_hlayout2->addWidget(last_multiple);
    title_hlayout2->addStretch();
    multiple_subsection->setLayout(title_hlayout2);

    title_vlayout->addLayout(title_hlayout1);
    title_vlayout->addWidget(multiple_subsection);

    title_frame->setLayout(title_vlayout);
    layout->addWidget(title_frame);

    QFont bold_font = title_label->font();
    bold_font.setBold(true);
    title_label->setFont(bold_font);

    // Now the body of the section
    for (auto snippet: section->childItems<RWSnippet*>())
    {
        layout->addWidget(add_snippet(columns, snippet));
    }

    // a REVEALED button for the CONTENTS
    QRadioButton *reveal = new QRadioButton(QString());
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    reveal->setChecked(section->isRevealed());
    connect(reveal, &QRadioButton::toggled, section, &RWContentsItem::setIsRevealed);

    // A generic TEXT box, allowing one or more snippets to be created as the contents for this section
    FieldMultiLineEdit *edit = new FieldMultiLineEdit(section->contentsText());
    edit->setToolTip("contents");
    RWStructureItem *purpose = section->partition->childElement("purpose");
    edit->setPlaceholderText(purpose ? purpose->structureText() : "<purpose>");
    if (section->contentsText().modelColumn() >= 0)
        edit->setText(column_name(columns, section->contentsText().modelColumn()));

    // Allow for MULTIPLE content snippets to be created from the same basic facet
    FieldLineEdit *multi_edit = new FieldLineEdit(section->lastContents());
    if (section->lastContents().modelColumn() >= 0)
        multi_edit->setText(column_name(columns, section->lastContents().modelColumn()));
    multi_edit->setPlaceholderText("Multiple");
    multi_edit->setToolTip("Leave this blank, unless you want multiple content snippets in which case set it to the LAST column containing the contents");

    QHBoxLayout *textlayout = new QHBoxLayout;
    textlayout->addWidget(reveal, 0);
    textlayout->addWidget(edit, 1);
    textlayout->addWidget(multi_edit, 0);
    textlayout->addWidget(create_snippet_options(section));
    layout->addLayout(textlayout);

    // Then display the sub-partitions
    QWidget *first_sub = nullptr;
    sections.append(1);
    for (auto child: section->childItems<RWSection*>())
    {
        QWidget *sub_part = add_section (sections, columns, child);
        if (first_sub == nullptr) first_sub = sub_part;
        layout->addWidget(sub_part);
        sections.last()++;
    }
    sections.removeLast();

    QFrame *frame = new QFrame;
    frame->setLayout(layout);
    return frame;
}

QWidget *RWTopicWidget::add_snippet(QAbstractItemModel *columns, RWSnippet *snippet)
{
    QRadioButton *reveal = nullptr;
    QLabel *label = nullptr;
    FieldLineEdit *filename = nullptr;
    FieldLineEdit *start_date = nullptr;
    FieldLineEdit *finish_date = nullptr;
    FieldLineEdit *number = nullptr;
    FieldComboBox *combo = nullptr;
    QWidget *edit_widget = nullptr;

    // Every snippet is revealable
    reveal = new QRadioButton(QString());
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    reveal->setChecked(snippet->isRevealed());
    connect(reveal, &QRadioButton::toggled, snippet, &RWContentsItem::setIsRevealed);

    const RWFacet *facet = snippet->facet;

    // Maybe a field-name to start the line
    if (facet->snippetType() == RWFacet::Labeled_Text ||
        facet->snippetType() == RWFacet::Hybrid_Tag   ||
        facet->snippetType() == RWFacet::Tag_Standard ||
        facet->snippetType() == RWFacet::Date_Game    ||
        facet->snippetType() == RWFacet::Date_Range   ||
        facet->snippetType() == RWFacet::Numeric)
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
// Tag_Standard

    // Maybe a tag selector
    if (facet->snippetType() == RWFacet::Hybrid_Tag ||
        facet->snippetType() == RWFacet::Tag_Standard )
    {
        combo = new FieldComboBox(snippet->tags(), RWDomain::getDomainById(facet->attributes().value("domain_id").toString()));
        if (snippet->tags().modelColumn() >= 0)
            combo->setIndexString(column_name(columns, snippet->tags().modelColumn()));
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
        filename = new FieldLineEdit(snippet->filename());
        filename->setPlaceholderText(facet->name());
        filename->setToolTip(tr("File containing asset"));
        // Change the background colour
        QPalette p = filename->palette();
        p.setBrush(QPalette::Base, p.button());
        filename->setPalette(p);
        filename->setBackgroundRole(QPalette::Button);
        filename->setMaximumWidth(100);
        if (snippet->filename().modelColumn() >= 0)
        {
            filename->setText(column_name(columns, snippet->filename().modelColumn()));
        }
    }

    if (facet->snippetType() == RWFacet::Date_Game ||
            facet->snippetType() == RWFacet::Date_Range)
    {
        start_date = new FieldLineEdit(snippet->startDate());
        start_date->setToolTip((facet->snippetType() == RWFacet::Date_Game) ? "Date" : "Start Date");
        start_date->setPlaceholderText(facet->name());
        // Change the background colour
        QPalette p = start_date->palette();
        p.setBrush(QPalette::Base, p.button());
        start_date->setPalette(p);
        start_date->setBackgroundRole(QPalette::Button);
        start_date->setMaximumWidth(100);
        if (snippet->startDate().modelColumn() >= 0)
        {
            start_date->setText(column_name(columns, snippet->startDate().modelColumn()));
        }

        // A second field is required to finish the range
        if (facet->snippetType() == RWFacet::Date_Range)
        {
            finish_date = new FieldLineEdit(snippet->finishDate());
            finish_date->setToolTip("Finish Date");
            finish_date->setPlaceholderText(facet->name());
            // Change the background colour
            QPalette p = finish_date->palette();
            p.setBrush(QPalette::Base, p.button());
            finish_date->setPalette(p);
            finish_date->setBackgroundRole(QPalette::Button);
            finish_date->setMaximumWidth(100);
            if (snippet->finishDate().modelColumn() >= 0)
            {
                finish_date->setText(column_name(columns, snippet->finishDate().modelColumn()));
            }
        }
    }
    if (facet->snippetType() == RWFacet::Numeric)
    {
        number = new FieldLineEdit(snippet->number());
        number->setMaximumWidth(100);
        number->setToolTip(tr("Numeric value"));
        // Change the background colour
        QPalette p = number->palette();
        p.setBrush(QPalette::Base, p.button());
        number->setPalette(p);
        number->setBackgroundRole(QPalette::Button);
        number->setPlaceholderText(facet->name());
        if (snippet->number().modelColumn() >= 0)
        {
            number->setText(column_name(columns, snippet->number().modelColumn()));
        }
    }

    // Now the text entry field
    if (facet->snippetType() == RWFacet::Multi_Line)
    {
        FieldMultiLineEdit *edit = new FieldMultiLineEdit(snippet->contentsText());
        edit_widget = edit;

        // Use the <description> child as a tool tip, if available
        RWContentsItem *description = snippet->childElement("description");
        edit->setToolTip(description ? description->structureText() : facet->uuid());
        edit->setPlaceholderText(facet->name());
        if (snippet->contentsText().modelColumn() >= 0)
        {
            edit->setText(column_name(columns, snippet->contentsText().modelColumn()));
        }
    }
    else
    {
        FieldLineEdit *edit = new FieldLineEdit(snippet->contentsText());
        edit_widget = edit;

        // Use the <description> child as a tool tip, if available
        RWContentsItem *description = snippet->childElement("description");
        edit->setToolTip(description ? description->structureText() : facet->uuid());
        if (facet->snippetType() == RWFacet::Hybrid_Tag ||
                facet->snippetType() == RWFacet::Tag_Standard ||
                facet->snippetType() == RWFacet::Numeric ||
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
        if (snippet->contentsText().modelColumn() >= 0)
        {
            edit->setText(column_name(columns, snippet->contentsText().modelColumn()));
        }
    }

    // Finally some selectable options for this snippet
    QWidget *options_button = create_snippet_options(snippet);

    //
    // Create a row containing all these widgets
    //
    QHBoxLayout *boxl = new QHBoxLayout;
    boxl->setContentsMargins(0,0,0,0);
    if (reveal) boxl->addWidget(reveal);
    if (label) boxl->addWidget(label);
    if (filename) boxl->addWidget(filename);
    if (start_date) boxl->addWidget(start_date);
    if (finish_date) boxl->addWidget(finish_date);
    if (combo) boxl->addWidget(combo);
    if (number) boxl->addWidget(number);
    if (edit_widget) boxl->addWidget(edit_widget);
    if (options_button) boxl->addWidget(options_button);

    // And the actual widget to contain the row's layout
    QWidget *box = new QWidget;
    box->setLayout(boxl);

    return box;
}

template<typename T>
QActionGroup *RWTopicWidget::create_enum_actions(const QString &section_name, T current_value, QMenu *menu, QMap<QString,QString> &rename)
{
    // Windows doesn't support menu->addSection, so emulate it outself.
    menu->addSeparator();
    QAction *title = new QAction(section_name, menu);
    title->setEnabled(false);
    menu->addAction(title);

    QActionGroup *group = new QActionGroup(menu);
    QMetaEnum meta = QMetaEnum::fromType<T>();
    for (int idx=0; idx<meta.keyCount(); idx++)
    {
        QString label = QString(meta.valueToKey(idx)).replace("_", " ");
        QAction *action = new QAction(rename.value(label,label), group);
        action->setCheckable(true);
        T value = T(meta.value(idx));
        action->setData(value);
        menu->addAction(action);
        if (value == current_value) action->setChecked(true);
    }
    return group;
}

/**
 * @brief RWTopicWidget::create_snippet_options
 * Creates the options drop-down menu button for SECTIONS
 * @param item
 * @return
 */

QWidget *RWTopicWidget::create_section_options(RWSection *item)
{
    // Add button to bring up the options menu for the snippet
    QMenu *options_menu = new QMenu(this);

    QAction *start_collapsed = options_menu->addAction("Start Collapsed");
    start_collapsed->setCheckable(true);
    start_collapsed->setChecked(item->p_start_collapsed);
    connect(start_collapsed, &QAction::triggered, [=] {
        item->p_start_collapsed = start_collapsed->isChecked();
    } );

    // Create the OPTIONS button
    QPushButton *options_button = new QPushButton(QIcon(":cog-icon"), QString());
    options_button->setFlat(true);
    options_button->setMenu(options_menu);
    return options_button;
}

/**
 * @brief RWTopicWidget::create_snippet_options
 * Creates the options drop-down menu button for SNIPPETS
 * @param item
 * @return
 */
QWidget *RWTopicWidget::create_snippet_options(RWContentsItem *item)
{
    // Add button to bring up the options menu for the snippet
    QMenu *options_menu = new QMenu(this);

    // SUB-MENU for the snippet style
    QMap<QString,QString> style_remap{ {"Handout", "Message"} };
    QActionGroup *styles = create_enum_actions<RWContentsItem::SnippetStyle>("Snippet Style", item->snippetStyle(), options_menu, style_remap);
    // For SnippetStyle, RWFacet::Handout needs to use the label "Message"
    for (auto action: styles->actions())
        if (action->text() == "Handout") action->setText("Message");
    connect(styles, &QActionGroup::triggered, [=] {
        if (QAction *act = styles->checkedAction())
            item->setSnippetStyle(act->data().value<RWContentsItem::SnippetStyle>());
    } );

    QMap<QString,QString> veracity_remap{{"Truth", "True"}, {"Partial", "Partially True"}, {"Lie", "Untrue"}};
    QActionGroup *veracity = create_enum_actions<RWContentsItem::SnippetVeracity>("Truth Level", item->snippetVeracity(), options_menu, veracity_remap);
    connect(veracity, &QActionGroup::triggered, [=] {
        if (QAction *act = veracity->checkedAction())
            item->setSnippetVeracity(act->data().value<RWContentsItem::SnippetVeracity>());
    } );

#if 0
    // Crashes RW on import
    QMap<QString,QString> purpose_remap;
    QActionGroup *purpose = create_enum_actions<RWContentsItem::SnippetPurpose>("Purpose", item->snippetPurpose(), options_menu, purpose_remap);
    connect(purpose, &QActionGroup::triggered, [=] {
        if (QAction *act = purpose->checkedAction())
            item->setSnippetPurpose(act->data().value<RWContentsItem::SnippetPurpose>());
    } );
#endif

    // Create the OPTIONS button
    QPushButton *options_button = new QPushButton(QIcon(":cog-icon"), QString());
    options_button->setFlat(true);
    options_button->setMenu(options_menu);
    return options_button;
}
