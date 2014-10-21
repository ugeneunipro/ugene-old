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

#ifndef _U2_IN_SILICO_PCR_TASK_H_
#define _U2_IN_SILICO_PCR_TASK_H_

#include <U2Algorithm/FindAlgorithmTask.h>

#include <U2Core/GObjectReference.h>
#include <U2Core/Task.h>

namespace U2 {

class InSilicoPcrTaskSettings {
public:
    InSilicoPcrTaskSettings();

    QByteArray sequence;
    GObjectReference sequenceObject;
    bool isCircular;
    QByteArray forwardPrimer;
    QByteArray reversePrimer;
    uint forwardMismatches;
    uint reverseMismatches;
    uint maxProductSize;
};

class InSilicoPcrProduct {
public:
    InSilicoPcrProduct();

    U2Region region;
    double ta;

    QByteArray forwardPrimer;
    QByteArray reversePrimer;
    int forwardPimerMatchLength;
    int reversePimerMatchLength;
};

class InSilicoPcrTask : public Task {
    Q_OBJECT
public:
    InSilicoPcrTask(const InSilicoPcrTaskSettings &settings);

    // Task
    void prepare();
    void run();
    const QList<InSilicoPcrProduct> & getResults() const;
    const InSilicoPcrTaskSettings & getSettings() const;

private:
    qint64 getProductSize(const U2Region &left, const U2Region &right) const;
    FindAlgorithmTaskSettings getFindPatternSettings(bool forward);
    InSilicoPcrProduct createResult(const U2Region &region) const;

private:
    InSilicoPcrTaskSettings settings;
    FindAlgorithmTask *forwardSearch;
    FindAlgorithmTask *reverseSearch;
    QList<InSilicoPcrProduct> results;
};

} // U2

#endif // _U2_IN_SILICO_PCR_TASK_H_
