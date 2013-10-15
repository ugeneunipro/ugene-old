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

#ifndef _U2_GT_BUILD_INDEX_DIALOG_FILLER_H_
#define _U2_GT_BUILD_INDEX_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

class BuildIndexDialogFiller : public Filler {
public:
    BuildIndexDialogFiller(U2OpStatus &os,
                           const QString &_refPath,
                           const QString &_refFileName,
                           const QString &_method = "UGENE Genome Aligner",
                           bool _useDefaultIndexName = true,
                           const QString &_indPath = QString(),
                           const QString &_indFileName = QString()):
        Filler(os, "BuildIndexFromRefDialog"),
        refPath(_refPath),
        refFileName(_refFileName),
        method(_method),
        useDefaultIndexName(_useDefaultIndexName),
        indPath(_indPath),
        indFileName(_indFileName) {}
    virtual void run();
private:
    QString refPath;
    QString refFileName;
    QString indPath;
    QString indFileName;
    QString method;
    bool useDefaultIndexName;
};

}

#endif // _U2_GT_BUILD_INDEX_DIALOG_FILLER_H_
