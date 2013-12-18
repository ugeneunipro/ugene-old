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

#ifndef _U2_FIND_PATTERN_TASK_H_
#define _U2_FIND_PATTERN_TASK_H_

#include <QtCore/QPointer>

#include <U2Algorithm/FindAlgorithmTask.h>

#include <U2Core/AnnotationData.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/Task.h>

namespace U2 {

class U2VIEW_EXPORT FindPatternTask : public Task
{
    Q_OBJECT

public:
    FindPatternTask(const FindAlgorithmTaskSettings& settings,
        AnnotationTableObject* annotObject,
        const QString& annotName,
        const QString& annotGroup,
        bool removeOverlaps);

    QList<Task*> onSubTaskFinished(Task* subTask);

private:
    void removeOverlappedResults(QList<FindAlgorithmResult>& results);

    FindAlgorithmTaskSettings           settings;
    QPointer<AnnotationTableObject>       annotObject;
    QString                             annotName;
    QString                             annotGroup;
    bool                                removeOverlaps;
    FindAlgorithmTask*                  findAlgorithmTask;

    static const float MAX_OVERLAP_K;
};

} // namespace

#endif

