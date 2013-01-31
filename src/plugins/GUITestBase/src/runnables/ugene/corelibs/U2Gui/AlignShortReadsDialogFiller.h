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

#ifndef _U2_GT_ALIGN_SHORT_READS_DIALOG_FILLER_H_
#define _U2_GT_ALIGN_SHORT_READS_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"
#include "api/GTFileDialog.h"

namespace U2 {

    class AlignShortReadsFiller : public Filler {
    public:
        AlignShortReadsFiller(U2OpStatus &_os, const QString &_path, const QString &_FileName,
                              const QString &_path1, const QString &_FileName1, bool _samBox=false) :
            Filler(_os, "AssemblyToRefDialog"),
            path(_path),
            fileName(_FileName),
            path1(_path1),
            fileName1(_FileName1),
            samBox(_samBox){}
        virtual void run();
    private:
        const QString path, fileName, path1, fileName1;
        bool samBox;
    };
}

#endif
