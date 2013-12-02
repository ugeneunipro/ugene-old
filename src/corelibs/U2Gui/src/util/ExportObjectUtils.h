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

#ifndef _U2_EXPORT_OBJECT_UTILS_H_
#define _U2_EXPORT_OBJECT_UTILS_H_

#include <U2Core/global.h>

namespace U2 {

class ExportDocumentDialogController;
class GUrl;

class U2GUI_EXPORT ExportObjectUtils {
public:
    static void            exportAnnotations( const QList<Annotation *> &annotations, const GUrl &dstUrl );
    static void            exportObject2Document( GObject *object, const QString &url = QString( ),
                        bool tracePath = true );
    static void            export2Document( ExportDocumentDialogController &dialog,
                        bool tracePath = true );
    static Task *          saveAnnotationsTask(const QString &filepath, const DocumentFormatId & format,
                        const QList<Annotation *> &annList );
};

} // namespace U2

#endif // _U2_EXPORT_OBJECT_UTILS_H_
