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

#include <QtCore/QDir>

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineRegistry.h>

#include "GenomeAlignerTask.h"
#include "GenomeAlignerCMDLineTask.h"

namespace U2 {

#define OPTION_INDEX_PATH   "index"
#define OPTION_SHORTREADS   "short-reads"
#define OPTION_RESULT       "result"
#define OPTION_BUILD_INDEX  "build-index"
#define OPTION_REFERENCE    "reference"

GenomeAlignerCMDLineTask::GenomeAlignerCMDLineTask()
:Task( tr( "Run genome aligner from command line" ), TaskFlags_NR_FOSCOE), onlyBuildIndex(false)
{
    // parse options
    
    QList<StringPair> options = AppContext::getCMDLineRegistry()->getParameters();
    
    foreach (const StringPair& opt, options ) {
        if (opt.first == OPTION_INDEX_PATH  ) {
            indexPath = opt.second;
        }else if (opt.first == OPTION_BUILD_INDEX ) {
            onlyBuildIndex = true;    
        } else if (opt.first == OPTION_REFERENCE) {
            refPath = opt.second;
        } else if (opt.first == OPTION_RESULT) {
            resultPath = opt.second;
        } else if (opt.first == OPTION_SHORTREADS) {
            QStringList urls = opt.second.split(";");
            foreach(const QString& url, urls) {
                shortReadUrls.append(url);
            }
        }
            
    }
        
    coreLog.info( tr( "Finished parsing genome aligner options." ) );

}

GenomeAlignerCMDLineTask::~GenomeAlignerCMDLineTask()
{
    // clean up resources
}

void GenomeAlignerCMDLineTask::prepare()
{
    if (onlyBuildIndex && refPath.isEmpty()) {
        setError(tr("Path to reference sequence is not set."));
        return;
    }
        
    if (!onlyBuildIndex) {
        if ( shortReadUrls.isEmpty() ) {
            setError(tr("Short reads list is empty."));
            return;
        } else {
            if (refPath.isEmpty() && indexPath.isEmpty()) {
                setError(tr("Reference (index or sequence) is not set."));
                return;
            }
        }
    }
     
    if (resultPath.isEmpty()) {
        resultPath =  QDir::current().path() + "/output.sam";
    }

    settings.resultFileName = resultPath;
    settings.shortReadUrls = shortReadUrls;
    settings.refSeqUrl = refPath;
    settings.indexFileName = indexPath;

    GenomeAlignerTask* task = new GenomeAlignerTask(settings, onlyBuildIndex);
    addSubTask(task);

}


}//namespace

