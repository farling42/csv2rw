#ifndef FIELDLINEEDIT_H
#define FIELDLINEEDIT_H

#include <QLineEdit>

class FieldLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    FieldLineEdit(QWidget *parent = 0);

signals:
    void modelColumnSelected(int row);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
};

#endif // FIELDLINEEDIT_H
