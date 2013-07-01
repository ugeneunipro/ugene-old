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

#ifndef _U2_SNP_REPORT_WRITER_TASK_
#define _U2_SNP_REPORT_WRITER_TASK_

#include <U2Core/Task.h>
#include <U2Core/U2Variant.h>

namespace U2 {

class SNPReportWriterSettings{
public:
    SNPReportWriterSettings(){};
    
    QString regulatoryReportPath;
    QString reportPath;
    QString dbPath;
};


class SNPReportWriterTask : public Task {
    Q_OBJECT
public:
    SNPReportWriterTask(const SNPReportWriterSettings& _settings, const QList<U2VariantTrack>& _tracks, const U2DbiRef& _dibRef);

    virtual void run();

    QStringList getOutputFilePaths() {return outPaths;}

private:
    SNPReportWriterSettings settings;
    QList<U2VariantTrack> tracks;
    U2DbiRef dbiRef;
    QStringList outPaths;

};

} // U2

#endif // _U2_SNP_REPORT_WRITER_TASK_
