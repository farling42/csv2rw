#ifndef FIELDCOMBOBOX_H
#define FIELDCOMBOBOX_H

#include <QComboBox>

class FieldComboBox : public QComboBox
{
    Q_OBJECT
public:
    FieldComboBox(QWidget *parent = 0);
signals:
    void modelColumnSelected(int row);

public Q_SLOTS:
    void setValue(const QString&);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
};


#endif // FIELDCOMBOBOX_H
