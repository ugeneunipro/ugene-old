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
        AlignShortReadsFiller(U2OpStatus &os,
            const QString &path,
            const QString &FileName,
            const QString &path1,
            const QString &FileName1,
            bool samBox = true,
            bool prebuilt = false,
            const QString &method = "UGENE Genome Aligner") :
            Filler(os, "AssemblyToRefDialog"),
            path(path),
            fileName(FileName),
            path1(path1),
            fileName1(FileName1),
            method(method),
            samBox(samBox),
            prebuilt(prebuilt){}
        virtual void run();
    private:
        QString path;
        QString fileName;
        QString path1;
        QString fileName1;
        QString method;
        bool samBox;
        bool prebuilt;
    };
}

#endif
