#ifndef RW_ALIAS_H
#define RW_ALIAS_H

#include <QObject>
#include "datafield.h"

class QXmlStreamWriter;
class QModelIndex;

class RWAlias : public QObject
{
    Q_OBJECT
public:
    enum CaseMatching  { Ignore, Sensitive, Correction };
    enum MatchPriority { Normal, Prefer, Avoid, Never };
    Q_ENUM(CaseMatching)
    Q_ENUM(MatchPriority)

    explicit RWAlias(QObject *parent = nullptr);

    bool isRevealed() const { return p_is_revealed; }
    bool isAutoAccept() const { return p_is_auto_accept; }
    bool isTrueName() const { return p_is_true_name; }
    bool isShowNavPane() const { return p_is_show_nav_pane; }
    CaseMatching caseMatching() const { return p_case_matching; }
    MatchPriority matchPriority() const { return p_match_priority; }

    virtual void writeToContents(QXmlStreamWriter*, const QModelIndex &index);
    DataField &namefield()   { return p_name_field; }

public slots:
    void setCaseMatchingInt(int v) { p_case_matching = static_cast<CaseMatching>(v); }
    void setMatchPriorityInt(int v) { p_match_priority = static_cast<MatchPriority>(v); }
    void setCaseMatching(CaseMatching v) { p_case_matching = v; }
    void setMatchPriority(MatchPriority v) { p_match_priority = v; }
    void setAutoAccept(bool v) { p_is_auto_accept = v; }
    void setRevealed(bool v) { p_is_revealed = v; }
    void setIsTrueName(bool v) { p_is_true_name = v; }
    void setIsRevealed(bool v) { p_is_revealed = v; }
    void setShowInNav(bool v) { p_is_show_nav_pane = v; }

private:
    bool p_is_auto_accept;
    CaseMatching p_case_matching;
    MatchPriority p_match_priority;
    bool p_is_show_nav_pane;
    bool p_is_true_name;
    bool p_is_revealed;
    DataField p_name_field;

};

#endif // RW_ALIAS_H
