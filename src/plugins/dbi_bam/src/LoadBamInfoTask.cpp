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

#include <U2Core/Task.h>
#include <U2Core/AppContext.h>

#include <memory>
#include "Reader.h"
#include "Index.h"
#include "BaiReader.h"
#include "LoadBamInfoTask.h"
#include "Exception.h"

namespace U2 {
namespace BAM {

LoadBamInfoTask::LoadBamInfoTask(const GUrl& _sourceUrl)
    : Task(tr("Load BAM info"), TaskFlag_None), sourceUrl(_sourceUrl)
{

}

const GUrl& LoadBamInfoTask::getSourceUrl() const {
    return sourceUrl;
}

void LoadBamInfoTask::run() {        
    try {
        std::auto_ptr<IOAdapter> ioAdapter;
        {
            IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(sourceUrl));
            ioAdapter.reset(factory->createIOAdapter());
        }

        GUrl baiUrl(sourceUrl.getURLString() + ".bai");
        bool hasIndex = true;
        std::auto_ptr<IOAdapter> ioIndexAdapter;
        {
            IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(baiUrl));
            ioIndexAdapter.reset(factory->createIOAdapter());
        }

        if(!ioAdapter->open(sourceUrl, IOAdapterMode_Read)) {
            stateInfo.setError(LoadBamInfoTask::tr("Can't open file '%1'").arg(sourceUrl.getURLString()));
            return;
        }

        if(!ioIndexAdapter->open(baiUrl, IOAdapterMode_Read)) {
            hasIndex = false;
        }

        std::auto_ptr<Reader> reader(new Reader(*ioAdapter));
        std::auto_ptr<BaiReader> baiReader(new BaiReader(*ioIndexAdapter));
        
        Index index;
        if(hasIndex) {
            index = baiReader->readIndex();
            bamInfo.setIndex(index);
        }

        bamInfo.setHeader(reader->getHeader());
    } catch(const Exception &ex) {
        stateInfo.setError(ex.getMessage());
    }
}

} // namespace BAM
} // namespace U2
