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

#ifndef EXPORT_SELECTED_SEQUENCE_FROM_ALIANMENT_DIALOG_FILLER_H
#define EXPORT_SELECTED_SEQUENCE_FROM_ALIANMENT_DIALOG_FILLER_H

#include "GTUtilsDialog.h"

namespace U2 {

    class ExportSelectedSequenceFromAlignment : public Filler {
    public:
        enum documentFormat {EMBL, FASTA, FASTQ, GFF, Genbank, Swiss_Prot};
        ExportSelectedSequenceFromAlignment(U2OpStatus &_os,QString _path,
                                            documentFormat _format=FASTA,bool _keepGaps=true,bool _addToProj=true);
        virtual void run();
    private:
        QString path;
        documentFormat format;
        QMap<documentFormat, QString> comboBoxItems;
        bool keepGaps,addToProj;
    };
}

#endif //EXPORT_SELECTED_SEQUENCE_FROM_ALIANMENT_DIALOG_FILLER_H
