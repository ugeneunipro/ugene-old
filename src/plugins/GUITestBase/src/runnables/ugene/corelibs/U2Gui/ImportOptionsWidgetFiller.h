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

#ifndef _U2_IMPORT_OPTIONS_WIDGET_FILLER_H_
#define _U2_IMPORT_OPTIONS_WIDGET_FILLER_H_

#include <QtCore/QVariantMap>

namespace U2 {

class ImportOptionsWidget;
class U2OpStatus;

class ImportOptionsWidgetFiller {
public:
    static void fill(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);

    static const QString DESTINATION_FOLDER;
    static const QString KEEP_FOLDERS_STRUCTURE;
    static const QString PROCESS_FOLDERS_RECUSIVELY;
    static const QString CREATE_SUBFOLDER_FOR_TOP_LEVEL_FOLDER;
    static const QString CREATE_SUBFOLDER_FOR_EACH_FILE;
    static const QString IMPORT_UNKNOWN_AS_UDR;
    static const QString MULTI_SEQUENCE_POLICY;
    static const QString CREATE_SUBFOLDER_FOR_DOCUMENT;
    static const QString MERGE_MULTI_SEQUENCE_POLICY_SEPARATOR_SIZE;

private:
    static void setDestinationFolder(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
    static void setKeepFoldersStructure(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
    static void setProcessFoldersRecusively(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
    static void setCreateSubfolderForTopLevelFolder(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
    static void setCreateSubfolderForEachFile(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
    static void setImportUnknownAsUdr(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
    static void setMultiSequencePolicy(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
    static void setCreateSubfolderForDocument(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
    static void setMergeMultiSequencePolicySeparatorSize(U2OpStatus& os, ImportOptionsWidget* optionsWidget, const QVariantMap& data);
};

}   // namespace U2

#endif // _U2_IMPORT_OPTIONS_WIDGET_FILLER_H_
