#ifndef TOPICKEY_H
#define TOPICKEY_H

#include <QDialog>

namespace Ui {
class TopicKey;
}

class QAbstractItemModel;

class TopicKey : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int selectedColumn READ selectedColumn WRITE setSelectedColumn)
    Q_PROPERTY(QString selectedValue READ selectedValue WRITE setSelectedValue)

public:
    explicit TopicKey(QWidget *parent = nullptr);
    ~TopicKey();

public:
    static void setModel(QAbstractItemModel *model);
    int selectedColumn() const;
    QString selectedValue() const;

public slots:
    void setSelectedColumn(int);
    void setSelectedValue(const QString &value);

private slots:
    void on_csvColumn_currentIndexChanged(int column);

private:
    Ui::TopicKey *ui;
};

#endif // TOPICKEY_H
