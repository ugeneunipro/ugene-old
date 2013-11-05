/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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


#include "TBP_TATA.h"
#include "AnalyzeTataBoxesTask.h"
#include "../SnpRequestKeys.h"

#include <U2Core/U2SafePoints.h>

#include <QtCore/QScopedArrayPointer>

#include <math.h>

namespace U2 {

AnalyzeTataBoxesTask::AnalyzeTataBoxesTask( const QVariantMap &inputData, const U2Variant& var  )
:BaseSnpAnnotationTask(inputData, var, "Get SNP effect on tata boxes")
{
}

void AnalyzeTataBoxesTask::prepare(){
    if (!inputData.contains(SnpRequestKeys::TATA_TOOLS_SEQ_1) || !inputData.contains(SnpRequestKeys::TATA_TOOLS_SEQ_2)){
        return;
    }
    QByteArray seq1 = inputData[SnpRequestKeys::TATA_TOOLS_SEQ_1].toByteArray();
    QByteArray seq2 = inputData[SnpRequestKeys::TATA_TOOLS_SEQ_2].toByteArray();

    SAFE_POINT(seq1.size() == seq2.size(), "AnalyzeTataBoxesTask::sites of tata boxes must be similar in the length", );

    QPair <float, float> seq1maxValues = getMaxValues(seq1);
    if (seq1maxValues.first == -1.0f || seq1maxValues.first == -1.0f){
        return;
    }

    QPair <float, float> seq2maxValues = getMaxValues(seq2);
    if (seq2maxValues.first == -1.0f || seq2maxValues.first == -1.0f){
        return;
    }

    double se = pow(seq1maxValues.second, 2) + pow(seq2maxValues.second, 2);
    se = sqrt(se) / 2.0;

    double sigma = (seq1maxValues.first > seq2maxValues.first ) ?  seq1maxValues.first - seq2maxValues.first : seq2maxValues.first - seq1maxValues.first;
    sigma = sigma / se;

    QString res = "";
    if (sigma>=4.891638475699) {res = QString("%1/0.999999").arg(sigma);}
    else if (sigma>=3.290526731492) {res = QString("%1/0.999").arg(sigma);}
    else if (sigma>=2.575829303549) {res = QString("%1/0.99").arg(sigma);}
    else if (sigma>=1.959963984540) {res = QString("%1/0.95").arg(sigma);}
    else {res = QString("%1/unsignificant").arg(sigma);}

    result[SnpResponseKeys::TATA_TOOLS] = res;
}

void AnalyzeTataBoxesTask::run(){
   
}

QVariantMap AnalyzeTataBoxesTask::getResult(){
    return result;
}

#define CLEAR_VALUE -999.
void AnalyzeTataBoxesTask::clearArray( float* arr, int len ){
    for (int i = 0; i < len; i++){
        arr[i] = CLEAR_VALUE;
    }
}

int AnalyzeTataBoxesTask::idxOfMaxElement( float* arr, int len ){
    int res = -1;
    float maxElement = float(-INT_MAX);

    for (int i = 0; i < len; i++){
        if (maxElement < arr[i]){
            maxElement = arr[i];
            res = i;
        }
    }
    
    return res;
}

QPair<float, float> AnalyzeTataBoxesTask::getMaxValues( QByteArray& seq){
    QPair<float, float> res;
    res.first = -1.0f;
    res.second = -1.0f;

    QScopedArrayPointer<float> directScores(new float[seq.size()]);
    QScopedArrayPointer<float> complementScores(new float[seq.size()]);
    int len = -1;
    clearArray(directScores.data(), seq.size());
    clearArray(complementScores.data(), seq.size());

    int resTbp = MATRIX_TBP(seq.data(), directScores.data(), complementScores.data(), &len);
    if(len == -1 || resTbp != 1){
        return res;
    }

    int idxMaxElement = idxOfMaxElement(directScores.data(), len);
    if (idxMaxElement == -1){
        return res;
    }
    res.first = directScores[idxMaxElement];
    res.second = complementScores[idxMaxElement];

    return res;
}

} // namespace U2
