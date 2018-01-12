#include "obsidianportalreader.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/*
 * The format of an Obsidian Portal XML backup file is:
 *
 * <campaign>
 *   <id>string</id>
 *   <link>string</link>
 *   <title>string</title>
 *   <blog>
 *     <entry gm_only="false>
 *
 *     </entry>
 *   </blog>
 *   <wiki>
 *   </wiki>
 *   <characters>
 *   </characters>
 *   <items>
 *   </items>
 *   <forum>
 *   </forum>
 * </campaign>
 *
 * Every element has the following children in this order:
 *   <id>string</id>
 *   <link>string</link>
 *   <title>string</title>
 *
 */

ObsidianPortalReader::ObsidianPortalReader(QObject *parent) : QObject(parent)
{

}


ObsidianPortalReader::convertFile(const QString &infile_name, const QString &outfile_name)
{
    QFile infile(infile_name);
    QFile outfile(outfile_name);

    if (!infile.exists())
    {
        warning("Input file does not exist!");
        return;
    }

    QXmlStreamReader xml;
    xml.setDevice(infile);

    //
    // <campaign>
    //    <bol;g
    // </campaign>
    while (!xml.atEnd())
    {
        xml.readNextStartElement();

        const QStringRef element = xml.name();
    }
}

/*
<xsl:template match="campaign/blog/entry">
    <!-- Convert OB blog entry into the appropriate RW topic -->
    <topic category_id="cat_blog">
        <xsl:attribute name="topic_id">blog-<xsl:number value="position()"/></xsl:attribute>
        <xsl:attribute name="public_name"><xsl:value-of select="title"/></xsl:attribute>
        <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
        <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->

        <xsl:variable name="revealed"><xsl:value-of select="published"/></xsl:variable>
        <section partition_id="p_blog_public">
            <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
            <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
            <xsl:for-each select="content[@format='html']">
                <snippet type="Multi_Line">
                    <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                    <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                    <!-- <xsl:attribute name="is_revealed">true</xsl:attribute><xsl:attribute name="reveal_date"><xsl:copy-of select="$revealed"/></xsl:attribute> -->
                    <contents>
                        <xsl:call-template name="content"/>
                    </contents>
                </snippet>
            </xsl:for-each>
        </section>
        <tag_assign tag_id="Tag_1" />
    </topic>
</xsl:template>
*/

void ObsidianPortalReader::read_blog(QXmlStreamReader &xml, QXmlStreamWriter &out)
{
    while (!xml.atEnd())
    {
        xml.readNextStartElement();
        if (xml.name() == "entry")
        {
            bool first = true;
            out.writeStartElement("topic");
            out.writeAttribute("category_id", "cat_blog");
            out.writeAttribute("topic_id", QString("blog-%1").arg(global_id++));
            out.writeAttribute("is_revealed", xml.attributes.value("gm_only") == "false");

            // check for attribute "gm_only"
            while (!xml.atEnd())
            {
                xml.readNextStartElement();
                if (xml.name() == "title")
                    out.writeAttribute("public_name", xml.text());
                else if (xml.name() == "published")
                    out.writeAttribute("revealed", xml.text());
                else if (xml.name() == "content")
                {
                    if (first)
                    {
                        out.writeStartElement("section");
                        out.writeAttribute("partition_id", "p_blog_public");
                        first = false;
                    }
                    out.writeStartElement("snippet");
                    out.writeAttribute("type", "Multi_Line");

                    out.writeAttribute("is_revealed", "!gm_only");
                    out.writeEndElement();
                }
            }
            if (!first)
            {
                out.writeEndElement(); // add "</section>"
            }
            out.writeEndElement();
        }
        xml.skipCurrentElement();
    }
}

/*
<xsl:template match="campaign/wiki/page">
    <!-- Convert OB wiki page into the appropriate RW topic -->
    <topic category_id="cat_wiki">
        <xsl:attribute name="topic_id">wikipage-<xsl:number value="position()"/></xsl:attribute>
        <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
        <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
        <xsl:attribute name="public_name">
            <xsl:value-of select="title"/>
        </xsl:attribute>

        <xsl:variable name="revealed"><xsl:value-of select="published"/></xsl:variable>
        <section partition_id="p_wiki_public">
            <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
            <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
            <xsl:for-each select="content[@format='html']">
                <snippet type="Multi_Line">
                    <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                    <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                    <!-- <xsl:attribute name="is_revealed">true</xsl:attribute><xsl:attribute name="reveal_date"><xsl:copy-of select="$revealed"/></xsl:attribute> -->
                    <contents>
                        <xsl:call-template name="content"/>
                    </contents>
                </snippet>
            </xsl:for-each>
        </section>
        <tag_assign tag_id="Tag_1" />
    </topic>
</xsl:template>
*/

void ObsidianPortalReader::read_wiki(QXmlStreamReader xml)
{

}

/*
<xsl:template match="campaign/characters/character">
    <!-- Convert OB character into the appropriate RW topic -->
    <topic category_id="cat_person">
        <xsl:attribute name="topic_id">character-<xsl:number value="position()"/></xsl:attribute>
        <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
        <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
        <xsl:attribute name="public_name">
            <xsl:value-of select="title"/>
        </xsl:attribute>

        <!-- no "published" element -->
        <xsl:variable name="revealed"><xsl:value-of select="published"/></xsl:variable>
        <section partition_id="p_person_public">
            <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
            <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
            <xsl:for-each select="content[@format='html']">
                <snippet type="Multi_Line">
                    <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                    <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                    <!-- <xsl:attribute name="is_revealed">true</xsl:attribute><xsl:attribute name="reveal_date"><xsl:copy-of select="$revealed"/></xsl:attribute> -->
                    <contents>
                        <xsl:call-template name="content"/>
                    </contents>
                </snippet>
            </xsl:for-each>
        </section>
        <tag_assign tag_id="Tag_1"/>
    </topic>
</xsl:template>
*/
void ObsidianPortalReader::read_characters(QXmlStreamReader xml)
{

}

/*
<xsl:template match="campaign/items/item">
    <!-- Convert OB item into the appropriate RW topic -->
    <topic category_id="cat_item">
        <xsl:attribute name="topic_id">item-<xsl:number value="position()"/></xsl:attribute>
        <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
        <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
        <xsl:attribute name="public_name">
            <xsl:value-of select="title"/>
        </xsl:attribute>
        <!-- no "published" element -->
        <xsl:variable name="revealed"><xsl:value-of select="published"/></xsl:variable>
        <section partition_id="p_item_public">
            <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
            <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
            <snippet type="Multi_Line">
                <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                <contents>
                    <xsl:value-of select="category"/>
                </contents>
            </snippet>
        </section>

        <section partition_id="p_item_public">
            <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
            <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
            <!-- <xsl:attribute name="reveal_date"><xsl:copy-of select="$revealed"/></xsl:attribute> -->
            <xsl:for-each select="content[@format='html']">
                <snippet type="Multi_Line">
                    <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                    <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                    <!-- <xsl:attribute name="is_revealed">true</xsl:attribute><xsl:attribute name="reveal_date"><xsl:copy-of select="$revealed"/></xsl:attribute> -->
                    <contents>
                        <xsl:call-template name="content"/>
                    </contents>
                </snippet>
            </xsl:for-each>
        </section>
        <tag_assign tag_id="Tag_1" />
    </topic>
</xsl:template>

 */
void ObsidianPortalReader::read_items(QXmlStreamReader xml)
{

}

/*
<xsl:template match="campaign/forum/topic">
    <!-- Convert OB forum thread into the appropriate RW topic -->
    <topic category_id="cat_forum">
        <xsl:attribute name="topic_id">forum-<xsl:number value="position()"/></xsl:attribute>
        <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
        <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
        <xsl:attribute name="public_name">
            <xsl:value-of select="title"/>
        </xsl:attribute>

        <xsl:for-each select="post">
            <section partition_id="p_forum_public">
                <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                <xsl:variable name="revealed"><xsl:value-of select="published"/></xsl:variable>
                <snippet type="Multi_Line">
                    <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                    <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                    <!-- <xsl:attribute name="is_revealed">true</xsl:attribute><xsl:attribute name="reveal_date"><xsl:copy-of select="$revealed"/></xsl:attribute> -->
                    <contents>
                        <xsl:copy-of select="$textstart"/>
                        Posted at <xsl:value-of select="published"/> by <xsl:value-of select="author"/>
                        <xsl:copy-of select="$textfinish"/>
                    </contents>
                </snippet>
                <snippet type="Multi_Line">
                    <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                    <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                    <!-- <xsl:attribute name="is_revealed">true</xsl:attribute><xsl:attribute name="reveal_date"><xsl:copy-of select="$revealed"/></xsl:attribute> -->
                    <contents>
                        <xsl:copy-of select="$textstart"/>
                        <xsl:value-of select="author"/>
                        <xsl:copy-of select="$textfinish"/>
                    </contents>
                </snippet>
                <xsl:for-each select="content[@format='html']">
                    <snippet type="Multi_Line">
                        <!-- <xsl:attribute name="original_uuid"><xsl:call-template name="uuid"/></xsl:attribute>
                        <xsl:attribute name="signature"><xsl:call-template name="signature"/></xsl:attribute> -->
                        <!-- <xsl:attribute name="is_revealed">true</xsl:attribute><xsl:attribute name="reveal_date"><xsl:copy-of select="$revealed"/></xsl:attribute> -->
                        <contents>
                            <xsl:call-template name="content"/>
                        </contents>
                    </snippet>
                </xsl:for-each>
            </section>
        </xsl:for-each>
        <tag_assign tag_id="Tag_1"/>
    </topic>
</xsl:template>
 */

void ObsidianPortalReader::read_forum(QXmlStreamReader xml)
{

}

/*
<xsl:template match="/">
<!-- This XSLT file can be invoked with a command similar to:
    "c:\Program Files\Saxonica\SaxonHE9.7N\bin\Transform.exe" -s:skulls-and-shackles-druid-style_2017-01-16.xml -xsl:ob2rw.xslt -o:obsidian.rwexport

    where:
    -s:{obsidian-file}.xml is the name of your obsidian portal backup file
    -o:{realm-file}.rwexport is the name of the rwexport file which will be created by this XSLT.
-->

    <export format_version="2" game_system_id="3" is_structure_only="false"
        xmlns="urn:lonewolfdevel.com:realm-works-export"
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
        <!-- <xsl:attribute name="export_date"><xsl:value-of select="current-dateTime()"/></xsl:attribute> -->
        <!-- <xsl:attribute name="owner_uuid">02000000-6350-D70E-0606-000000000000</xsl:attribute> -->
        <!-- <xsl:attribute name="owner_uuid">01<xsl:call-template name="short-uuid"/></xsl:attribute> -->
        <!-- <xsl:attribute name="source_scope_uuid">02<xsl:call-template name="short-uuid"/></xsl:attribute> -->
        <xsl:attribute name="xsi:noNamespaceSchemaLocation">RealmWorksExport v2.xsd</xsl:attribute>
        <definition>
            <details>
                <xsl:attribute name="original_uuid">B8409DB6-FE6C-A317-A7EF-6B935B3A6BA9</xsl:attribute>
                <xsl:attribute name="name"><xsl:value-of select="campaign/title"/></xsl:attribute>
                <xsl:attribute name="import_tag_id">Tag_1</xsl:attribute>
                <!-- original_uuid and import_tag_id are mandatory -->
                <summary>Automatic conversion from Obsidian Portal XML backup</summary>
            </details>
            <content_summary>
                <xsl:attribute name="max_domain_count">1</xsl:attribute>
                <xsl:attribute name="max_category_count">5</xsl:attribute>
                <xsl:variable name="set" select="/campaign/blog/entry | /campaign/wiki/page | /campaign/characters/character | /campaign/items/item | /campaign/forum/topic"/>
                <xsl:attribute name="topic_count"><xsl:value-of select="count($set)"/></xsl:attribute>
                <xsl:attribute name="plot_count">0</xsl:attribute>
            </content_summary>
        </definition>
        <structure>
            <domain_global domain_id="Domain_1" name="Import">
                <!-- this "Import" domain is required in order to establish the "Tag_1" required for import_tag_id of details -->
                <xsl:attribute name="global_uuid">0008287B-91C6-5D3A-00D0-6A935C3C6BA9</xsl:attribute>
                <tag tag_id="Tag_1" match_priority="Normal" signature="622704">
                    <xsl:attribute name="original_uuid">12345678-FE6C-A317-A7EF-6B935B3A6BA9</xsl:attribute>
                    <xsl:attribute name="name"><xsl:value-of select="campaign/title"/></xsl:attribute>
                </tag>
            </domain_global>

            <!-- One structure for each type of item exported: Blog, Wikipage, Character, Item, Topic -->
            <category category_id="cat_blog" name="Obsidian Portal Adventure Log" abbrev="Obsidian P" role="Topic" family_id="7" asset_name="Open Book" asset_uuid="B600287B-91C6-5D3A-00D0-6A935C3C6BA9" original_uuid="5A8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="623677">
                <description />
                <summary />
                <partition partition_id="p_blog_over" name="Overview" original_uuid="5B8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622706">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_blog_public" name="Public" original_uuid="5C8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622707">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_blog_gm" name="GM Only" original_uuid="5D8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622708">
                    <description />
                    <purpose />
                </partition>
            </category>
            <category category_id="cat_person" name="Obsidian Portal Character" abbrev="Obsidian P" role="Topic" family_id="1" asset_name="Individual" asset_uuid="F7AD297B-91C6-5D3A-00D0-6A935D3C6BA9" original_uuid="5E8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="623678">
                <description />
                <summary />
                <partition partition_id="p_person_over" name="Overview" original_uuid="5F8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622710">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_person_public" name="Public" original_uuid="608092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622711">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_person_gm" name="GM Only" original_uuid="618092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622712">
                    <description />
                    <purpose />
                </partition>
            </category>
            <category category_id="cat_item" name="Obsidian Portal Item" abbrev="Obsidian P" role="Topic" family_id="4" asset_name="Named Object" asset_uuid="FAAD297B-91C6-5D3A-00D0-6A935D3C6BA9" original_uuid="648092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="623679">
                <description />
                <summary />
                <partition partition_id="p_item_over" name="Overview" original_uuid="658092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622716">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_item_public" name="Public" original_uuid="668092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622717">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_item_gm" name="GM only" original_uuid="678092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622718">
                    <description />
                    <purpose />
                </partition>
            </category>
            <category category_id="cat_forum" name="Obsidian Portal Forum Thread" abbrev="Obsidian P" role="Topic" family_id="7" asset_name="Further Information" asset_uuid="4701287B-91C6-5D3A-00D0-6A935C3C6BA9" original_uuid="688092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="623680">
                <description />
                <summary />
                <partition partition_id="p_forum_over" name="Overview" original_uuid="698092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622720">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_forum_public" name="Public" original_uuid="6A8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622721">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_forum_gm" name="GM Only" original_uuid="6B8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622722">
                    <description />
                    <purpose />
                </partition>
            </category>
            <category category_id="cat_wiki" name="Obsidian Portal Wiki" abbrev="Obsidian P" role="Topic" family_id="7" asset_name="Deity" asset_uuid="0DAE297B-91C6-5D3A-00D0-6A935D3C6BA9" original_uuid="6C8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="623681">
                <description />
                <summary />
                <partition partition_id="p_wiki_over" name="Overview" original_uuid="6D8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622724">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_wiki_public" name="Public" original_uuid="6E8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622725">
                    <description />
                    <purpose />
                </partition>
                <partition partition_id="p_wiki_gm" name="GM Only" original_uuid="6F8092B6-FE6C-A317-A7EF-6B935B3A6BA9" signature="622726">
                    <description />
                    <purpose />
                </partition>
            </category>
        </structure>
        <contents>
            <xsl:apply-templates select="campaign/blog/entry"/>
            <xsl:apply-templates select="campaign/wiki/page"/>
            <xsl:apply-templates select="campaign/characters/character"/>
            <xsl:apply-templates select="campaign/items/item"/>
            <xsl:apply-templates select="campaign/forum/topic"/>
        </contents>
    </export>
</xsl:template>
 */

void ObsidianPortalReader::load_campaign(QXmlStreamReader xml)
{
    read_standard_elements(xml);
    while (!xml.atEnd())
    {
        xml.readNextStartElement();
        const QStringRef element = xml.name();

        if (element == "blog")
            read_blog(xml);
        else if (element == "wiki")
            read_wiki(xml);
        else if (element = "characters")
            read_characters(xml);
        else if (element == "items")
            read_items(xml);
        else if (element == "forum")
            read_forum(xml);
    }
}
