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

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "DifferentialExpressionFormat.h"

namespace U2 {

static const QString SEPARATOR("\t");
static const QString LOCUS_COLUMN("locus");
static const QString LOCUS_SEP1(":");
static const QString LOCUS_SEP2("-");
static const QString CHROMOSOME("chromosome");
static const QString UNKNOWN_CHR("unknown");
static const QString LOG2_COLUMN("log2(fold_change)");

/************************************************************************/
/* DifferentialFormat */
/************************************************************************/
DifferentialExpressionFormat::DifferentialExpressionFormat(QObject *parent)
: AbstractDifferentialFormat(parent)
{
    formatName = tr("Diff expression");
    supportedObjectTypes += GObjectTypes::ANNOTATION_TABLE;
    formatDescription = tr("FASTA format is a text-based format for representing either nucleotide sequences or peptide sequences, in which base pairs or amino acids are represented using single-letter codes. The format also allows for sequence names and comments to precede the sequences.");
    annotationName = "differential_expression";
    columns = QList<ColumnDataParser::Column>()
        << ColumnDataParser::Column("test_id", ColumnDataParser::STRING, "-", true /*required*/)
        << ColumnDataParser::Column("gene_id", ColumnDataParser::STRING, "-", true /*required*/)
        << ColumnDataParser::Column("gene", ColumnDataParser::STRING)
        << ColumnDataParser::Column(LOCUS_COLUMN, ColumnDataParser::STRING, "-", true /*required*/)
        << ColumnDataParser::Column("sample_1", ColumnDataParser::STRING, "-", true /*required*/)
        << ColumnDataParser::Column("sample_2", ColumnDataParser::STRING, "-", true /*required*/)
        << ColumnDataParser::Column("status", ColumnDataParser::STRING, "OK", true /*required*/)
        << ColumnDataParser::Column("value_1", ColumnDataParser::DOUBLE, "1")
        << ColumnDataParser::Column("value_2", ColumnDataParser::DOUBLE, "1")
        << ColumnDataParser::Column(LOG2_COLUMN, ColumnDataParser::DOUBLE, "0")
        << ColumnDataParser::Column("test_stat", ColumnDataParser::DOUBLE, "0")
        << ColumnDataParser::Column("p_value", ColumnDataParser::DOUBLE, "1")
        << ColumnDataParser::Column("q_value", ColumnDataParser::DOUBLE, "1")
        << ColumnDataParser::Column("significant", ColumnDataParser::STRING, "-");
}

DocumentFormatId DifferentialExpressionFormat::getFormatId() const {
    return BaseDocumentFormats::DIFF_EXP;
}

const QString & DifferentialExpressionFormat::getFormatName() const {
    return formatName;
}

QList<ColumnDataParser::Column> DifferentialExpressionFormat::getColumns() const {
    return columns;
}

QString DifferentialExpressionFormat::getAnnotationName() const {
    return annotationName;
}

} // U2
