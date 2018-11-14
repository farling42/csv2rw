#ifndef RW_RELATIONSHIP_H
#define RW_RELATIONSHIP_H

#include <QObject>
#include "datafield.h"

class QXmlStreamWriter;

class RWRelationship : public QObject
{
    Q_OBJECT
public:
    explicit RWRelationship(QObject *parent = nullptr);

    enum Nature { Arbitrary, Generic, Union, Parent_To_Offspring, Offspring_To_Parent, Master_To_Minion, Minion_To_Master, Public_Attitude_Towards, Private_Attitude_Towards };
    enum Attitude { Hostile, Angry, Annoyed, Neutral, Pleased, Friendly, Gracious };
    Q_ENUM(Nature)
    Q_ENUM(Attitude)

    DataField &thisLink()   { return p_this_link; }
    DataField &otherLink()  { return p_other_link; }
    bool is_revealed{false};
    Nature   nature{Arbitrary};
    Attitude attitude;
    QString qualifier_tag_name;

    void writeToContents(QXmlStreamWriter*, const QModelIndex &index) const;

signals:

public slots:

private:
    DataField p_this_link;   // The column in THIS row containing the value to match
    DataField p_other_link;  // The column in THE MODEL in which the match should be found
    friend QDataStream& operator<<(QDataStream&, const RWRelationship&);
    friend QDataStream& operator>>(QDataStream&, RWRelationship&);
};

extern QDataStream& operator<<(QDataStream&, const RWRelationship&);
extern QDataStream& operator>>(QDataStream&, RWRelationship&);

#endif // RW_CONNECTION_H
