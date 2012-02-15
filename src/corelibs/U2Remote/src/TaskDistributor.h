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


#ifndef _U2_TASK_DISTRIBUTOR_H_
#define _U2_TASK_DISTRIBUTOR_H_

#include <QtCore/QList>
#include "LocalTask.h"

namespace U2 {

/*
 * Base class. Each algorithm needs its own distributor
 */
class U2REMOTE_EXPORT TaskDistributor {
public:
    virtual ~TaskDistributor(){}
    /*scatters task settings to some number of parts
      returns new allocated settings */
    virtual QList<LocalTaskSettings *> scatter( const LocalTaskSettings * settings)const = 0;
    /* returns new allocated results */
    virtual LocalTaskResult * gather(const QList<LocalTaskResult *> &results)const = 0;
    
}; // TaskDistributor

/*
 * Template to TaskDistributor. Makes it easier to write own TaskDistributor implementation.
 */
template<class SettingsT, class ResultT>
class U2REMOTE_EXPORT TaskDistributorTemplate : public TaskDistributor {
public:
    virtual QList<LocalTaskSettings *> scatter( LocalTaskSettings * settings)const
    {
        SettingsT *castedSettings = dynamic_cast<SettingsT *>(settings);
        if(NULL == castedSettings)
        {
            return QList<LocalTaskSettings *>();
        }
        QList<LocalTaskSettings *> settingsList;
        foreach(SettingsT *settings, scatter(castedSettings))
        {
            settingsList.append(settings);
        }
        return settingsList;
    }
    virtual LocalTaskResult *gather(const QList<LocalTaskResult *> &results)const
    {
        QList<ResultT *> castedResults;
        foreach(LocalTaskResult *result, results)
        {
            ResultT *castedResult = dynamic_cast<ResultT *>(result);
            if(NULL == castedResult)
            {
                return NULL;
            }
            castedResults.append(castedResult);
        }
        return gather(castedResults);
    }

    virtual QList<SettingsT *> scatter(const SettingsT *settings)const = 0;
    virtual ResultT *gather(const QList<const ResultT *> &results)const = 0;
    
}; // TaskDistributorTemplate

} // U2

#endif // _U2_TASK_DISTRIBUTOR_H_
