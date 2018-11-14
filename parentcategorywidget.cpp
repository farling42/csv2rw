/*
CSV2RW
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

#include "parentcategorywidget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include "rw_topic.h"
#include "rw_topic_widget.h"
#include "realmworksstructure.h"

ParentCategoryWidget::ParentCategoryWidget(RealmWorksStructure *structure, QAbstractItemModel *columns, int indent, RWTopic *topic, QWidget *parent) :
    QFrame(parent),
    structure(structure),
    category_widget(nullptr),
    header_model(columns),
    ignore_select(false)
{
    //setFrameStyle(QFrame::Panel | QFrame::Sunken);

    combo = new QComboBox;
    category_area = new QHBoxLayout;
    delete_button = new QPushButton;
    delete_button->setIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton));
    delete_button->setToolTip("Remove this parent");
    delete_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QHBoxLayout *main_layout = new QHBoxLayout;
    main_layout->setContentsMargins(0,0,0,0);
    if (indent > 0) main_layout->addSpacing(indent);
    main_layout->addWidget(combo, 0);
    main_layout->addLayout(category_area, 1);
    main_layout->addWidget(delete_button, 0);
    setLayout(main_layout);

    connect(delete_button, &QPushButton::clicked, this, &ParentCategoryWidget::deleteRequested);
    connect(combo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &ParentCategoryWidget::select_category);

    // Prevent slot from triggering if we are adding an explicit topic
    ignore_select = (topic != nullptr);

    // Set up the list of available categories
    QStringList cats;
    for (auto category: structure->categories)
    {
        cats.append(category->name());
    }
    cats.sort();
    combo->clear();
    combo->addItems(cats);

    if (topic)
    {
        combo->setCurrentText(topic->structure->name());
        category_widget = new RWTopicWidget(topic, header_model, /*include_sections*/ false); // TODO - create a hierarchy of RWContentItems
        category_area->addWidget(category_widget);
    }
    ignore_select = false;
}


RWTopic *ParentCategoryWidget::topic() const
{
    return category_widget ? category_widget->topic() : nullptr;
}

void ParentCategoryWidget::select_category(const QString &selection)
{
    if (ignore_select) return;

    RWCategory *new_category = nullptr;
    for (auto category: structure->categories)
    {
        if (category->name() == selection)
        {
            new_category = category;
            break;
        }
    }
    if (new_category == nullptr) return;

    if (category_widget)
    {
        category_area->removeWidget(category_widget);
        category_widget->deleteLater();
    }
    RWTopic *topic = qobject_cast<RWTopic*>(new_category->createContentsTree());
    category_widget = new RWTopicWidget(topic, header_model, /*include_sections*/ false);
    category_area->addWidget(category_widget);

    emit categoryChanged();
}

void ParentCategoryWidget::setCanDelete(bool v)
{
    delete_button->setEnabled(v);
}
