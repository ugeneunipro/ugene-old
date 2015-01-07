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

#include <U2Core/AppContext.h>
#include <U2Core/ScriptingToolRegistry.h>

#include "JavaSupport.h"

namespace U2 {

JavaSupport::JavaSupport(const QString &name, const QString &path)
: ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }
    executableFileName = "java";

    validMessage = "java version \"\\d+.[789]";
    validationArguments << "-version";

    description += tr("Java Platform lets you develop and deploy Java applications on desktops and servers.<br><i>(Requires Java 1.7 or higher)</i>.<br>"
                      "Java can be freely downloaded on the official web-site: https://www.java.com/en/download/");
    versionRegExp = QRegExp("(\\d+.\\d+.\\d+)");
    toolKitName="Java";

    muted = true;

    connect(this, SIGNAL(si_toolValidationStatusChanged(bool)), SLOT(sl_toolValidationStatusChanged(bool)));
}

void JavaSupport::sl_toolValidationStatusChanged(bool isValid) {
    Q_UNUSED(isValid);
    ScriptingTool::onPathChanged(this, QStringList() 

     #ifdef Q_OS_WIN
     #else
     << "-Xmx6G" //default memory pool is 2GB. This command makes it 6Gb
     #endif

    << "-jar");
}

} // U2
