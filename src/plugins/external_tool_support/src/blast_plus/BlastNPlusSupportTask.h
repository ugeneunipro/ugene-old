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

#ifndef _U2_BLASTNPLUS_SUPPORT_TASK_H
#define _U2_BLASTNPLUS_SUPPORT_TASK_H

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/AnnotationData.h>


#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include "utils/ExportTasks.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>

#include "ExternalToolRunTask.h"
#include "BlastPlusSupportCommonTask.h"

#include <QtXml/QDomNode>
#include <QtXml/QDomDocument>

namespace U2 {

class BlastNPlusSupportTask : public BlastPlusSupportCommonTask {
    Q_OBJECT
public:
    BlastNPlusSupportTask(const BlastTaskSettings& settings):BlastPlusSupportCommonTask(settings){}
    virtual ExternalToolRunTask* createBlastPlusTask();
};

}//namespace
#endif // _U2_BLASTNPLUS_SUPPORT_TASK_H
