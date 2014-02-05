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

#ifndef _U2_PYTHON_SUPPORT_H_
#define _U2_PYTHON_SUPPORT_H_

#include <U2Core/ExternalToolRegistry.h>
#include "utils/ExternalToolSupportAction.h"

#define ET_PYTHON "python"
#define ET_PYTHON_DJANGO "django"
#define ET_PYTHON_NUMPY "numpy"

namespace U2 {

class PythonSupport : public ExternalTool {
    Q_OBJECT
public:
    PythonSupport(const QString& name, const QString& path = "");
private slots:
    void sl_toolValidationStatusChanged(bool isValid);
};

class PythonModuleSupport : public ExternalToolModule {
    Q_OBJECT
public:
    PythonModuleSupport(const QString& name);
};

class PythonModuleDjangoSupport : public PythonModuleSupport {
    Q_OBJECT
public:
    PythonModuleDjangoSupport(const QString& name);
};

class PythonModuleNumpySupport : public PythonModuleSupport {
    Q_OBJECT
public:
    PythonModuleNumpySupport(const QString& name);
};

}//namespace
#endif // _U2_PYTHON_SUPPORT_H_
