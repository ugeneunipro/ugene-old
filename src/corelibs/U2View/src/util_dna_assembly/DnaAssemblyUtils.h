/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DNA_ASSEMBLEY_UTILS_H_
#define _U2_DNA_ASSEMBLEY_UTILS_H_

#include <U2Core/global.h>

namespace U2 {

class DnaAssemblyToRefTaskSettings;

class U2VIEW_EXPORT DnaAssemblySupport : public QObject {
    Q_OBJECT
public:
    DnaAssemblySupport();

private slots:
    void sl_showDnaAssemblyDialog();
    void sl_showBuildIndexDialog();
    void sl_showConvertToSamDialog();

}; 

// class DnaAssemblyLoadShortReadsTask : public Task {
//     Q_OBJECT
// public:
//     DnaAssemblyLoadShortReadsTask(DnaAssemblyToRefTaskSettings& settings, const QList<GUrl>& shortReads);
//     virtual void prepare();
//     QList<Task*> onSubTaskFinished(Task* subTask);
//     QList<DN
//     virtual ReportResult report();
// private:
//     DnaAssemblyToRefTaskSettings& settings;
//     const QList<GUrl>& shortReads;
// }; 

} // U2


#endif // _U2_DNA_ASSEMBLEY_TASK_H_
