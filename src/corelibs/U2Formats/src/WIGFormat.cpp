/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include "WIGFormat.h"

namespace U2 {

static const QString BROWSER_TAG("browser");
static const QString TRACK_TAG("track");
static const QString COMMENT_START("#");
static const QString VARIABLE_STEP_TYPE("variableStep");
static const QString FIXED_STEP_TYPE("fixedStep");

WIGFormat::WIGFormat(QObject *p)
: DocumentFormat(p, DocumentFormatFlags_SW, QStringList()<<"wig")
{
    formatName = tr("WIG");
    supportedObjectTypes += GObjectTypes::ANNOTATION_TABLE;
    formatDescription = tr("WIG format is a text-based format for representing wiggle graphs for Genome Browser.");
}

FormatCheckResult WIGFormat::checkRawData(const QByteArray &rawData, const GUrl &) const {
    return FormatDetection_Matched;
}

Document * WIGFormat::loadDocument(IOAdapter *io, const U2DbiRef &dbiRef, const QVariantMap &fs, U2OpStatus &os) {
    LineReader lineReader(io);

    // skip browser tags
    do {
        lineReader.readNextLine();
    } while(lineReader.getCurrentLine().startsWith(BROWSER_TAG));

    QList<WiggleData> dataList;
    do {
        WiggleData data = readWiggleData(lineReader, os);
        CHECK_OP(os, NULL);
        dataList << data;
    } while(!lineReader.getCurrentLine().isEmpty());

    foreach (const WiggleData &data, dataList) {
        // create something for each data
    }
    // create document

    return NULL;
}

WiggleData WIGFormat::readWiggleData(LineReader &lineReader, U2OpStatus &os) {
    // read comments
    QStringList comments;
    if (lineReader.getCurrentLine().startsWith(COMMENT_START)) {
        do {
            comments << lineReader.getCurrentLine().mid(COMMENT_START.size());
            lineReader.readNextLine();
        } while(lineReader.getCurrentLine().startsWith(COMMENT_START));
    }

    // read track parameters
    QString parameters;
    static const QString trackBegin = TRACK_TAG + " ";
    if (lineReader.getCurrentLine().startsWith(trackBegin)) {
        QString paramsStr = lineReader.getCurrentLine().mid(trackBegin.size());

        parameters = paramsStr.simplified();
        lineReader.readNextLine();
    }

    // create wiggle reader
    QScopedPointer<WiggleReader> reader(WiggleReader::createReader(lineReader.getCurrentLine(), os));
    CHECK_OP(os, WiggleData());
    lineReader.readNextLine();

    // read wiggles
    QList<WiggleData::Wiggle> wiggles = reader->read(lineReader, os);
    CHECK_OP(os, WiggleData());

    return WiggleData(comments, parameters, wiggles);
}

void WIGFormat::storeDocument(Document *d, IOAdapter *io, U2OpStatus &os) {

}

void WIGFormat::storeEntry(IOAdapter *io, const QMap< GObjectType, QList<GObject*> > &objectsMap, U2OpStatus &os) {

}

/************************************************************************/
/* WiggleData */
/************************************************************************/
WiggleData::WiggleData() {

}

WiggleData::WiggleData(const QStringList &_comments, const QString &_parameters, const QList<Wiggle> &_wiggles)
: comments(_comments), parameters(_parameters), wiggles(_wiggles)
{

}

const QStringList &WiggleData::getComments() const {
    return comments;
}

const QString &WiggleData::getParameters() const {
    return parameters;
}
const QList<WiggleData::Wiggle> &WiggleData::getWiggles() const {
    return wiggles;
}

WiggleData::Wiggle::Wiggle(qint64 start, qint64 length, double _value)
: region(start, length), value(_value)
{

}

U2Region WiggleData::Wiggle::getRegion() const {
    return region;
}

double WiggleData::Wiggle::getValue() const {
    return value;
}

/************************************************************************/
/* LineReader */
/************************************************************************/
LineReader::LineReader(IOAdapter *_io)
: io(_io), buffer(BUFFER_SIZE, 0)
{

}

QString LineReader::readNextLine() {
    char *bufferData = buffer.data();

    do {
        int lineLength = io->readLine(bufferData, buffer.size());
        if (0 == lineLength && io->isEof()) {
            currentLine.clear();
            break;
        }
        currentLine = buffer.left(lineLength).simplified();
    } while(currentLine.isEmpty());

    return currentLine;
}

QString LineReader::getCurrentLine() const {
    return currentLine;
}

/************************************************************************/
/* WiggleReader */
/************************************************************************/
const QString WiggleReader::CHROM_TAG("chrom");
const QString WiggleReader::SPAN_TAG("span");

WiggleReader::WiggleReader(const QString &_chromName, int _spanSize)
: chromName(_chromName), spanSize(_spanSize)
{

}

bool WiggleReader::isNextTrackStart(const QString &line) {
    if (line.startsWith(TRACK_TAG)|| line.startsWith(COMMENT_START)
        || line.startsWith(VARIABLE_STEP_TYPE) || line.startsWith(FIXED_STEP_TYPE)) {
            return true;
    }
    return false;
}

class VariableStepReader : public WiggleReader {
public:
    VariableStepReader(const QString &chromName, int spanSize, const QStringList &params, U2OpStatus &os)
    : WiggleReader(chromName, spanSize)
    {
        if (params.size() > 0) {
            os.setError(WIGFormat::tr("Unknown parameters: %1, ...").arg(params[0]));
        }
    }

    virtual QList<WiggleData::Wiggle> read(LineReader &lineReader, U2OpStatus &os) {
        QList<WiggleData::Wiggle> result;
        qint64 prevPos = -1;
        do {
            if (isNextTrackStart(lineReader.getCurrentLine())) {
                return result;
            }
            // parse string to pos and value
            QStringList values = lineReader.getCurrentLine().split(QRegExp("\\s"));
            if (2 != values.size()) {
                os.setError(WIGFormat::tr("Wrong wiggle value string: %1").arg(lineReader.getCurrentLine()));
                return result;
            }
            bool ok = false;
            qint64 pos = values[0].toLongLong(&ok) - 1;
            if (!ok) {
                os.setError(WIGFormat::tr("Wrong wiggle position '%1' in the string: %2").arg(values[0]).arg(lineReader.getCurrentLine()));
                return result;
            }
            double value = values[1].toDouble(&ok);
            if (!ok) {
                os.setError(WIGFormat::tr("Wrong wiggle value '%1' in the string: %2").arg(values[1]).arg(lineReader.getCurrentLine()));
                return result;
            }
            // add result
            result << WiggleData::Wiggle(pos, spanSize, value);
            if (pos <= prevPos) {
                coreLog.error(WIGFormat::tr("Unordered wiggle positions"));
            }
            prevPos = pos + spanSize - 1;
            lineReader.readNextLine();
        } while (!lineReader.getCurrentLine().isEmpty());
        return result;
    }

private:
};

class FixedStepReader : public WiggleReader {
public:
    FixedStepReader(const QString &chromName, int spanSize, const QStringList &_params, U2OpStatus &os)
    : WiggleReader(chromName, spanSize)
    {
        QStringList params = _params;
        startPos = takeIntParameter(params, START_TAG, true, os);
        CHECK_OP(os, );
        stepSize = takeIntParameter(params, STEP_TAG, false, os);
        CHECK_OP(os, );

        if (params.size() > 0) {
            os.setError(WIGFormat::tr("Unknown parameters: %1, ...").arg(params[0]));
        }
    }

    virtual QList<WiggleData::Wiggle> read(LineReader &lineReader, U2OpStatus &os) {
        QList<WiggleData::Wiggle> result;
        qint64 pos = startPos - 1;
        do {
            if (isNextTrackStart(lineReader.getCurrentLine())) {
                return result;
            }
            // parse string value
            bool ok = false;
            double value = lineReader.getCurrentLine().toDouble(&ok);
            if (!ok) {
                os.setError(WIGFormat::tr("Wrong wiggle value: %1").arg(lineReader.getCurrentLine()));
                return result;
            }
            // add result
            result << WiggleData::Wiggle(pos, spanSize, value);
            pos += stepSize;
            lineReader.readNextLine();
        } while (!lineReader.getCurrentLine().isEmpty());
        return result;
    }

private:
    static const QString START_TAG;
    static const QString STEP_TAG;

    int stepSize;
    int startPos;
};
const QString FixedStepReader::START_TAG("start");
const QString FixedStepReader::STEP_TAG("step");

WiggleReader * WiggleReader::createReader(const QString &stepString, U2OpStatus &os) {
    QStringList words = stepString.split(QRegExp("\\s"), QString::SkipEmptyParts);
    if (words.size() < 2) { // "...Step" and "chrom=..."
        os.setError(WIGFormat::tr("Wrong step type string: %1").arg(stepString));
        return NULL;
    }
    QString stepType = words.takeFirst();
    if (VARIABLE_STEP_TYPE != stepType && FIXED_STEP_TYPE != stepType) {
        os.setError(WIGFormat::tr("Unknown step type: %1").arg(stepType));
        return NULL;
    }

    QString chrom = takeStringParameter(words, CHROM_TAG, true, os);
    CHECK_OP(os, NULL);
    int spanSize = takeIntParameter(words, SPAN_TAG, false, os);
    CHECK_OP(os, NULL);

    WiggleReader *result = NULL;
    if (VARIABLE_STEP_TYPE == stepType) {
        result = new VariableStepReader(chrom, spanSize, words, os);
    } else if(FIXED_STEP_TYPE == stepType) {
        result = new FixedStepReader(chrom, spanSize, words, os);
    }

    if (os.hasError()) {
        delete result;
        return NULL;
    }
    return result;
}

int WiggleReader::takeIntParameter(QStringList &paramList, const QString &tagName, bool required, U2OpStatus &os) {
    QString valueStr = takeStringParameter(paramList, tagName, required, os);
    CHECK_OP(os, 0);

    if (valueStr.isEmpty()) {
        return 1;
    }

    bool ok = false;
    int result = valueStr.toInt(&ok);
    if (!ok) {
        os.setError(WIGFormat::tr("Wrong %1 value: %2").arg(tagName).arg(valueStr));
    }
    return result;
}

QString WiggleReader::takeStringParameter(QStringList &paramList, const QString &tagName, bool required, U2OpStatus &os) {
    QString result;

    QString tagBegin = tagName + "=";
    for (QStringList::Iterator iter = paramList.begin(); iter != paramList.end(); ) {
        QString tag = *iter;
        if (tag.startsWith(tagBegin)) {
            QString value = tag.mid(tagBegin.size());
            if (!result.isEmpty()) {
                os.setError(WIGFormat::tr("Double definition of %1 tag: '%2' and '%3'").arg(tagName).arg(result).arg(value));
                return "";
            }
            result = value;
            iter = paramList.erase(iter);
        } else {
            iter++;
        }
    }

    if (required && result.isEmpty()) {
        os.setError(WIGFormat::tr("No %1 tag").arg(tagName));
    }
    return result;
}

/************************************************************************/
/* WIGFormatParameters */
/************************************************************************/
bool WIGFormatParameters::isCorrectParameter(const QString &paramName) {
    static QStringList parameterList = QStringList()
        << name
        << description
        << visibility
        << color
        << altColor
        << priority
        << autoScale
        << alwaysZero
        << gridDefault
        << maxHeightPixels
        << graphType
        << viewLimits
        << viewLimitsMax
        << yLineMark
        << yLineOnOff
        << windowingFunction
        << smoothingWindow
        << transformFunc
        << wigColorBy
        << spanList;
    return parameterList.contains(paramName);
}

const QString WIGFormatParameters::name("name");
const QString WIGFormatParameters::description("description");
const QString WIGFormatParameters::visibility("visibility");
const QString WIGFormatParameters::color("color");
const QString WIGFormatParameters::altColor("altColor");
const QString WIGFormatParameters::priority("priority");
const QString WIGFormatParameters::autoScale("autoScale");
const QString WIGFormatParameters::alwaysZero("alwaysZero");
const QString WIGFormatParameters::gridDefault("gridDefault");
const QString WIGFormatParameters::maxHeightPixels("maxHeightPixels");
const QString WIGFormatParameters::graphType("graphType");
const QString WIGFormatParameters::viewLimits("viewLimits");
const QString WIGFormatParameters::viewLimitsMax("viewLimitsMax");
const QString WIGFormatParameters::yLineMark("yLineMark");
const QString WIGFormatParameters::yLineOnOff("yLineOnOff");
const QString WIGFormatParameters::windowingFunction("windowingFunction");
const QString WIGFormatParameters::smoothingWindow("smoothingWindow");
const QString WIGFormatParameters::transformFunc("transformFunc");
const QString WIGFormatParameters::wigColorBy("wigColorBy");
const QString WIGFormatParameters::spanList("spanList");

} // U2
