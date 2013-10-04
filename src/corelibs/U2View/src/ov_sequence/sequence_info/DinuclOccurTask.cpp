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

#include "DinuclOccurTask.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>


namespace U2 {

DinuclOccurTask::DinuclOccurTask(const DNAAlphabet* _alphabet,
                                 const U2EntityRef _seqRef,
                                 U2Region _region)
    : BackgroundTask< QMap<QByteArray, qint64> >(
    "Calculating dinculeotides occurrence",
    TaskFlag_None),
      alphabet(_alphabet),
      seqRef(_seqRef),
      region(_region)
{
    tpm = Progress_Manual;
    stateInfo.setProgress(0);
}


void DinuclOccurTask::run()
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

    QMap<QByteArray, qint64> dinuclOccurrence;

    // Initializing
    foreach (char firstChar, alphabetChars) {
        foreach (char secondChar, alphabetChars) {
            if (('-' != firstChar) && ('-' != secondChar)) {
                QByteArray dinucl;
                dinucl.append(firstChar);
                dinucl.append(secondChar);

                dinuclOccurrence[dinucl] = 0;
            }
        }
    }

    int seqLength = sequenceDbi->getSequenceObject(seqRef.entityId, os).length;;
    CHECK_OP(os, );

    if (seqLength < 2) {
        return;
    }

    // If there are no 'N' characters in the sequence,
    // then corresponding dinucleotides wouldn't be shown
    bool thereAreNChars = false;


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

        // If this is not the first iteration, then we should take into account
        // the overlapped dinucleotide (i.e. the dinucleotide located on the border of two chunks)
        if (iterNum > 0) {
            iterRegion = U2Region(iterRegion.startPos - 1, iterRegion.length + 1);
        }

        // Get the selected region and verify that the data has been correctly read
        QByteArray sequence = sequenceDbi->getSequenceData(seqRef.entityId, iterRegion, os);

        if (os.hasError() || sequence.isEmpty()) {
            taskLog.details("Skipping calculation of the dinucleotides occurrence.");
            break;
        }


        const char* sequenceData = sequence.constData();

        // Verify the first character in the region
        char zeroChar  = sequenceData[0];
        if ('N' == zeroChar) {
            thereAreNChars = true;
        }
        SAFE_POINT(alphabetChars.contains(zeroChar),
            QString("Unexpected characters has been detected in the sequence: {%1}").arg(zeroChar),);

        // Calculating the values
        for (int i = 0, n = sequence.size(); i < n - 1; ++i) {

            char firstChar = sequence[i];
            char secondChar = sequence[i + 1];
            SAFE_POINT(alphabetChars.contains(secondChar),
                QString("Unexpected characters has been detected in the sequence: {%1}").arg(secondChar),);

            QByteArray currentDinucleotide;
            currentDinucleotide.append(firstChar);
            currentDinucleotide.append(secondChar);

            dinuclOccurrence[currentDinucleotide]++;

            if ('N' == secondChar) {
                thereAreNChars = true;
            }
        }

        // Update the task progress
        stateInfo.setProgress((region.length - wholeRegionLength) * 100 / region.length);
        CHECK_OP(stateInfo,);

        iterNum++;

    } while (wholeRegionLength != 0);

    // Remove dinucleotides containing 'N' characters if they are all equal to 0
    if (!thereAreNChars) {
        foreach (char character, alphabetChars) {
            QByteArray charNIsLeft;
            charNIsLeft.append('N');
            charNIsLeft.append(character);

            dinuclOccurrence.remove(charNIsLeft);

            QByteArray charNIsRight;
            charNIsRight.append(character);
            charNIsRight.append('N');

            dinuclOccurrence.remove(charNIsRight);
        }
    }

    result = dinuclOccurrence;
}



} // namespace
