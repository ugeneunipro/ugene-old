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

#include "BgzipTask.h"
#include "bgzf.h"

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2SafePoints.h>
#include <QtCore/QDir>


namespace U2 {

class BGZF_wrapper {
public:
    BGZF_wrapper(BGZF *adapter) : adapter(adapter) {}
    ~BGZF_wrapper(){ bgzf_close(adapter); }
private:
    BGZF *adapter;
};

BgzipTask::BgzipTask(const GUrl& fileUrl, const GUrl& bgzfUrl)
    : Task(tr("Bgzip Compression task"), (TaskFlag)(TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled)),
      fileUrl(fileUrl),
      bgzfUrl(bgzfUrl)
{
}

void BgzipTask::run() {
    taskLog.details(tr("Start bgzip compression '%1'").arg(fileUrl.getURLString()));

    SAFE_POINT_EXT(AppContext::getIOAdapterRegistry() != NULL, setError(tr("IOAdapterRegistry is NULL!")), );
    IOAdapterFactory* ioFactory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);

    SAFE_POINT_EXT(ioFactory != NULL, setError(tr("IOAdapterFactory is NULL!")), );
    QScopedPointer<IOAdapter> in(ioFactory->createIOAdapter());
    SAFE_POINT_EXT(!in.isNull(), setError(tr("Can not create IOAdapter!")), );

    bool res = in->open( fileUrl, IOAdapterMode_Read);
    if (!res) {
        Task::setError(tr("Can not open input file '%1'").arg(fileUrl.getURLString()));
        return;
    }
    if (bgzfUrl.isEmpty()) {
        bgzfUrl = GUrl(fileUrl.getURLString() + ".gz");
    }

    BGZF* out = bgzf_open( bgzfUrl.getURLString().toLatin1().data(), "w");
    BGZF_wrapper out_wr(out);
    if (out == NULL) {
        Task::setError(tr("Can not open output file '%2'").arg(bgzfUrl.getURLString()));
        return;
    }

    const int BUFFER_SIZE = 2048;
    QByteArray readBuffer(BUFFER_SIZE, '\0');
    char* buffer = readBuffer.data();

    while ( !in->isEof() ) {
        if ( isCanceled() ) {
            return;
        }
        int len = in->readBlock(buffer, BUFFER_SIZE);
        if (len == 0) {
            stateInfo.setError(tr("Error reading file"));
            return;
        }
        int written = bgzf_write(out, buffer, len);
        if (written == -1) {
            stateInfo.setError(tr("Error writing to file"));
            return;
        }

        stateInfo.setProgress( in->getProgress() );
    }

    taskLog.details(tr("Bgzip compression finished"));
}

QString BgzipTask::generateReport() const {
    if (hasError() || isCanceled()) {
        return tr("Bgzip compression task was finished with an error: %1").arg(getError());
    }
    return tr("Bgzip compression task was finished. A new bgzf file is: <a href=\"%1\">%2</a>").arg(bgzfUrl.getURLString()).arg(bgzfUrl.getURLString());
}

Task::ReportResult BgzipTask::report() {
    if (hasError() || isCanceled()) {
        QDir outputDir(bgzfUrl.dirPath());
        outputDir.remove(bgzfUrl.getURLString());
    }
    return ReportResult_Finished;
}

bool BgzipTask::checkBgzf(const GUrl &fileUrl) {
    return bgzf_check_bgzf(fileUrl.getURLString().toLatin1().constData());
}

} // namespace
