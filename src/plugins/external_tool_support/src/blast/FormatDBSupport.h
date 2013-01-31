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

#ifndef _U2_FORMATDB_SUPPORT_H
#define _U2_FORMATDB_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include "utils/ExternalToolSupportAction.h"

#define FORMATDB_TOOL_NAME "FormatDB"
#define CUDA_FORMATDB_TOOL_NAME "CUDA-FormatDB"
#define MAKEBLASTDB_TOOL_NAME "MakeBLASTDB"
#define GPU_MAKEBLASTDB_TOOL_NAME "GPU-MakeBLASTDB"
namespace U2 {

class FormatDBSupport : public ExternalTool {
    Q_OBJECT
public:
    FormatDBSupport(const QString& name, const QString& path = "");

public slots:
    void sl_runWithExtFileSpecify();
};

}//namespace
#endif // _U2_FORMATDB_SUPPORT_H
