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

#ifndef _GB2_UHMM3_PHMMER_H_
#define _GB2_UHMM3_PHMMER_H_

#include <QtCore/QObject>

#include <U2Core/Task.h>
#include <U2Core/SMatrix.h>

#include <hmmer3/hmmer.h>

#include <search/uhmm3SearchResult.h>

namespace U2 {

class UHMM3PhmmerSettings {
public:
    // same as in UHMM3SearchSettings
    double e;
    double t;
    double z;
    double domE;
    double domT;
    double domZ;
    
    double incE;
    double incT;
    double incDomE;
    double incDomT;
    
    double f1;
    double f2;
    double f3;
    int doMax;
    int noBiasFilter;
    int noNull2;
    
    // same as in UHMM3BuildSettings
    int     eml;                        // --EmL. length of sequences for MSV Gumbel mu fit
    int     emn;                        // --EmN. number of sequences for MSV Gumbel mu fit
    int     evl;                        /* length of sequences for Viterbi Gumbel mu fit */
    int     evn;                        /* number of sequences for Viterbi Gumbel mu fit */
    int     efl;                        /* length of sequences for Forward exp tail mu fit */
    int     efn;                        /* number of sequences for Forward exp tail mu fit */
    float   eft;                        /* tail mass for Forward exponential tail mu fit */
    int     seed;
    
    // scoring system
	double          popen;                        /* gap open probability */
    double          pextend;                      /* gap extend probability */
    SMatrix         substMatr;                    /* default is null here -> BLOSUM62 there */
	
    UHMM3PhmmerSettings();
    
    UHMM3BuildSettings getBuildSettings() const;
    void setBuildSettings( const UHMM3BuildSettings & from );
    
    UHMM3SearchSettings getSearchSettings() const;
    void setSearchSettings( const UHMM3SearchSettings & from );
    
    bool isValid() const;
    
}; // UHMM3PhmmerSettings

class UHMM3Phmmer : public QObject {
    Q_OBJECT
public:
    static UHMM3SearchResult phmmer( const char * querySq, int querySqLen, const char * dbSq, int dbSqLen, 
                                     const UHMM3PhmmerSettings & settings, TaskStateInfo & ti, int wholeSeqSz );
    
}; // UHMM3Phmmer

} // U2

#endif // _GB2_UHMM3_PHMMER_H_
