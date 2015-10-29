/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GT_RUNNABLES_EXPORT_SEQUENCES_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_EXPORT_SEQUENCES_DIALOG_FILLER_H_

#include "utils/GTUtilsDialog.h"

namespace U2 {
using namespace HI;

    class ExportSelectedRegionFiller : public Filler {
    public:
        ExportSelectedRegionFiller(U2OpStatus &_os, const QString &_path, const QString &_name,
            bool translate = false, const QString& seqName = QString(), bool saveAllAminoFrames = true);
        ExportSelectedRegionFiller(U2OpStatus &os, CustomScenario *customScenario);

        void commonScenario();
        void setPath(const QString &value);
        void setName(const QString &value);

        static QString defaultExportPath;
    private:
        QString path, name;
        QString seqName;
        bool translate;
        bool saveAllAminoFrames;
    };

    class ExportSequenceOfSelectedAnnotationsFiller : public Filler {
    public:
        enum FormatToUse {Fasta, Fastq, Gff, Genbank};
        enum MergeOptions {SaveAsSeparate, Merge};
        ExportSequenceOfSelectedAnnotationsFiller(U2OpStatus &_os, const QString &_path, FormatToUse _format, MergeOptions _options, int _gapLength = 0,
                                                  bool _addDocToProject = true, bool _exportWithAnnotations = false, GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
    private:
        QString path;
        int gapLength;
        FormatToUse format;
        bool addToProject;
        bool exportWithAnnotations;
        MergeOptions options;
        GTGlobals::UseMethod useMethod;
        QMap<FormatToUse, QString> comboBoxItems;
        QMap<MergeOptions, QString> mergeRadioButtons;
    };

}

#endif
