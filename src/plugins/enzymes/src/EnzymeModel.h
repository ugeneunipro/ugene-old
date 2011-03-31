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

namespace U2 {

#define ULOG_ENZYME_PLUGIN QString("Plugin: Enzymes")

class DNAAlphabet;

#define QUALIFIER_SPLIT                 "SPLIT"
#define QUALIFIER_LEFT_TERM             "5'term"
#define QUALIFIER_LEFT_OVERHANG         "5'overhang"
#define QUALIFIER_LEFT_TYPE             "5'type"
#define QUALIFIER_RIGHT_TERM            "3'term"
#define QUALIFIER_RIGHT_OVERHANG        "3'overhang"
#define QUALIFIER_RIGHT_TYPE            "3'type"
#define QUALIFIER_SPLIT_TERM            "split_term"
#define QUALIFIER_SPLIT_OVERHANG        "split_overhang"
#define QUALIFIER_SPLIT_TYPE            "split_type"
#define QUALIFIER_SOURCE                "fragment_source"
#define QUALIFIER_INVERTED              "fragment_inverted"
#define OVERHANG_TYPE_BLUNT             "blunt"
#define OVERHANG_TYPE_STICKY            "sticky"
#define ANNOTATION_GROUP_FRAGMENTS      "fragments"
#define ANNOTATION_GROUP_ENZYME         "enzyme"

#define ENZYME_CUT_UNKNOWN 0x7FFFFF

// Enzymes plugin settings

#define DATA_DIR_KEY        "enzymes"
#define DATA_FILE_KEY       "plugin_enzymes/lastFile"
#define LAST_SELECTION      "plugin_enzymes/selection"
#define ENABLE_HIT_COUNT    "plugin_enzymes/enable_hit_count"
#define MAX_HIT_VALUE       "plugin_enzymes/max_hit_value"
#define MIN_HIT_VALUE       "plugin_enzymes/min_hit_value"
#define NON_CUT_REGION      "plugin_enzymes/non_cut_region"
#define MAX_RESULTS         "plugin_enzymes/max_results"
#define SEP                 ","
#define COMMON_ENZYMES      "ClaI,BamHI,BglII,DraI,EcoRI,EcoRV,HindIII,PstI,SalI,SmaI,XmaI"

class EnzymeData: public QSharedData {
public:
    inline EnzymeData();

    QString         id;
    QString         accession;
    QString         type;
    QByteArray      seq;
    int             cutDirect; //starts from the first char in direct strand
    int             cutComplement; // starts from the first char in complement strand, negative->right offset
    QString         organizm;
    DNAAlphabet*    alphabet;
};

EnzymeData::EnzymeData() {
    cutDirect = ENZYME_CUT_UNKNOWN;
    cutComplement = ENZYME_CUT_UNKNOWN;
    alphabet = NULL;
}

typedef QSharedDataPointer<EnzymeData> SEnzymeData;

} //namespace

#endif
