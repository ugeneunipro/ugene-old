/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_FORMAT_DB_DIALOG_FILLER_H_
#define _U2_FORMAT_DB_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"
#include "api/GTFileDialog.h"

namespace U2 {

class FormatDBSupportRunDialogFiller : public Filler {
public:
    class Parameters {
    public:
        Parameters():
            justCancel(false), checkAlphabetType(false), customFiller_3551(false)
        {

        }
        enum Type {Nucleotide, Protein};

        bool justCancel;
        bool checkAlphabetType;
        QString inputFilePath;
        Type alphabetType;
        QString outputDirPath;

        bool customFiller_3551;
    };


    FormatDBSupportRunDialogFiller(U2OpStatus &os, const Parameters &parameters);
    void run();

private:
    Parameters parameters;
};

} // U2

#endif // _U2_FORMAT_DB_DIALOG_FILLER_H_
