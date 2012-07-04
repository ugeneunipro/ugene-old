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

#ifndef _U2_CUFFLINKS_SETTINGS_H_
#define _U2_CUFFLINKS_SETTINGS_H_

#include "../RnaSeqCommon.h"

#include <U2Core/AnnotationData.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/DbiDataStorage.h>


namespace U2 {


class CufflinksSettings
{
public:
    CufflinksSettings();

    // Workflow element parameters
    QString             referenceAnnotation;
    QString             rabtAnnotation;
    RnaSeqLibraryType   libraryType;
    QString             maskFile;
    bool                multiReadCorrect;
    double              minIsoformFraction;
    QString             fragBiasCorrect;
    double              preMrnaFraction;

    // Input assembly
    Workflow::DbiDataStorage*           storage;
    Workflow::SharedDbiDataHandler      assemblyId;
};


} // namespace

#endif
