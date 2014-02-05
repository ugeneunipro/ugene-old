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

#ifndef _U2_DNA_ASSEMBLEY_UTILS_H_
#define _U2_DNA_ASSEMBLEY_UTILS_H_

#include <U2Algorithm/DnaAssemblyMultiTask.h>

namespace U2 {

class DnaAssemblyToRefTaskSettings;

class U2VIEW_EXPORT DnaAssemblySupport : public QObject {
    Q_OBJECT
public:
    DnaAssemblySupport();

    static QMap<QString, QString> toConvert(const DnaAssemblyToRefTaskSettings &settings, QList<GUrl> &unknownFormatFiles);
    static QString toConvertText(const QMap<QString, QString> &files);
    static QString unknownText(const QList<GUrl> &unknownFormatFiles);

private slots:
    void sl_showDnaAssemblyDialog();
    void sl_showBuildIndexDialog();
    void sl_showConvertToSamDialog();

};

class U2VIEW_EXPORT DnaAssemblyTaskWithConversions : public Task {
public:
    DnaAssemblyTaskWithConversions(const DnaAssemblyToRefTaskSettings &settings, bool viewResult = false, bool justBuildIndex = false);

    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);

private:
    DnaAssemblyToRefTaskSettings settings;
    bool viewResult;
    bool justBuildIndex;
    int conversionTasksCount;
    DnaAssemblyMultiTask *assemblyTask;
};

} // U2


#endif // _U2_DNA_ASSEMBLEY_TASK_H_
