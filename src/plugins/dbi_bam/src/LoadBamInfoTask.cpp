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
#include "SamReader.h"

namespace U2 {
namespace BAM {

LoadInfoTask::LoadInfoTask(const GUrl& _sourceUrl, bool _sam)
    : Task(tr("Load BAM info"), TaskFlag_None), sourceUrl(_sourceUrl), sam(_sam)
{

}

const GUrl& LoadInfoTask::getSourceUrl() const {
    return sourceUrl;
}

void LoadInfoTask::run() {        
    try {
        std::auto_ptr<IOAdapter> ioAdapter;
        {
            IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(sourceUrl));
            ioAdapter.reset(factory->createIOAdapter());
        }

        GUrl baiUrl(sourceUrl.getURLString() + ".bai");
        bool hasIndex = true;
        std::auto_ptr<IOAdapter> ioIndexAdapter;
        IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(baiUrl));
        ioIndexAdapter.reset(factory->createIOAdapter());

        if(!ioAdapter->open(sourceUrl, IOAdapterMode_Read)) {
            stateInfo.setError(LoadInfoTask::tr("Can't open file '%1'").arg(sourceUrl.getURLString()));
            return;
        }

        if (sam) {
            hasIndex = false;
        } else {
            if(!ioIndexAdapter->open(baiUrl, IOAdapterMode_Read)) {
                hasIndex = false;
            }
        }

        std::auto_ptr<Reader> reader(NULL);
        if (sam) {
            reader.reset(new SamReader(*ioAdapter));
        } else {
            reader.reset(new BamReader(*ioAdapter));
        }
        bamInfo.setHeader(reader->getHeader());

        if (!sam) {
            std::auto_ptr<BaiReader> baiReader(new BaiReader(*ioIndexAdapter));
            Index index;
            if(hasIndex) {
                index = baiReader->readIndex();
                if(index.getReferenceIndices().count() != reader->getHeader().getReferences().size()) {
                    throw Exception("Invalid index");
                }
                bamInfo.setIndex(index);
            }
        }
    } catch(const Exception &ex) {
        stateInfo.setError(ex.getMessage());
    }
}

} // namespace BAM
} // namespace U2
