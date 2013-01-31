/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_WIG_FORMAT_
#define _U2_WIG_FORMAT_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2Region.h>

namespace U2 {

class IOAdapter;
class LineReader;
class WiggleData;

/**
 * The specification of this format is described here:
 * http://genome.ucsc.edu/goldenPath/help/wiggle.html
 */
class U2FORMATS_EXPORT WIGFormat : public DocumentFormat {
    Q_OBJECT
public:
    WIGFormat(QObject *p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::WIG;}

    virtual const QString & getFormatName() const {return formatName;}

    virtual void storeDocument(Document *d, IOAdapter *io, U2OpStatus &os);

    virtual FormatCheckResult checkRawData(const QByteArray &rawData, const GUrl & = GUrl()) const;

    virtual bool isStreamingSupport() {return true;}

    virtual void storeEntry(IOAdapter *io, const QMap< GObjectType, QList<GObject*> > &objectsMap, U2OpStatus &os);

protected:
    virtual Document * loadDocument(IOAdapter *io, const U2DbiRef &dbiRef, const QVariantMap &fs, U2OpStatus &os);

private:
    QString formatName;

    WiggleData readWiggleData(LineReader &lineReader, U2OpStatus &os);
};

class WIGFormatParameters {
public:
    static const QString name;              //<trackLabel       # default is "User Track"
    static const QString description;       //<centerLabel>     # default is "User Supplied Track"
    static const QString visibility;        //<full|dense|hide> # default is hide (will also take numeric values 2|1|0)
    static const QString color;             //<RRR,GGG,BBB>     # default is 255,255,255
    static const QString altColor;          //<RRR,GGG,BBB>     # default is 128,128,128
    static const QString priority;          //<N>               # default is 100
    static const QString autoScale;         //<on|off>          # default is on
    static const QString alwaysZero;        //<on|off>          # default is off
    static const QString gridDefault;       //<on|off>          # default is off
    static const QString maxHeightPixels;   //<max:default:min> # default is 128:128:11
    static const QString graphType;         //<bar|points>      # default is bar
    static const QString viewLimits;        //<lower:upper>     # default is range found in data
    static const QString viewLimitsMax;     //<lower:upper>     # UNENFORCED: suggested bounds of viewLimits
    static const QString yLineMark;         //<real-value>      # default is 0.0
    static const QString yLineOnOff;        //<on|off>          # default is off
    static const QString windowingFunction; //<mean+whiskers|maximum|mean|minimum> # default is maximum
    static const QString smoothingWindow;   //<off|[2-16]>      # default is off
    static const QString transformFunc;     //<NONE|LOG>        # default is NONE
    static const QString wigColorBy;        //<bed table>       # use colors in bed table for wiggle in overlapping regions
    static const QString spanList;          //<first>           # sets spans to be first span in table
                                            /*<s1,s2,s3...>     # list of spans in the loaded table
                                                                # you can find the spans with the following SQL query:
                                                                # "SELECT span FROM <table> GROUP BY span"
                                                                # typically spanList is only 1:
                                                                # spanList 1
                                                                # rarely, it may be more:
                                                                # spanList 1,1000
                                                                # special efforts must be made to load extra spans
                                                                # into the table for special purposes */
    static bool isCorrectParameter(const QString &paramName);
};

class WiggleData {
public:
    class Wiggle {
    public:
        Wiggle(qint64 start, qint64 length, double value);
        U2Region getRegion() const;
        double getValue() const;
    private:
        U2Region region;
        double value;
    };

    WiggleData();
    WiggleData(const QStringList &comments, const QString &parameters, const QList<Wiggle> &wiggles);

    const QStringList &getComments() const;
    const QString &getParameters() const;
    const QList<Wiggle> &getWiggles() const;

private:
    QStringList comments;
    QString parameters;
    QList<Wiggle> wiggles;
};

class LineReader {
public:
    LineReader(IOAdapter *io);
    /**
     * Skips empty lines and return next not empty one.
     * If io is ended then returns empty string.
     */
    QString readNextLine();
    QString getCurrentLine() const;

private:
    IOAdapter *io;
    QString currentLine;
    QByteArray buffer;

    static const int BUFFER_SIZE = 100*1024;
};

class WiggleReader {
public:
    static WiggleReader *createReader(const QString &stepString, U2OpStatus &os);

    virtual QList<WiggleData::Wiggle> read(LineReader &lineReader, U2OpStatus &os) = 0;

protected:
    WiggleReader(const QString &chromName, int spanSize);

    static int takeIntParameter(QStringList &paramList, const QString &tagName, bool required, U2OpStatus &os);
    static QString takeStringParameter(QStringList &paramList, const QString &tagName, bool required, U2OpStatus &os);

protected:
    QString chromName;
    int spanSize;

    static bool isNextTrackStart(const QString &line);

private:
    static const QString CHROM_TAG;
    static const QString SPAN_TAG;
};

} // U2

#endif // _U2_WIG_FORMAT_
