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

#ifndef _U2_ORF_ALG_TASK_H_
#define _U2_ORF_ALG_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Core/DNASequenceObject.h>

#include "ORFFinder.h"

#include <QtCore/QMutex>

namespace U2 {

class U2ALGORITHM_EXPORT ORFFindTask : public Task, public ORFFindResultsListener {
    Q_OBJECT
public:
    ORFFindTask(const ORFAlgorithmSettings& s,const U2EntityRef& entityRef);

    virtual void run();
    virtual void onResult(const ORFFindResult& r, U2OpStatus& oss);

    QList<ORFFindResult> popResults();

    const ORFAlgorithmSettings& getSettings() const {return config;}
private:
    ORFAlgorithmSettings config;
    U2EntityRef entityRef;
    QList<ORFFindResult> newResults;
    QMutex lock;
};


} //namespace

#endif
