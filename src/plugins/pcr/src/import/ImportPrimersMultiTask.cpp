/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2Core/L10n.h>

#include "ImportPrimersMultiTask.h"

namespace U2 {

ImportPrimersMultiTask::ImportPrimersMultiTask(const QList<Task *> &importSubtasks) :
    MultiTask(tr("Import primers"), importSubtasks, false,
              TaskFlags(TaskFlag_NoRun | TaskFlag_ReportingIsEnabled | TaskFlag_ReportingIsSupported)) {
}

QString ImportPrimersMultiTask::generateReport() const {
    QString report = "<hr><br>";
    foreach (Task *subtask, getSubtasks()) {
        report += subtask->generateReport() + "<br>";
    }
    return report;
}

}   // namespace U2
