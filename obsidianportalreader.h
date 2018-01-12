#ifndef OBSIDIANPORTALREADER_H
#define OBSIDIANPORTALREADER_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;

class ObsidianPortalReader : public QObject
{
    Q_OBJECT
    void read_blog(QXmlStreamReader &xml, QXmlStreamWriter &out);
    void read_wiki(QXmlStreamReader xml);
    void read_characters(QXmlStreamReader xml);
    void read_items(QXmlStreamReader xml);
    void read_forum(QXmlStreamReader xml);
    void load_campaign(QXmlStreamReader xml);
public:
    explicit ObsidianPortalReader(QObject *parent = nullptr);

    convertFile(const QString &infile_name, const QString &outfile_name);
signals:

public slots:
};

#endif // OBSIDIANPORTALREADER_H
