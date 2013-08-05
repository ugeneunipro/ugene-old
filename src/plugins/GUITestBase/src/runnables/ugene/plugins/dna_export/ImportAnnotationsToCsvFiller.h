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

#ifndef _U2_GT_RUNNABLES_IMPORT_ANNOTATIONS_TO_CSV_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_IMPORT_ANNOTATIONS_TO_CSV_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

    class ImportAnnotationsToCsvFiller : public Filler {
    public:
        struct RoleParameter {
            virtual void ke(){}
        };

        enum fileFormat {BED, EMBL, FPKM, GFF, GTF, Genbank, Swiss_Prot};
        struct RoleColumnParameter {
            RoleColumnParameter(int _column, RoleParameter* _p) : column(_column), parameter(_p){}
            int column;
            RoleParameter* parameter;
        };
        typedef QList<RoleColumnParameter> RoleParameters;

        struct StartParameter : public RoleParameter {
            StartParameter(bool _addOffset, int _numberOfBp=0) : addOffset(_addOffset), numberOfBp(_numberOfBp){}
            bool addOffset;
            int numberOfBp;
        };

        struct EndParameter : public RoleParameter {
            EndParameter(bool _endPos) : endPos(_endPos){}
            bool endPos;
        };
        
        struct LengthParameter : public RoleParameter {
        };
        
        struct StrandMarkParameter : public RoleParameter {
            StrandMarkParameter(bool _markValue, QString _markValueName) : markValue(_markValue), markValueName(_markValueName){}
            bool markValue;
            QString markValueName;
        };

        struct IgnoreParameter : public RoleParameter {
        };

        struct NameParameter : public RoleParameter {
        };

        struct QualifierParameter : public RoleParameter {
            QualifierParameter(const QString& _name) : name(_name){}
            QString name;
        };
        ImportAnnotationsToCsvFiller(U2OpStatus &_os, const QString &_fileToRead, const QString &_resultFile,
                                        ImportAnnotationsToCsvFiller::fileFormat _format, bool _addResultFileToProject, 
                                        bool _columnSeparator, const QString &_separator,
                                        bool _script, int _numberOfLines,
                                        const QString &_skipAllLinesStartsWith, bool _interpretMultipleAsSingle,
                                        bool _removeQuotesButton, const QString &_defaultAnnotationName, const RoleParameters& roleParameters = RoleParameters(), GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
    private:
        QString fileToRead, resultFile;
        fileFormat format;
        bool addResultFileToProject, columnSeparator, script;
        int numberOfLines;
        QString separator, skipAllLinesStartsWith;
        bool interpretMultipleAsSingle, removeQuotesButton;
        QString defaultAnnotationName;
        QMap<fileFormat, QString> comboBoxItems;
        RoleParameters roleParameters;
        GTGlobals::UseMethod useMethod;
    };
}

#endif