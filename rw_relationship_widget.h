#ifndef RW_RELATIONSHIP_WIDGET_H
#define RW_RELATIONSHIP_WIDGET_H

#include <QWidget>
#include "datafield.h"
#include "rw_relationship.h"

class RWRelationship;
class FieldLineEdit;
class QComboBox;
class QPushButton;
class QRadioButton;

class RWRelationshipWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RWRelationshipWidget(RWRelationship *relationship, QAbstractItemModel *columns, QWidget *parent = nullptr);
    //DataField &thisLink()  { return this_link; }
    //DataField &otherLink()  { return other_link; }
    RWRelationship *relationship() { return p_relationship; }

signals:
    void deleteRequested();

public slots:
    void setNature(const QString &reason);
    void setQualifier(const QString &qualifier);

private:
    RWRelationship *p_relationship;
    DataField p_linkedTopic;
    QRadioButton *reveal;
    QComboBox *nature;
    QComboBox *qualifier;
    FieldLineEdit *this_link;
    FieldLineEdit *other_link;
    QPushButton *remove;
};

#endif // RW_CONNECTION_WIDGET_H
