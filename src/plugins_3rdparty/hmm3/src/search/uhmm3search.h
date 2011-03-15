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

#ifndef _GB2_UHMM3_SEARCH_H_
#define _GB2_UHMM3_SEARCH_H_

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

#include <U2Core/Task.h>

#include <hmmer3/hmmer.h>

#include "uhmm3SearchResult.h"

namespace U2 {

class UHMM3Search : public QObject {
    Q_OBJECT
public:
    static UHMM3SearchResult search( const P7_HMM* ahmm, const char* sq, int sqLen, 
                                     const UHMM3SearchSettings& set, TaskStateInfo& si, int wholeSeqSz );
    
}; // UHMM3Search

} // U2

#endif // _GB2_UHMM3_SEARCH_H_
