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
    uint perfectMatch;
};

class InSilicoPcrProduct {
public:
    InSilicoPcrProduct();

    /* Region within the original sequence */
    U2Region region;
    /* Annealing temperature */
    double ta;
    /* Primer sequence */
    QByteArray forwardPrimer;
    /* Primer sequence */
    QByteArray reversePrimer;
    /* The length of found primer region within the original sequence */
    int forwardPrimerMatchLength;
    /* The length of found primer region within the original sequence */
    int reversePrimerMatchLength;
};

class InSilicoPcrTask : public Task {
    Q_OBJECT
public:
    InSilicoPcrTask(const InSilicoPcrTaskSettings &settings);

    // Task
    void prepare();
    void run();
    QString generateReport() const;

    const QList<InSilicoPcrProduct> & getResults() const;
    const InSilicoPcrTaskSettings & getSettings() const;

private:
    class PrimerBind {
    public:
        QByteArray primer;
        uint mismatches;
        U2Region region;
    };
    PrimerBind getPrimerBind(const FindAlgorithmResult &forward, const FindAlgorithmResult &reverse, U2Strand::Direction direction) const;

    qint64 getProductSize(const U2Region &left, const U2Region &right) const;
    FindAlgorithmTaskSettings getFindPatternSettings(U2Strand::Direction direction);
    bool isCorrectProductSize(qint64 productSize, qint64 minPrimerSize) const;
    bool filter(const PrimerBind &leftBind, const PrimerBind &rightBind, qint64 productSize) const;
    bool checkPerfectMatch(const U2Region &region, QByteArray primer, U2Strand::Direction direction) const;
    QByteArray getSequence(const U2Region &region, U2Strand::Direction direction) const;
    InSilicoPcrProduct createResult(const U2Region &leftPrimer, const U2Region &product, const U2Region &rightPrimer, U2Strand::Direction direction) const;

private:
    InSilicoPcrTaskSettings settings;
    FindAlgorithmTask *forwardSearch;
    FindAlgorithmTask *reverseSearch;
    QList<InSilicoPcrProduct> results;
    int minProductSize;
};

} // U2

Q_DECLARE_METATYPE(U2::InSilicoPcrProduct)

#endif // _U2_IN_SILICO_PCR_TASK_H_
