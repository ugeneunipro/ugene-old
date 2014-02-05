/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_BED_FORMAT_H_
#define _U2_BED_FORMAT_H_

#include <U2Core/AnnotationData.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2Region.h>

namespace U2 {

/** Validates a line from a BED file */
class BEDLineValidateFlags
{
public:
    BEDLineValidateFlags();

    bool incorrectNumberOfFields; // There must be at least 3 fields, all lines must have the same number of fields
    bool emptyFields; // Each field shouldn't be empty or shouldn't consist of white spaces
    bool incorrectCoordinates; // Start and end should be integer (the first base in a chromosome is numbered 0), start should be < end, end is not included
    bool incorrectScore; // A score between 0 and 1000
    bool incorrectStrand; // Either '+' or '-'
    bool incorrectThickCoordinates; // start <= thickStart <= end
                                    // start <= thickEnd <= end
                                    // thickStart < thickEnd
    bool incorrectItemRgb; // Must be an RGB value of the form R, G, B
    bool incorrectBlocks; // There are 1) Block numbers - the number of blocks (exons) in the BED line, i.e. must be an integer
                          // Block sizes is a comma-separated list of the block sizes
                          // Block starts is a comma-separated list of block starts

    bool hasTrackLine;

    FormatDetectionScore getFormatDetectionScore();

    bool isFileInvalid() {
        return incorrectNumberOfFields ||
            emptyFields ||
            incorrectCoordinates ||
            incorrectScore ||
            incorrectStrand ||
            incorrectThickCoordinates ||
            incorrectItemRgb ||
            incorrectBlocks;
    }
};

enum BedLineFieldsIndeces {BED_CHROM_NAME_INDEX = 0,
    BED_CHROM_START_INDEX = 1,
    BED_CHROM_END_INDEX = 2,
    BED_ANNOT_NAME_INDEX = 3,
    BED_SCORE_INDEX = 4,
    BED_STRAND_INDEX = 5,
    BED_THICK_START_INDEX = 6,
    BED_THICK_END_INDEX = 7,
    BED_ITEM_RGB_INDEX = 8,
    BED_BLOCK_COUNT_INDEX = 9,
    BED_BLOCK_SIZES_INDEX = 10,
    BED_BLOCK_STARTS_INDEX = 11
};


struct BedLineData
{
    QString seqName;
    U2Region region;
    QColor annotColor; // Used only if the "itemRgb" value was read
    QMap<QString, QString> additionalFields;
};


class IOAdapter;

/**
 * The following specification was used: http://genome.ucsc.edu/FAQ/FAQformat.html#format1
 * Track lines (see http://genome.ucsc.edu/goldenPath/help/customTrack.html#TRACK)
 * are partially supported: only a name and a description of a track.
 */
class U2FORMATS_EXPORT BedFormat : public DocumentFormat {
    Q_OBJECT

public:
    BedFormat(QObject* parent);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::BED;}

    virtual const QString& getFormatName() const {return FORMAT_NAME;}

    virtual void storeDocument(Document* doc, IOAdapter* io, U2OpStatus& os);

    virtual FormatCheckResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

    /** Gets annotation data from a BED file, but doesn't create an annotation table */
    static QList<SharedAnnotationData> getAnnotData(IOAdapter* io, U2OpStatus& os);

protected:
    virtual Document* loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os);

    BedLineData parseAndValidateLine(const QString& line, int numOfFields, BEDLineValidateFlags& status) const;

    /**
    * A common method for parsing and validating an input file.
    * It is used during loading the file or just getting the annotations data from it.
    */
    QHash<QString, QList<SharedAnnotationData> > parseDocument(IOAdapter* io, const QString& defaultAnnotName, U2OpStatus& os);

private:
    void load(IOAdapter* io, QList<GObject*>& objects, const U2DbiRef& dbiRef, U2OpStatus& os);
    inline void addToResults(QHash<QString, QList<SharedAnnotationData> > & resHash, QList<SharedAnnotationData>& result, const QString& seqName );

    static const QString FORMAT_NAME;

    static const QString TRACK_NAME_QUALIFIER_NAME;
    static const QString TRACK_DESCR_QUALIFIER_NAME;
    static const QString CHROM_QUALIFIER_NAME;
    static const QString ANNOT_QUALIFIER_NAME;
    static const QString SCORE_QUALIFIER_NAME;
    static const QString STRAND_QUALIFIER_NAME;
    static const QString THICK_START_QUALIFIER_NAME;
    static const QString THICK_END_QUALIFIER_NAME;
    static const QString ITEM_RGB_QUALIFIER_NAME;
    static const QString BLOCK_COUNT_QUALIFIER_NAME;
    static const QString BLOCK_SIZES_QULAIFIER_NAME;
    static const QString BLOCK_STARTS_QUALIFIER_NAME;
};

}//namespace

#endif
