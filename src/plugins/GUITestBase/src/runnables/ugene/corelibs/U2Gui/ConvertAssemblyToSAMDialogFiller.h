/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GT_CONVERT_ASSEMBLY_TO_SAM_DIALOG_FILLER_H
#define _U2_GT_CONVERT_ASSEMBLY_TO_SAM_DIALOG_FILLER_H

#include "utils/GTUtilsDialog.h"
#include <base_dialogs/GTFileDialog.h>

namespace U2 {
using namespace HI;

    class ConvertAssemblyToSAMDialogFiller : public Filler {
    public:
        ConvertAssemblyToSAMDialogFiller(HI::GUITestOpStatus &_os,
                                         const QString &_dbPath, const QString &_dbFileName):
            Filler(_os, "AssemblyToSamDialog"),
            databasePath(_dbPath),
            databaseFileName(_dbFileName){}
        void commonScenario();
    private:
        const QString databasePath;
        const QString databaseFileName;
    };
}

#endif // _U2_GT_CONVERT_ASSEMBLY_TO_SAM_DIALOG_FILLER_H
