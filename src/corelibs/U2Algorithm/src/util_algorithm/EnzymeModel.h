/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_ENZYME_MODEL_H_
#define _U2_ENZYME_MODEL_H_

#include <QtCore/QSharedData>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QByteArray>

#include <U2Core/global.h>

namespace U2 {

#define ULOG_ENZYMES QString("Restriction Enzymes")

class DNAAlphabet;

#define QUALIFIER_LEFT_TERM             "5'term"
#define QUALIFIER_LEFT_OVERHANG         "5'overhang"
#define QUALIFIER_LEFT_TYPE             "5'type"
#define QUALIFIER_RIGHT_TERM            "3'term"
#define QUALIFIER_RIGHT_OVERHANG        "3'overhang"
#define QUALIFIER_RIGHT_TYPE            "3'type"
#define QUALIFIER_SOURCE                "fragment_source"
#define QUALIFIER_INVERTED              "fragment_inverted"
#define OVERHANG_TYPE_BLUNT             "blunt"
#define OVERHANG_TYPE_STICKY            "sticky"
#define ANNOTATION_GROUP_FRAGMENTS      "fragments"
#define ANNOTATION_GROUP_ENZYME         "enzyme"

#define ENZYME_CUT_UNKNOWN              0x7FFFFF
#define ENZYME_LIST_SEPARATOR           ","

class U2ALGORITHM_EXPORT EnzymeSettings {
public:
    static const QString DATA_DIR_KEY;
    static const QString DATA_FILE_KEY;
    static const QString LAST_SELECTION;
    static const QString ENABLE_HIT_COUNT;
    static const QString MAX_HIT_VALUE;
    static const QString MIN_HIT_VALUE;
    static const QString NON_CUT_REGION;
    static const QString MAX_RESULTS;
    static const QString COMMON_ENZYMES;
};

class U2ALGORITHM_EXPORT EnzymeData : public QSharedData {
public:
    EnzymeData(); 

    QString         id;
    QString         accession;
    QString         type;
    QByteArray      seq;
    int             cutDirect; //starts from the first char in direct strand
    int             cutComplement; // starts from the first char in complement strand, negative->right offset
    QString         organizm;
    DNAAlphabet*    alphabet;
};


typedef QSharedDataPointer<EnzymeData> SEnzymeData;

} //namespace

#endif
