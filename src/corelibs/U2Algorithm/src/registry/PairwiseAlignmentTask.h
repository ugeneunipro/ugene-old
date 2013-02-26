/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PAIRWISE_ALIGNMENT_TASK_H_
#define _U2_PAIRWISE_ALIGNMENT_TASK_H_

#include <U2Core/global.h>
#include <U2Core/U2Type.h>
#include <U2Core/Task.h>
#include <U2Core/GUrl.h>
#include <U2Core/U2Alphabet.h>

#include <U2Algorithm/PairwiseAlignmentRegistry.h>

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

namespace U2 {

class PairwiseAlignmentSettings;
class PairwiseAlignmentTask;
class PairwiseAlignmentTaskFactory;

#define PA_DEFAULT_NAME "DefaultName"
#define PA_RESULT_FILE_NAME "resultFileName"
#define PA_ALGIRITHM_NAME "algorithmName"
#define PA_REALIZATION_NAME "realizationName"
#define PA_FIRST_SEQUENCE_REF "firstSequenceRef"
#define PA_SECOND_SEQUENCE_REF "secondSequenceRef"
#define PA_IN_NEW_WINDOW "inNewWindow"
#define PA_ALPHABET "alphabet"

class U2ALGORITHM_EXPORT PairwiseAlignmentTaskSettings : public QObject {
    Q_OBJECT

public:
    PairwiseAlignmentTaskSettings();
    PairwiseAlignmentTaskSettings(const QVariantMap& someSettings);
    PairwiseAlignmentTaskSettings(const PairwiseAlignmentTaskSettings &s);
    virtual ~PairwiseAlignmentTaskSettings();

    void setCustomSettings(const QVariantMap& settings);
    void appendCustomSettings(const QVariantMap& settings);
    QVariant getCustomValue(const QString& optionName, const QVariant& defaultVal) const;
    void setCustomValue(const QString& optionName, const QVariant& val);
    virtual bool convertCustomSettings();
    bool containsValue(const QString& optionName) const;
    void setResultFileName(QString newFileName);
    virtual bool isValid() const;

public:
    QString algorithmName;
    QString realizationName;
    U2EntityRef firstSequenceRef;           //could`n be in customSettings. Set manually.
    U2EntityRef secondSequenceRef;          //could`n be in customSettings. Set manually.
    U2EntityRef msaRef;                     //could`n be in customSettings. Set manually.
    bool inNewWindow;
    U2AlphabetId alphabet;                  //could`n be in customSettings. Set manually.
    GUrl resultFileName;                    //could be in customSettings as a string only.

protected:
    QVariantMap customSettings;
};

class U2ALGORITHM_EXPORT PairwiseAlignmentTask : public Task {
    Q_OBJECT

public:
    PairwiseAlignmentTask(TaskFlags flags = TaskFlags_FOSCOE);
    virtual ~PairwiseAlignmentTask();

protected:
    QByteArray first;
    QByteArray second;
};

class U2ALGORITHM_EXPORT PairwiseAlignmentTaskFactory {
public:
    virtual PairwiseAlignmentTask* getTaskInstance(PairwiseAlignmentTaskSettings* settings) const = 0;
    virtual ~PairwiseAlignmentTaskFactory();

};

}   //namespace

#endif // _U2_PAIRWISE_ALIGNMENT_TASK_H_
