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

#ifndef _U2_MSA_ALIGN_TASK_H_
#define _U2_MSA_ALIGN_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/GUrl.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequence.h>
#include <U2Core/UIndex.h>

namespace U2 {

class Document;

class U2ALGORITHM_EXPORT MSAAlignTaskSettings {
public:
    MSAAlignTaskSettings() : loadResultDocument(false), useAminoMode(false) {}
    GUrl resultFileName;
    QString algName;
    bool loadResultDocument;
    bool useAminoMode;
    void setCustomSettings(const QMap<QString, QVariant>& settings);
    QVariant getCustomValue(const QString& optionName, const QVariant& defaultVal) const;
    void setCustomValue(const QString& optionName, const QVariant& val);
private:
    QMap<QString, QVariant> customSettings;
};


class U2ALGORITHM_EXPORT MSAAlignTask : public Task {
    Q_OBJECT
public:
    MSAAlignTask(MAlignmentObject* obj, const MSAAlignTaskSettings& settings, TaskFlags flags = TaskFlags_FOSCOE);
    virtual ~MSAAlignTask() {}
protected:
    const MSAAlignTaskSettings& settings;
    MAlignmentObject* obj;
};


class U2ALGORITHM_EXPORT MSAAlignTaskFactory {
public:
    virtual MSAAlignTask* createTaskInstance(MAlignmentObject* obj, const MSAAlignTaskSettings& settings) = 0;
    virtual ~MSAAlignTaskFactory() {}
};

#define MSA_ALIGN_TASK_FACTORY(c) \
public: \
    static const QString taskName; \
class Factory : public MSAAlignTaskFactory { \
public: \
    Factory() { } \
    MSAAlignTask* createTaskInstance(MAlignmentObject* obj, const MSAAlignTaskSettings& s) { return new c(obj, s); } \
};

} // U2


#endif // _U2_MSA_ALIGN_TASK_H_
