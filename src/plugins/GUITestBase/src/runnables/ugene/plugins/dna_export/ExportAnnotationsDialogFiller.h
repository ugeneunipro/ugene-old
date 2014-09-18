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

#ifndef _U2_GT_RUNNABLES_EXPORT_ANNOTATIONS_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_EXPORT_ANNOTATIONS_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

    class ExportAnnotationsFiller : public Filler {
    public:
        enum fileFormat {bed, genbank, gff, csv};
        ExportAnnotationsFiller(const QString &exportToFile, fileFormat format, U2OpStatus &os);
        ExportAnnotationsFiller(U2OpStatus &_os, const QString &_exportToFile, fileFormat _format, bool _saveSequencesUnderAnnotations = true,
                                bool _saveSequenceNames = true, GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
    private:
        void init(const QString &exportToFile);
        bool softMode;
        QString exportToFile;
        fileFormat format;
        QMap<fileFormat, QString> comboBoxItems;
        bool saveSequencesUnderAnnotations;
        bool saveSequenceNames;

        GTGlobals::UseMethod useMethod;
    };

}

#endif
