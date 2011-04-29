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


#include <U2Core/AppContext.h>
#include <U2Algorithm/CudaGpuRegistry.h>
#include <cuda_runtime.h>
#include "SuffixSearchCUDA.h"


namespace U2 {

extern "C" void cudaBinarySearch(quint64* pList1Dev, int nList1Length, quint64* pList2Dev, int nList2Length);

quint64* U2::SuffixSearchCUDA::runSearch( const quint64* numbers, const int numbersSize, 
                                            const quint64* query, const int querySize, quint64 filter )
{
    cudaMalloc((void **)&numbersListDev, numbersSize * sizeof(quint64));
    cudaMalloc((void **)&queryListDev,querySize * sizeof(quint64));
    cudaMemcpy(numbersListDev, numbers, numbersSize * sizeof(quint64), cudaMemcpyHostToDevice);
    cudaMemcpy(queryListDev, query, querySize * sizeof(quint64), cudaMemcpyHostToDevice);
    
    cudaBinarySearch(numbersListDev, numbersSize, queryListDev, querySize);
    quint64* results = new quint64[querySize];
    cudaMemcpy(results, queryListDev, querySize * sizeof(quint64), cudaMemcpyDeviceToHost);
    
    /*for (int i = 0; i < querySize; ++i) {
        printf("results[%d] = %d", i, results[i]);
    }*/


    cudaFree((void **)&numbersListDev); 
    cudaFree((void **)&queryListDev); 

    return results;
}



} //namespace

#endif


