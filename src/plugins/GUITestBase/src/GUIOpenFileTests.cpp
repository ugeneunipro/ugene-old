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

#include "GUIOpenFileTests.h"
#include "QtUtils.h"
#include "ProjectUtils.h"
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>
#include "LogUtils.h"
#include "DocumentUtils.h"

namespace U2 {

void OpenFastaFile::execute(U2OpStatus &os) {
    GUrl url(testDir + "_common_data/fasta/fa1.fa");

    ProjectUtils::openFile(os, url);
}

void OpenMultipleFiles::execute(U2OpStatus &os) {

    QList<QUrl> urls;
    urls << QUrl(dataDir+"samples/Genbank/murine.gb");
    urls << QUrl(dataDir+"samples/Genbank/sars.gb");
    urls << QUrl(dataDir+"samples/Genbank/CVU55762.gb");
    ProjectUtils::openFiles(os, urls);

    DocumentUtils::checkDocumentExists(os, "murine.gb");
    DocumentUtils::checkDocumentExists(os, "sars.gb");
    DocumentUtils::checkDocumentExists(os, "CVU55762.gb");
}

void OpenCloseFastaFile::execute(U2OpStatus &os)
{
    LogTracer log;
    GUrl url(dataDir + "samples/FASTA/human_T1.fa");

    ProjectUtils::openFile(os, url);

    ProjectUtils::CloseProjectSettings button_to_press;
    button_to_press.saveOnClose = ProjectUtils::CloseProjectSettings::CANCEL;

    ProjectUtils::closeProject(os, button_to_press);

    LogUtils::checkHasError(os, log);
}

} // namespace
