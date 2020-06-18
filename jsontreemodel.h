#ifndef JSONTREEMODEL_H
#define JSONTREEMODEL_H

#include <QStandardItemModel>
#include <QFile>

class JsonTreeModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit JsonTreeModel(QObject *parent = nullptr);
    bool readFile(QFile &file);
private:
    typedef QStandardItemModel SuperClass;
};

#endif // JSONTREEMODEL_H
