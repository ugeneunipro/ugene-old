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

#include "PairwiseAlignmentTask.h"

namespace U2 {

const QString PairwiseAlignmentTaskSettings::PA_DEFAULT_NAME("PairwiseAlignmentResult.aln");
const QString PairwiseAlignmentTaskSettings::PA_RESULT_FILE_NAME("resultFileName");
const QString PairwiseAlignmentTaskSettings::PA_ALGIRITHM_NAME("algorithmName");
const QString PairwiseAlignmentTaskSettings::PA_REALIZATION_NAME("realizationName");
const QString PairwiseAlignmentTaskSettings::PA_FIRST_SEQUENCE_REF("firstSequenceRef");
const QString PairwiseAlignmentTaskSettings::PA_SECOND_SEQUENCE_REF("secondSequenceRef");
const QString PairwiseAlignmentTaskSettings::PA_IN_NEW_WINDOW("inNewWindow");
const QString PairwiseAlignmentTaskSettings::PA_ALPHABET("alphabet");

PairwiseAlignmentTaskSettings::PairwiseAlignmentTaskSettings() : inNewWindow(true) {
}

PairwiseAlignmentTaskSettings::PairwiseAlignmentTaskSettings(const QVariantMap& someSettings) : inNewWindow(true), customSettings(someSettings) {
}

PairwiseAlignmentTaskSettings::PairwiseAlignmentTaskSettings(const PairwiseAlignmentTaskSettings &s) :
    algorithmName(s.algorithmName),
    realizationName(s.realizationName),
    firstSequenceRef(s.firstSequenceRef),
    secondSequenceRef(s.secondSequenceRef),
    msaRef(s.msaRef),
    inNewWindow(s.inNewWindow),
    alphabet(s.alphabet),
    resultFileName(s.resultFileName),
    customSettings(s.customSettings) {
}

PairwiseAlignmentTaskSettings::~PairwiseAlignmentTaskSettings() {
}

QVariant PairwiseAlignmentTaskSettings::getCustomValue(const QString& optionName, const QVariant& defaultVal) const
{
    if (customSettings.contains(optionName)) {
        return customSettings.value(optionName);
    } else {
        return defaultVal;
    }
}

void PairwiseAlignmentTaskSettings::setCustomValue(const QString& optionName, const QVariant& val)
{
    customSettings.insert(optionName,val);
}

bool PairwiseAlignmentTaskSettings::convertCustomSettings() {
    if (customSettings.contains(PA_ALGIRITHM_NAME)) {
        algorithmName = customSettings.value(PA_ALGIRITHM_NAME).toString();
        customSettings.remove(PA_ALGIRITHM_NAME);
    }
    if (customSettings.contains(PA_REALIZATION_NAME)) {
        realizationName = customSettings.value(PA_REALIZATION_NAME).toString();
        customSettings.remove(PA_REALIZATION_NAME);
    }
    if (customSettings.contains(PA_RESULT_FILE_NAME)) {
        if (customSettings.value(PA_RESULT_FILE_NAME).type() == QVariant::String) {
            resultFileName = GUrl(customSettings.value(PA_RESULT_FILE_NAME).toString());
            customSettings.remove(PA_RESULT_FILE_NAME);
        }
    }
    if (customSettings.contains(PA_IN_NEW_WINDOW)) {
        inNewWindow = customSettings.value(PA_IN_NEW_WINDOW).toBool();
        customSettings.remove(PA_IN_NEW_WINDOW);
    }
    return true;
}

bool PairwiseAlignmentTaskSettings::containsValue(const QString& optionName) const {
    return customSettings.contains(optionName);
}

void PairwiseAlignmentTaskSettings::setResultFileName(QString newFileName) {
    resultFileName = GUrl(newFileName);
}

void PairwiseAlignmentTaskSettings::setCustomSettings(const QMap<QString, QVariant>& settings)
{
    customSettings = settings;
}

void PairwiseAlignmentTaskSettings::appendCustomSettings(const QMap<QString, QVariant>& settings) {
    foreach (QString key, settings.keys()) {
        customSettings.insert(key, settings.value(key));
    }
}

bool PairwiseAlignmentTaskSettings::isValid() const {
    return firstSequenceRef.isValid() && secondSequenceRef.isValid() &&
            msaRef.isValid() && alphabet.isValid() &&
            (!resultFileName.isEmpty() || inNewWindow == false);
}


PairwiseAlignmentTask::PairwiseAlignmentTask(TaskFlags flags) :
    Task("PairwiseAlignmentTask", flags) {
}

PairwiseAlignmentTask::~PairwiseAlignmentTask() {
}

PairwiseAlignmentTaskFactory::~PairwiseAlignmentTaskFactory() {}

}   //namespace
