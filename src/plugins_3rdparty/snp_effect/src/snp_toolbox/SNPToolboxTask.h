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

#ifndef _U2_SNP_TOOLBOX_TASK_
#define _U2_SNP_TOOLBOX_TASK_

#include <U2Core/Task.h>
#include <U2Core/U2Variant.h>

namespace U2 {

class SNPToolboxSettings{
public:
    SNPToolboxSettings(){};
    
    QString dbPath;
};


class SNPToolboxTask : public Task {
    Q_OBJECT
public:
    SNPToolboxTask(const SNPToolboxSettings& _settings, const U2VariantTrack& _track, const U2DbiRef& _dibRef);

    virtual void run();

private:
    SNPToolboxSettings settings;
    U2VariantTrack track;
    U2DbiRef dbiRef;

};

} // U2

#endif // _U2_SNP_TOOLBOX_TASK_
