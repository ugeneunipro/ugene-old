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

#ifndef _U2_GT_RUNNABLES_CREATE_DOCUMENT_FROM_TEST_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_CREATE_DOCUMENT_FROM_TEST_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

    class CreateDocumentFiller : public Filler {
    public:
        enum documentFormat {FASTA, Genbank};
        enum documentAlphabet {StandardDNA, StandardRNA, ExtendedDNA, ExtendedRNA, StandardAmino, AllSymbols};

        CreateDocumentFiller(U2OpStatus &_os, const QString &_pasteDataHere, bool customSettings, documentAlphabet _alphabet, bool skipUnknownSymbols,
                             bool replaceUnknownSymbols, const QString symbol, const QString &_documentLocation,
                             documentFormat _format, const QString &_sequenceName, bool saveFile, GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
    private:

        QString pasteDataHere;
        bool customSettings;
        documentAlphabet alphabet;
        bool skipUnknownSymbols;
        bool replaceUnknownSymbols;
        QString symbol;
        QMap<documentAlphabet, QString> comboBoxAlphabetItems;
        QString documentLocation;
        documentFormat format;
        QMap<documentFormat, QString> comboBoxItems;
        QString sequenceName;
        bool saveFile;
        GTGlobals::UseMethod useMethod;
    };

}

#endif