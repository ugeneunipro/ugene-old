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


#ifdef GA_BUILD_WITH_CUDA

#ifndef _U2_SUFFIX_SEARCH_CUDA_H_
#define _U2_SUFFIX_SEARCH_CUDA_H_

#include <qglobal.h>

namespace U2 {

class SuffixSearchCUDA {
public:
    SuffixSearchCUDA() {}
    quint64* runSearch(const quint64* _numbers, const int _numbersSize, 
        const quint64* _findNumbers, const int _findNumbersSize, 
        quint64 filter);
private:
    quint64 *numbersListDev, *queryListDev; 

};

#endif //_U2_SUFFIX_SEARCH_CUDA_H_

} //namespace

#endif
