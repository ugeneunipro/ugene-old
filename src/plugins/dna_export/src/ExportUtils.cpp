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

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "ExportSequencesDialog.h"
#include "ExportSequenceTask.h"
#include "ExportUtils.h"

namespace U2 {

void ExportUtils::loadDNAExportSettingsFromDlg(ExportSequenceTaskSettings& s, const ExportSequencesDialog& d)  {
    s.fileName = d.file;
    s.merge = d.merge;
    s.mergeGap = d.mergeGap;
    s.allAminoFrames = d.translateAllFrames;
    s.strand = d.strand;
    s.formatId = d.formatId;
    s.mostProbable = d.mostProbable;
    s.saveAnnotations = d.withAnnotations;
    s.sequenceName = d.sequenceName;
}

Task* ExportUtils::wrapExportTask(DocumentProviderTask* t, bool addToProject) {
    if (!addToProject) {
        return t;
    }
    return new AddExportedDocumentAndOpenViewTask(t);
}



QString ExportUtils::genUniqueName(const QSet<QString>& names, QString prefix) {
    if (!names.contains(prefix)) {
        return prefix;
    }
    QString name = prefix;
    int i=0;
    do {
        if (!names.contains(name)) {
            break;
        }
        name = prefix + "_" + QString::number(++i);
    } while(true);
    return name;
}

}//namespace
