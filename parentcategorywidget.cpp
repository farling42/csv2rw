#include "parentcategorywidget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include "rwcategorywidget.h"
#include "realmworksstructure.h"

ParentCategoryWidget::ParentCategoryWidget(RealmWorksStructure *structure, QAbstractItemModel *columns, int indent, QWidget *parent) :
    QFrame(parent),
    structure(structure),
    category_widget(0),
    header_model(columns)
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
    connect(combo, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &ParentCategoryWidget::select_category);

    // Set up the list of available categories
    QStringList cats;
    foreach (RWCategory *category, structure->categories)
    {
        cats.append(category->name());
    }
    cats.sort();
    combo->clear();
    combo->addItems(cats);
}


RWCategory *ParentCategoryWidget::category()
{
    return category_widget ? category_widget->category() : 0;
}

void ParentCategoryWidget::select_category(const QString &selection)
{
    RWCategory *new_category = 0;
    foreach (RWCategory *category, structure->categories)
    {
        if (category->name() == selection)
        {
            new_category = category;
            break;
        }
    }
    if (new_category == 0) return;

    if (category_widget)
    {
        category_area->removeWidget(category_widget);
        category_widget->deleteLater();
    }
    category_widget = new RWCategoryWidget(new_category, header_model, /*include_sections*/ false);
    category_area->addWidget(category_widget);
}


void ParentCategoryWidget::setCanDelete(bool v)
{
    delete_button->setEnabled(v);
}
