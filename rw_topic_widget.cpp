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
    FieldLineEdit *name   = new FieldLineEdit(topic->namefield());
    FieldLineEdit *prefix = new FieldLineEdit(topic->prefix());
    FieldLineEdit *suffix = new FieldLineEdit(topic->suffix());
    QPushButton   *addName = new QPushButton("+Name");

    addName->setToolTip("Adds a True Name/Other Name\n"
                        "Any name whose CSV cell is empty will not be created in RealmWorks.\n"
                        "This allows different sets of attributes to set for different alternative names.");

    // Should the category be marked as revealed
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    connect(reveal, &QRadioButton::toggled, topic, &RWContentsItem::setIsRevealed);

    if (p_key)
    {
        connect(p_key, &QPushButton::clicked, this, &RWTopicWidget::show_key);
        set_key_tooltip();
    }

    name->setPlaceholderText("<name>");
    description = topic->category->childElement("description");     // TODO - probably need to find the local version
    name->setToolTip(description ? description->structureText() : topic->category->name());
    if (topic->namefield().modelColumn() >= 0)
        name->setText(column_name(columns, topic->namefield().modelColumn()));

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
    title->addWidget(addName, 0);
    layout->addLayout(title);

    // For a parent category, we don't want to allow sections to be defined.
    if (include_sections)
    {
        // Now deal with the sections
        QList<int> sections;
        sections.append(1);
        QList<RWSection*> child_items = topic->childItems<RWSection*>();
        p_first_section = 0;
        foreach (RWSection *child, child_items)
        {
            QWidget *sec = add_section(sections, columns, child);
            if (p_first_section == 0) p_first_section = sec;
            layout->addWidget(sec);
            sections.last()++;
        }
        layout->addStretch(2);
    }
    setLayout(layout);

    // Maybe they've switched to a category with some name entries already present.
    foreach (RWAlias *alias, p_topic->aliases)
        add_rwalias(alias);
}


void RWTopicWidget::add_rwalias(RWAlias *alias)
{
    bool is_true_name = (p_topic->aliases.first() == alias);

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
    connect(delete_name,    &QPushButton::clicked,  this,  &RWTopicWidget::remove_name);
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


void RWTopicWidget::remove_name()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button == 0) return;

    QBoxLayout *row   = button->property("layout").value<QBoxLayout*>();
    RWAlias    *alias = button->property("alias").value<RWAlias*>();
    if (row == 0 || alias == 0) return;

    // Remove alias from the structure
    p_topic->aliases.removeAll(alias);
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

void RWTopicWidget::show_key()
{
    static TopicKey *key_dialog = 0;
    if (key_dialog == 0)
    {
        key_dialog = new TopicKey(parentWidget());
    }
    key_dialog->setSelectedColumn(p_topic->keyColumn());
    key_dialog->setSelectedValue(p_topic->keyValue());
    if (key_dialog->exec() == QDialog::Accepted)
    {
        p_topic->setKeyColumn(key_dialog->selectedColumn());
        p_topic->setKeyValue(key_dialog->selectedValue());
        qDebug() << "show_key: column" << p_topic->keyColumn() << p_topic->keyValue();

        set_key_tooltip();
    }
}


void RWTopicWidget::do_insert()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button == 0) return;
    QBoxLayout *layout = button->property("where").value<QBoxLayout*>();
    if (layout == 0) return;

    // Add an additional contents field
    FieldLineEdit *edit = new FieldLineEdit(p_topic->contentsText());
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

QWidget *RWTopicWidget::add_section(QList<int> sections, QAbstractItemModel *columns, RWSection *section)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,/*top*/9,0,0);

    QLabel *title = new QLabel;
    title->setFrameStyle(QFrame::Panel | QFrame::Raised);
    title->setLineWidth(2);
    title->setMargin(3);
    title->setText(QString("%1  %2").arg(section_string(sections)).arg(section->partition->name()));
    RWStructureItem *description = section->partition->childElement("description");
    title->setToolTip(description ? description->structureText() : section->partition->id());
    layout->addWidget(title);

    QFont bold_font = title->font();
    bold_font.setBold(true);
    title->setFont(bold_font);

    QList<RWSnippet*> child_snippets = section->childItems<RWSnippet*>();
    foreach (RWSnippet *snippet, child_snippets)
    {
        layout->addWidget(add_snippet(columns, snippet));
    }

    // Finally a generic text box (the contents + reveal)
    QRadioButton *reveal = new QRadioButton(QString());
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    reveal->setChecked(section->isRevealed());
    connect(reveal, &QRadioButton::toggled, section, &RWContentsItem::setIsRevealed);

    FieldMultiLineEdit *edit = new FieldMultiLineEdit(section->contentsText());
    edit->setToolTip("contents");
    RWStructureItem *purpose = section->partition->childElement("purpose");
    edit->setPlaceholderText(purpose ? purpose->structureText() : "<purpose>");
    if (section->contentsText().modelColumn() >= 0)
        edit->setText(column_name(columns, section->contentsText().modelColumn()));

    QWidget *options = create_option_button(section);

    QHBoxLayout *textlayout = new QHBoxLayout;
    textlayout->addWidget(reveal);
    textlayout->addWidget(edit);
    textlayout->addWidget(options);
    layout->addLayout(textlayout);

#ifdef ADD_SNIPPET
    // Add a button to allow a second <contents> section to be added.
    QPushButton *insert_button = new QPushButton("+");
    connect(insert_button, &QPushButton::clicked, this, &RWTopicWidget::do_insert);
    insert_button->setProperty("where", QVariant::fromValue(layout));
    layout->addWidget(insert_button);
#endif

    // Then display the sub-partitions
    QWidget *first_sub = 0;
    sections.append(1);
    QList<RWSection*> child_sections = section->childItems<RWSection*>();
    foreach (RWSection *child, child_sections)
    {
        QWidget *sub_part = add_section (sections, columns, child);
        if (first_sub == 0) first_sub = sub_part;
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
    QRadioButton *reveal = 0;
    QLabel *label = 0;
    FieldLineEdit *filename = 0;
    FieldLineEdit *start_date = 0;
    FieldLineEdit *finish_date = 0;
    FieldLineEdit *number = 0;
    FieldComboBox *combo = 0;
    QWidget *edit_widget = 0;

    // Every snippet is revealable
    reveal = new QRadioButton(QString());
    reveal->setAutoExclusive(false);
    reveal->setToolTip("revealed?");
    connect(reveal, &QRadioButton::toggled, snippet, &RWContentsItem::setIsRevealed);

    const RWFacet *facet = snippet->facet;

    // Maybe a field-name to start the line
    if (facet->snippetType() == RWFacet::Labeled_Text ||
        facet->snippetType() == RWFacet::Hybrid_Tag   ||
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
//Multi_Line, Hybrid_Tag, Labeled_Text, Tag_Standard, Picture

    // Maybe a tag selector
    if (facet->snippetType() == RWFacet::Hybrid_Tag)
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

    if (facet->snippetType() == RWFacet::Date_Game || facet->snippetType() == RWFacet::Date_Range)
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
        if (facet->snippetType() == RWFacet::Hybrid_Tag)
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
    else
    {
        FieldLineEdit *edit = new FieldLineEdit(snippet->contentsText());
        edit_widget = edit;

        // Use the <description> child as a tool tip, if available
        RWContentsItem *description = snippet->childElement("description");
        edit->setToolTip(description ? description->structureText() : facet->uuid());
        if (facet->snippetType() == RWFacet::Hybrid_Tag ||
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
    QWidget *options_button = create_option_button(snippet);

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
    box->setProperty("facet", QVariant::fromValue((void*)facet));
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


QWidget *RWTopicWidget::create_option_button(RWContentsItem *item)
{
    // Add button to bring up the options menu for the snippet
    QMenu *options_menu = new QMenu(this);

    // SUB-MENU for the snippet style
    QMap<QString,QString> style_remap{ {"Handout", "Message"} };
    QActionGroup *styles = create_enum_actions<RWContentsItem::SnippetStyle>("Snippet Style", item->snippetStyle(), options_menu, style_remap);
    // For SnippetStyle, RWFacet::Handout needs to use the label "Message"
    foreach (QAction *action, styles->actions())
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

    //QPushButton *options_button = new QPushButton(style()->standardIcon(QStyle::SP_CommandLink), QString());
    QPushButton *options_button = new QPushButton("Options");
    options_button->setMenu(options_menu);
    return options_button;
}
