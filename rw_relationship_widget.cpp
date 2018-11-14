#include "rw_relationship_widget.h"
#include <QHBoxLayout>
#include <QRadioButton>
#include <QMetaEnum>
#include <QPushButton>
#include <QComboBox>
#include <QDebug>
#include "fieldlineedit.h"
#include "rw_domain.h"

static QMetaEnum nature_enum   = QMetaEnum::fromType<RWRelationship::Nature>();
static QMetaEnum attitude_enum = QMetaEnum::fromType<RWRelationship::Attitude>();

static inline QString column_name(QAbstractItemModel *model, int column)
{
    return model->index(column, 0).data().toString();
}



RWRelationshipWidget::RWRelationshipWidget(RWRelationship *relationship, QAbstractItemModel *columns, QWidget *parent) :
    QWidget(parent),
    p_relationship(relationship)
{
    // Reveal button
    // Connection Nature
    // (optional) Connection qualifier
    // Related Topic
    // Delete button

    reveal = new QRadioButton;
    nature = new QComboBox;
    qualifier = new QComboBox;
    qualifier->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    this_link  = new FieldLineEdit(p_relationship->thisLink());
    other_link = new FieldLineEdit(p_relationship->otherLink());
    remove = new QPushButton;
    remove->setIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton));

    reveal->setToolTip("revealed?");
    nature->setToolTip("Type of relationship");
    qualifier->setToolTip("Qualifier");
    this_link->setToolTip("The column in THIS row containing the value to search for");
    other_link->setToolTip("The column to search in the table");
    remove->setToolTip("Remove this relationship");

    for (int i=0; i<nature_enum.keyCount(); i++)
        nature->addItem(nature_enum.key(i));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(reveal, 0);
    layout->addWidget(nature, 0);
    layout->addWidget(qualifier, 0);
    layout->addWidget(this_link,   1);
    layout->addWidget(other_link,  1);
    layout->addWidget(remove, 0);
    setLayout(layout);

    connect(remove, &QPushButton::clicked, this, &RWRelationshipWidget::deleteRequested);
    connect(reveal, &QRadioButton::toggled, this, [=](bool value) { p_relationship->is_revealed = value; });
    connect(nature, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), [=](const QString &value) { setNature(value); });
    connect(qualifier, QOverload<const QString&>::of(&QComboBox::activated), [=](const QString &value) { setQualifier(value); });

    // Transfer values from supplied relationship
    reveal->setChecked(relationship->is_revealed);
    QString qualifier = relationship->qualifier_tag_name; // before setNature might change it
    setNature(nature_enum.valueToKey(relationship->nature));
    setQualifier(qualifier);
    if (p_relationship->thisLink().modelColumn() >= 0)
        this_link->setText(column_name(columns, p_relationship->thisLink().modelColumn()));
    if (p_relationship->otherLink().modelColumn() >= 0)
        other_link->setText(column_name(columns, p_relationship->otherLink().modelColumn()));

}


void RWRelationshipWidget::setNature(const QString &reason)
{
    qDebug() << "RWRelationshipWidget::setNature =" << reason;
    bool ok;
    int value = nature_enum.keyToValue(qPrintable(reason), &ok);
    if (!ok) return;

    nature->setCurrentText(reason);
    qualifier->clear();

    RWDomain *domain;
    p_relationship->nature = RWRelationship::Nature(value);
    switch (p_relationship->nature)
    {
    case RWRelationship::Master_To_Minion:
    case RWRelationship::Minion_To_Master:
        // Requires tag from "Comprises Relationship Types" domain
        domain = RWDomain::getDomainByName("Comprises Relationship Types");
        if (domain == nullptr) return;
        qualifier->addItems(domain->tagNames());
        qualifier->show();
        break;

    case RWRelationship::Generic:
        // Requires tag from "Generic Relationship Types" domain
        domain = RWDomain::getDomainByName("Generic Relationship Types");
        if (domain == nullptr) return;
        qualifier->addItems(domain->tagNames());
        qualifier->show();
        break;

    case RWRelationship::Public_Attitude_Towards:
    case RWRelationship::Private_Attitude_Towards:
        for (int i=0; i<attitude_enum.keyCount(); i++)
            qualifier->addItem(attitude_enum.key(i));
        qualifier->show();
        break;

    case RWRelationship::Parent_To_Offspring:
    case RWRelationship::Offspring_To_Parent:
    case RWRelationship::Arbitrary:
    case RWRelationship::Union:
        qualifier->hide();
        // No additional attributes
        break;
    }
}


void RWRelationshipWidget::setQualifier(const QString &value)
{
    qDebug() << "RWRelationshipWidget::setQualifier =" << value;
    p_relationship->qualifier_tag_name = value;
    qualifier->setCurrentText(value);
}
