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

#include "CharOccurTask.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>


namespace U2 {


CharOccurResult::CharOccurResult(char _charInSequence, qint64 _numberOfOccurrence, double _percentageOfOccur)
    : charInSequence(_charInSequence),
      numberOfOccurr(_numberOfOccurrence),
      percentageOfOccur(_percentageOfOccur)
{
}


CharOccurTask::CharOccurTask(const DNAAlphabet* _alphabet,
                             U2EntityRef _seqRef,
                             U2Region _region)
    : BackgroundTask< QList<CharOccurResult> >(
    "Calculating characters occurrence",
    TaskFlag_None),
      alphabet(_alphabet),
      seqRef(_seqRef),
      region(_region)
{
    tpm = Progress_Manual;
    stateInfo.setProgress(0);
}


void CharOccurTask::run()
{
    // Create the connection
    U2OpStatus2Log os;
    DbiConnection dbiConnection(seqRef.dbiRef, os);
    CHECK_OP(os, );

    U2SequenceDbi* sequenceDbi = dbiConnection.dbi->getSequenceDbi();

    // Verify the alphabet
    SAFE_POINT(0 != alphabet, "The alphabet is NULL!",)

    QByteArray alphabetChars = alphabet->getAlphabetChars();
    SAFE_POINT(!alphabetChars.isEmpty(), "There are no characters in the alphabet!",);

    QMap<char, qint64> charactersOccurrence;

    // Initializing
    foreach (char character, alphabetChars) {
        if ('-' != character) { // skip all gaps
            charactersOccurrence[character] = 0;
        }
    }

    // If the input region length is more than REGION_TO_ANALAYZE,
    // divide the analysis into iterations
    int iterNum = 0;
    qint64 wholeRegionLength = region.length;
    do
    {
        U2Region iterRegion;

        if (wholeRegionLength <= REGION_TO_ANALAYZE) {
            iterRegion = U2Region(region.startPos + iterNum * REGION_TO_ANALAYZE, wholeRegionLength);
            wholeRegionLength = 0;
        } else {
            iterRegion = U2Region(region.startPos + iterNum * REGION_TO_ANALAYZE, REGION_TO_ANALAYZE);
            wholeRegionLength -= REGION_TO_ANALAYZE;
        }
        
        // Get the selected region and verify that the data has been correctly read
        QByteArray sequence = sequenceDbi->getSequenceData(seqRef.entityId, iterRegion, os);
        qint64 test = sequence.length();

        if (os.hasError() || sequence.isEmpty()) {
            taskLog.details("Skipping calculation of the characters occurrence.");
            break;
        }

        // Calculating the values
        const char* sequenceData = sequence.constData();
        for (int i = 0, n = sequence.size(); i < n; i++) {
            char character = sequenceData[i];
            SAFE_POINT(alphabetChars.contains(character),
                QString("Unexpected characters has been detected in the sequence: {%1}").arg(character),);

            charactersOccurrence[character]++;
        }

        // Update the task progress
        stateInfo.setProgress((region.length - wholeRegionLength) * 100 / region.length);
        CHECK_OP(stateInfo, );

        iterNum++;
    } while (wholeRegionLength != 0);


    // If it is a standard alphabet and there are no 'N' characters in the sequence,
    // then do not output them ('N' = 0)
    QString alphabetId = alphabet->getId();

    if ((alphabetId == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()) ||
        (alphabetId == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT()))
    {
        if (charactersOccurrence['N'] == 0) {
            charactersOccurrence.remove('N');
        }
    }

    // Calculate the percentage and format the result
    QList<CharOccurResult> calculatedResults;
    QMap<char, qint64>::const_iterator i = charactersOccurrence.constBegin();
    while (i != charactersOccurrence.constEnd()) {
        char charInSequence = i.key();
        qint64 numberOfOccur = i.value();
        double percentageOfOccur = numberOfOccur * 100.0 / region.length;
        CharOccurResult calcResult =
            CharOccurResult(charInSequence, numberOfOccur, percentageOfOccur);
        calculatedResults.append(calcResult);

        ++i;
    }

    result = calculatedResults;
}



} // namespace
