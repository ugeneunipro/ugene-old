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

#ifndef _U2_BEDTOOLS_SUPPORT_H_
#define _U2_BEDTOOLS_SUPPORT_H_

#include <U2Core/ExternalToolRegistry.h>

#define ET_BEDTOOLS "bedtools"

namespace U2 {

class BedtoolsSupport : public ExternalTool {
    Q_OBJECT
public:
    BedtoolsSupport(const QString& name, const QString& path = "");

private slots:
    void sl_validationStatusChanged(bool newStatus);

};

}//namespace
#endif // _U2_BEDTOOLS_SUPPORT_H_
