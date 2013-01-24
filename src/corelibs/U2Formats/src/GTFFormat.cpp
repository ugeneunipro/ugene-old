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

#include "GTFFormat.h"
#include "DocumentFormatUtils.h"

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>

#include <memory>


namespace U2 {

//-------------------------------------------------------------------
//  GTFLineValidateFlags
//-------------------------------------------------------------------
GTFLineValidateFlags::GTFLineValidateFlags()
    : incorrectNumberOfFields(false),
      emptyField(false),
      incorrectFeatureField(false),
      incorrectCoordinates(false),
      incorrectScore(false),
      incorrectStrand(false),
      incorrectFrame(false),
      noGeneIdAttribute(false),
      noTranscriptIdAttribute(false),
      incorrectFormatOfAttributes(false)
{}


FormatDetectionScore GTFLineValidateFlags::getFormatDetectionScore()
{
    if (incorrectNumberOfFields || emptyField || incorrectCoordinates){
        return FormatDetection_NotMatched;
    }

    if (noGeneIdAttribute || noTranscriptIdAttribute) {
        return FormatDetection_VeryLowSimilarity;
    }

    if (incorrectScore || incorrectStrand || incorrectFrame) {
        return FormatDetection_LowSimilarity;
    }

    if (incorrectFeatureField || incorrectFormatOfAttributes) {
        return FormatDetection_HighSimilarity;
    }

    return FormatDetection_Matched;
}


//-------------------------------------------------------------------
//  GTFFormat
//-------------------------------------------------------------------
const QString GTFFormat::FORMAT_NAME = GTFFormat::tr("GTF");

const int GTFFormat::FIELDS_COUNT_IN_EACH_LINE = 9;

const QString GTFFormat::NO_VALUE_STR = ".";

const QString GTFFormat::CHROMOSOME = "chromosome";
const QString GTFFormat::SOURCE_QUALIFIER_NAME = "source";
const QString GTFFormat::SCORE_QUALIFIER_NAME = "score";
const QString GTFFormat::STRAND_QUALIFIER_NAME = "strand";
const QString GTFFormat::FRAME_QUALIFIER_NAME = "frame";
const QString GTFFormat::GENE_ID_QUALIFIER_NAME = "gene_id";
const QString GTFFormat::TRANSCRIPT_ID_QUALIFIER_NAME = "transcript_id";


GTFFormat::GTFFormat(QObject* parent)
    : DocumentFormat(parent, DocumentFormatFlag_SupportWriting, QStringList("gtf"))
{
    formatDescription = tr("The Gene transfer format (GTF) is a file format used to hold"
        " information about gene structure.");

    supportedObjectTypes += GObjectTypes::ANNOTATION_TABLE;

    GTF_FEATURE_FIELD_VALUES << "CDS" << "start_codon" << "stop_codon"
        << "5UTR" << "3UTR" << "inter" << "inter_CNS" << "intron_CNS"
        << "exon" << "transcript" << "missing_data";
}


Document* GTFFormat::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& hints, U2OpStatus& os)
{
    Q_UNUSED(dbiRef);
    Q_UNUSED(hints);

    CHECK_EXT(io != NULL && io->isOpen(), os.setError(L10N::badArgument("IO adapter")), NULL);
    QList<GObject*> objects;

    load(io, objects, os);
    CHECK_OP_EXT(os, qDeleteAll(objects), NULL);

    Document* doc = new Document(this, io->getFactory(), io->getURL(), dbiRef, objects);
    return doc;
}


#define READ_BUFF_SIZE 4096
int readGTFLine(QString &buffer, IOAdapter* io, gauto_array<char>& charbuff) {
    int len;
    buffer.clear();
    do {
        len = io->readLine(charbuff.data, READ_BUFF_SIZE -1);
        charbuff.data[len] = '\0';
        buffer.append(QString(charbuff.data));
    } while (len == READ_BUFF_SIZE - 1);
    return buffer.length();
}


QList<SharedAnnotationData> GTFFormat::getAnnotData(IOAdapter* io, U2OpStatus& os)
{
    std::auto_ptr<QObject> parent(new QObject());
    GTFFormat gtfFormat(parent.get());
    QString seqName;
    return gtfFormat.parseDocument(io, seqName, os);
}


QList<SharedAnnotationData> GTFFormat::parseDocument(IOAdapter* io, QString &sequenceName, U2OpStatus& os)
{
    QList<SharedAnnotationData> result;

    int length;
    gauto_array<char> buff = new char[READ_BUFF_SIZE];
    QString qstrbuf;
    QStringList fields;

    bool fileIsValid = true;
    int lineNumber = 1;
    while ((length = readGTFLine(qstrbuf, io, buff)) > 0) {

        // Parse and validate the line
        GTFLineValidateFlags validationStatus;
        GTFLineData gtfLineData = parseAndValidateLine(qstrbuf, validationStatus);

        // Check that an annotation can be created
        if (validationStatus.isIncorrectNumberOfFields()) {
            os.setError(tr("GTF parsing error: incorrect number of fields at line %1!").arg(lineNumber));
            return result;
        }

        if (validationStatus.isEmptyField()) {
            os.setError(tr("GTF parsing error: a field at line %1 is empty!").arg(lineNumber));
            return result;
        }

        if (validationStatus.isIncorrectCoordinates()) {
            os.setError(tr("GTF parsing error: incorrect coordinates at line %1!").arg(lineNumber));
            return result;
        }

        // If file is invalid, but can be parsed an error is written to the log,
        // all details are written to the trace log.
        if (validationStatus.isFileInvalid()) {
            fileIsValid = false;
        }

        // Verify the sequence name (error doesn't occur, just warning)
        if (!sequenceName.isEmpty()) {
            if (gtfLineData.seqName != sequenceName) {
                ioLog.trace(tr("GTF parsing warning: different sequence names were detected"
                    " in an input GTF file. Sequence name '%1' is used.").arg(sequenceName));
            }
        }
        else {
            sequenceName = gtfLineData.seqName;
        }

        // Verify the feature field
        if (validationStatus.isIncorrectFeatureField()) {
            ioLog.trace(tr("GTF parsing error: unexpected value of the \"feature\""
                " value \"%1\" at line %2!").arg(gtfLineData.feature).arg(lineNumber));
        }

        // Create the annotation
        QString annotName = gtfLineData.feature;

        SharedAnnotationData annotData(new AnnotationData());
        annotData->name = annotName;
        annotData->location->regions << gtfLineData.region;


        // Add qualifiers
        annotData->qualifiers << U2Qualifier(CHROMOSOME, gtfLineData.seqName);

        if (NO_VALUE_STR != gtfLineData.source) {
            annotData->qualifiers.push_back(U2Qualifier(SOURCE_QUALIFIER_NAME, gtfLineData.source));
        }

        if (validationStatus.isIncorrectScore()) {
            // Write the error to the log, but open the file
            ioLog.trace(tr("GTF parsing error: incorrect score"
                " value \"%1\" at line %2!").arg(gtfLineData.score).arg(lineNumber));
        }
        else if (NO_VALUE_STR != gtfLineData.score) {
            annotData->qualifiers.push_back(U2Qualifier(SCORE_QUALIFIER_NAME, gtfLineData.score));
        }


        if (validationStatus.isIncorrectFrame()) {
            ioLog.trace(tr("GTF parsing error: incorrect frame"
                " value \"%1\" at line %2!").arg(gtfLineData.frame).arg(lineNumber));
        }
        else if (NO_VALUE_STR != gtfLineData.frame) {
            annotData->qualifiers.push_back(U2Qualifier(FRAME_QUALIFIER_NAME, gtfLineData.frame));
        }


        foreach (QString attributeName, gtfLineData.attributes.keys()) {
            U2Qualifier qualifier(attributeName, gtfLineData.attributes.value(attributeName));
            if (!qualifier.isValid()) {
                validationStatus.setFlagIncorrectFormatOfAttributes();
            }

            // If all qualifier are correct, add the qualifiers
            if (!validationStatus.isIncorrectFormatOfAttributes()) {
                annotData->qualifiers.push_back(qualifier);
            }
        }

        if (validationStatus.isIncorrectFormatOfAttributes()) {
            ioLog.trace(tr("GTF parsing error: invalid attributes"
                " format at line %1!").arg(lineNumber));
        }


        // Verify that mandatory attributes "gene_id" and "transcript_id" are present
        if (validationStatus.isGeneIdAbsent()) {
            ioLog.trace(tr("GTF parsing error: mandatory attribute '") +
                GENE_ID_QUALIFIER_NAME + tr("' is absent at line %1!").arg(lineNumber));
        }

        if (validationStatus.isTranscriptIdAbsent()) {
            ioLog.trace(tr("GTF parsing error: mandatory attribute '") +
                TRANSCRIPT_ID_QUALIFIER_NAME + tr("' is absent at line %1!").arg(lineNumber));
        }


        // Verify the strand
        if (validationStatus.isIncorrectStrand()) {
            // Write the error to the log, but open the file
            ioLog.trace(tr("GTF parsing error: incorrect strand"
                " value \"%1\" at line %2!").arg(gtfLineData.strand).arg(lineNumber));
        }
        else if ("+" == gtfLineData.strand) {
            annotData->qualifiers.push_back(U2Qualifier(STRAND_QUALIFIER_NAME, L10N::directStrandStr()));
        }
        else if ("-" == gtfLineData.strand) {
            annotData->setStrand(U2Strand::Complementary);
            annotData->qualifiers.push_back(U2Qualifier(STRAND_QUALIFIER_NAME, L10N::complementStrandStr()));
        }

        // Append the result
        result.append(annotData);

        // Move to the next line
        lineNumber++;
    }

    if (false == fileIsValid) {
        ioLog.error("GTF parsing error: one or more errors occurred while parsing the input file,"
            " see TRACE log for details!");
    }

    return result;
}


void GTFFormat::load(IOAdapter* io, QList<GObject*>& objects, U2OpStatus& os)
{
    QString sequenceName;
    QList<SharedAnnotationData> annotations = parseDocument(io, sequenceName, os);

    foreach (SharedAnnotationData annotData, annotations) {
        // Get or create the annotations table

        QString annotTableName = sequenceName + FEATURES_TAG;
        AnnotationTableObject* annotTable = NULL;
        foreach (GObject* object, objects) {
            if (object->getGObjectName() == annotTableName) {
                annotTable = (AnnotationTableObject*) object;
            }
        }
        if (!annotTable) {
            annotTable = new AnnotationTableObject(annotTableName);
            objects.append(annotTable);
        }

        QString groupName = annotData->name; // Assume that the group name is the same as the annotation name
        if (!AnnotationGroup::isValidGroupName(groupName, false)) {
            groupName = "Group"; // Or just a value if the name of the feature is not appropriate
        }

        annotTable->addAnnotation(new Annotation(annotData), groupName);
    }
}


FormatCheckResult GTFFormat::checkRawData(const QByteArray& rawData, const GUrl& /* = GUrl */) const
{
    const char* data = rawData.constData();
    int size = rawData.size();

    bool hasBinaryData = TextUtils::contains(TextUtils::BINARY, data, size);
    if (hasBinaryData) {
        return FormatDetection_NotMatched;
    }

    // There should be: '<seqname> <source> <feature> <start> <end> <score> <strand> <frame>'
    // and two attributes at the end of the line are also required: 'gene_id "something";' and
    // 'transcript_id "something"'. This is, at least, 35 characters.
    const int MIN_POSSIBLE_LINE_SIZE = 35;
    if (size < MIN_POSSIBLE_LINE_SIZE) {
        return FormatDetection_NotMatched;
    }

    QString dataStr(rawData);
    QStringList fileLines = dataStr.split("\n");
    GTFLineValidateFlags validationStatus;

    int numToIterate;
    int HUGE_DATA = 65536;
    if (size < HUGE_DATA) {
        numToIterate = fileLines.size(); 
    }
    else {
        // Skip the last line as it can be incomplete
        numToIterate = fileLines.size() - 1;
    }

    for (int i = 0; i < numToIterate; ++i) {
        if (!fileLines[i].trimmed().isEmpty()) { // e.g. the last line in file can be empty
            parseAndValidateLine(fileLines[i], validationStatus);
        }
    }

    return validationStatus.getFormatDetectionScore();
}


// Attributes must end in a semicolon which must then be separated
// from the start of any subsequent attribute by exactly one space
// character (NOT a tab character).
// Textual attributes should be surrounded by double quotes.
bool parseAttributes(QString attributesStr, QMap<QString, QString>& parsedAttrValues)
{
    QMap<QString, QString> result;

    QString attributeName;
    QString attributeValue;

    int pos = 0;

    while (pos < attributesStr.size()) {
        int spaceCharIndex = attributesStr.indexOf(' ', pos);

        if (-1 == spaceCharIndex) {
            // There are characters, but there is no a space character
            return false;
        }

        attributeName = attributesStr.mid(pos, spaceCharIndex - pos);

        if (attributeName.contains(';')) {
            // Semicolon is a separator. If the attribute name contains a semicolon,
            // then, probably an error occurred.
            return false;
        }

        // Set the position to the next position after the space character
        pos = spaceCharIndex + 1;

        int semicolonCharIndex = attributesStr.indexOf(';', pos);

        if (-1 == semicolonCharIndex) {
            // There is an attribute name, but no a semicolon
            return false;
        }

        // Skip double quotes for textual attributes
        if (('"' == attributesStr[pos]) && ('"' == attributesStr[semicolonCharIndex - 1])) {
            attributeValue = attributesStr.mid(pos + 1, semicolonCharIndex - pos - 2);
        }
        else {
            attributeValue = attributesStr.mid(pos, semicolonCharIndex - pos);
            bool attributeValueIsInt;
            bool attributeValueIsFloat;
            attributeValue.toInt(&attributeValueIsInt);
            attributeValue.toFloat(&attributeValueIsFloat);
            if (!attributeValueIsInt && !attributeValueIsFloat) {
                // This must be a number, therefore format is incorrect
                return false;
            }
        }

        // Different attributes must be separated by one space
        if ((' ' != attributesStr[semicolonCharIndex + 1])) {
            // This is not the end of the line
            if (attributesStr.size() != semicolonCharIndex + 1) {
                return false;
            }
        }

        // Add the attribute to the parsed values
        parsedAttrValues[attributeName] = attributeValue;

        // Go to the next attribute (+ 2 as we skip "; " at the end of the current attribute)
        pos = semicolonCharIndex + 2;
    }

    // Attributes string was successfully parsed
    return true;
}


GTFLineData GTFFormat::parseAndValidateLine(QString line, GTFLineValidateFlags& status) const
{
    GTFLineData parsedData;

    // If there is a comment char, then ignore all characters after it
    const QChar COMMENT_CHAR('#');
    int commentCharIndex = line.indexOf(COMMENT_CHAR);

    if (-1 != commentCharIndex) {
        line = line.left(commentCharIndex);
    }

    // All fields should be separated by a single TAB
    // and no white spaces
    QStringList fields = line.split("\t");


    // Verify that there is required number of fields and they are not empty
    if (FIELDS_COUNT_IN_EACH_LINE != fields.count()) {
        status.setFlagIncorrectNumberOfFields();
        return parsedData; // Do not continue to validate the line
    }

    foreach (QString field, fields) {
        if (field.trimmed().isEmpty()) {
            status.setFlagEmptyField();
            return parsedData; // Do not continue to validate the line
        }
    }

    // Coordinates
    bool startIsInt;
    bool endIsInt;
    int start = fields[GTF_START_INDEX].toInt(&startIsInt);
    int end = fields[GTF_END_INDEX].toInt(&endIsInt);
    if (!startIsInt || !endIsInt || (start < 1) || (start > end)) {
        status.setFlagIncorrectCoordinates();
        return parsedData; // Do not continue to validate the line
    }


    // Attributes
    QMap<QString, QString> parsedAttrValues;
    bool attributesParsedSuccessfully = parseAttributes(fields[GTF_ATTRIBUTES_INDEX], parsedAttrValues);
    if (!attributesParsedSuccessfully) {
        status.setFlagIncorrectFormatOfAttributes();
    }

    if (!parsedAttrValues.keys().contains(GENE_ID_QUALIFIER_NAME)) {
        status.setFlagNoGeneIdAttribute();
    }

    if (!parsedAttrValues.keys().contains(TRANSCRIPT_ID_QUALIFIER_NAME)) {
        status.setFlagNoTrascriptIdAttribute();
    }


    // Fill in the data and continue validation even if a value is incorrect
    parsedData.seqName = fields[GTF_SEQ_NAME_INDEX];
    parsedData.source = fields[GTF_SOURCE_INDEX];
    parsedData.feature = fields[GTF_FEATURE_INDEX];
    parsedData.region = U2Region(start - 1, end - start + 1); // In GTF sequence numbering starts at 1
    parsedData.score = fields[GTF_SCORE_INDEX];
    parsedData.strand = fields[GTF_STRAND_INDEX];
    parsedData.frame = fields[GTF_FRAME_INDEX];
    parsedData.attributes = parsedAttrValues;


    // Feature
    if (!GTF_FEATURE_FIELD_VALUES.contains(parsedData.feature)) {
        status.setFlagIncorrectFeatureField();
    }


    // Score: can be either an integer, a float number, or a dot (".")
    bool scoreIsInt;
    parsedData.score.toInt(&scoreIsInt);
    if (!scoreIsInt) {
        bool scoreIsFloat;
        parsedData.score.toFloat(&scoreIsFloat);
        if (!scoreIsFloat) {
            if (NO_VALUE_STR != parsedData.score) {
                status.setFlagIncorrectScore();
            }
        }
    }


    // Strand
    if (("+" != parsedData.strand) &&
        ("-" != parsedData.strand) &&
        (NO_VALUE_STR != parsedData.strand))
    {
        status.setFlagIncorrectStrand();
    }


    // Frame
    bool frameIsInt;
    int frame = parsedData.frame.toInt(&frameIsInt);
    if (!frameIsInt && (NO_VALUE_STR != parsedData.frame))
    {
        status.setIncorrectFrame();
    }
    else if ((0 != frame) && (1 != frame) && (2 != frame)) {
        status.setIncorrectFrame();
    }


    return parsedData;
}


void GTFFormat::storeDocument(Document* doc, IOAdapter* io, U2OpStatus& os)
{
    bool noErrorsDuringStoring = true;
    QList<GObject*> annotTables = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);

    QStringList cleanFields;
    for (int i = 0; i < FIELDS_COUNT_IN_EACH_LINE; ++i) {
        cleanFields.append(NO_VALUE_STR);
    }

    QByteArray lineData;

    foreach (GObject* annotTable, annotTables) {
        QList<Annotation*> annotationsList =
            (qobject_cast<AnnotationTableObject*>(annotTable))->getAnnotations();

        foreach (const Annotation* annot, annotationsList) {
            QString annotName = annot->getAnnotationName();
            if (annotName == U1AnnotationUtils::lowerCaseAnnotationName ||
                annotName == U1AnnotationUtils::upperCaseAnnotationName)
            {
                    continue;
            }

            QStringList lineFields = cleanFields;
            QVector<U2Region> annotRegions = annot->getRegions();
            QVector<U2Qualifier> annotQualifiers = annot->getQualifiers();

            // Joined annotations are currently stored as other annotations (we do not store that they are joined)
            foreach (const U2Region region, annotRegions) {
                QString annotTableName = annotTable->getGObjectName();
                if (annotTableName.endsWith(FEATURES_TAG)) {
                    lineFields[GTF_SEQ_NAME_INDEX] = 
                        annotTableName.left(annotTableName.size() - QString(FEATURES_TAG).size());
                }
                else {
                    lineFields[GTF_SEQ_NAME_INDEX] = annotTableName;
                }

                lineFields[GTF_FEATURE_INDEX] = annotName;

                lineFields[GTF_START_INDEX] = QString::number(region.startPos + 1);
                lineFields[GTF_END_INDEX] = QString::number(region.endPos());

                QString geneIdAttributeStr;
                QString transcriptIdAttributeStr;
                QString otherAttributesStr;
                foreach (U2Qualifier qualifier, annotQualifiers) {
                    if (CHROMOSOME == qualifier.name) {
                        lineFields[GTF_SEQ_NAME_INDEX] = qualifier.value;
                    }
                    else if (SOURCE_QUALIFIER_NAME == qualifier.name) {
                        lineFields[GTF_SOURCE_INDEX] = qualifier.value;
                    }
                    else if (SCORE_QUALIFIER_NAME == qualifier.name) {
                        lineFields[GTF_SCORE_INDEX] = qualifier.value;
                    }
                    else if (STRAND_QUALIFIER_NAME == qualifier.name) {
                        if (L10N::directStrandStr() == qualifier.value) {
                            lineFields[GTF_STRAND_INDEX] = "+";
                        }
                        else if (L10N::complementStrandStr() == qualifier.value) {
                            lineFields[GTF_STRAND_INDEX] = "-";
                        }
                        else {
                            noErrorsDuringStoring = false;
                            ioLog.trace(tr("GTF saving error: unknown value"
                                " of the strand qualifier \"%1\" of an annotation"
                                " \"%2\" was skipped!").arg(qualifier.name).arg(annotName));
                        }
                    }
                    else if (FRAME_QUALIFIER_NAME == qualifier.name) {
                        lineFields[GTF_FRAME_INDEX] = qualifier.value;
                    }
                    else {
                        // All other qualifiers are saved as attributes
                        QString attrStr = qualifier.name + " \"" + qualifier.value + "\";";
                        if (GENE_ID_QUALIFIER_NAME != qualifier.name) {
                            attrStr = " " + attrStr; // Exactly one space char between different attributes
                        }

                        if (GENE_ID_QUALIFIER_NAME == qualifier.name) {
                            geneIdAttributeStr = attrStr;
                        }
                        else if (TRANSCRIPT_ID_QUALIFIER_NAME == qualifier.name) {
                            transcriptIdAttributeStr = attrStr;
                        }
                        else {
                            otherAttributesStr += attrStr;
                        }                        
                    }
                }
                lineFields[GTF_ATTRIBUTES_INDEX] = geneIdAttributeStr +
                    transcriptIdAttributeStr +
                    otherAttributesStr;

                lineData = lineFields.join("\t").toAscii() + "\n";
                qint64 len = io->writeBlock(lineData);
                if (len != lineData.size()) {
                    os.setError(L10N::errorWritingFile(doc->getURL()));
                    return;
                }
            }
        }
    }

    if (!noErrorsDuringStoring) {
        ioLog.error(tr("GTF saving error: one or more errors occurred while saving a file,"
            " see TRACE log for details!"));
    }
}


} // namespace
