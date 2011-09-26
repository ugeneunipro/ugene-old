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

#include "DNAFlexGraphAlgorithm.h"
#include "FindHighFlexRegionsAlgorithm.h"


namespace U2 {


DNAFlexGraphAlgorithm::DNAFlexGraphAlgorithm()
{
}


DNAFlexGraphAlgorithm::~DNAFlexGraphAlgorithm()
{
}


/**
 * Calculates data for a DNA Flexibility graph
 *
 * @param result Points of the graph
 * @param sequenceObject The sequence used to draw the graph
 * @param region The region of the sequence to use
 * @param windowData Current parameters of the graph (window, step, etc.)
 */
void DNAFlexGraphAlgorithm::calculate(
    QVector<float>& result,
    DNASequenceObject* sequenceObject,
    const U2Region& region,
    const GSequenceGraphWindowData* windowData)
{
    assert(windowData !=NULL);

    const QByteArray& sequence = sequenceObject->getSequence();

    int windowSize = windowData->window;
    int windowStep = windowData->step;

    // Variables
    int windowLeft = region.startPos;
    float windowThreshold = 0.0;

    // Getting the number of steps
    int stepsNumber = GSequenceGraphUtils::getNumSteps(region, windowData->window, windowData->step);

    // Allocating memory for the results
    result.reserve(stepsNumber);

    // Calculating the results
    for (int i = 0; i < stepsNumber; ++i)
    {
        // Calculating the threshold in the current window
        windowThreshold = 0;
        for (int j = windowLeft; j < windowLeft + windowSize - 1; ++j)
        {
            windowThreshold += FindHighFlexRegionsAlgorithm::flexibilityAngle(sequence[j], sequence[j + 1]);
        }
        windowThreshold /= (windowSize - 1);


        // Returning the point on the graph
        result.append(windowThreshold);

        // Enlarging the left position to the step
        windowLeft += windowStep;
    }
}


} // namespace
