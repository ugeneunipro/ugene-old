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

#ifndef _U2_ANALYZE_TATA_BOXES_TASK_
#define _U2_ANALYZE_TATA_BOXES_TASK_

#include "RequestForSnpTask.h"

#include <U2Core/Task.h>

#include <QtCore/QUrl>
#include <QtCore/QPair>

namespace U2 {

class AnalyzeTataBoxesTask :   public BaseSnpAnnotationTask
{
public:
                                AnalyzeTataBoxesTask(const QVariantMap &inputData, const U2Variant& var );

    void                        run( );
    QVariantMap                 getResult( );

private:
    QVariantMap                 result;

private:
    void clearArray(float* arr, int len);
    int idxOfMaxElement(float* arr, int len);
    QPair<float, float> getMaxValues(QByteArray& seq);
};

} // U2

#endif // _U2_ANALYZE_TATA_BOXES_TASK_