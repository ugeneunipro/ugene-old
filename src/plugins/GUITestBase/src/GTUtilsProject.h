/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PROJECT_UTILS_H_
#define _U2_PROJECT_UTILS_H_

#include "api/GTGlobals.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsDialogRunnables.h"
#include <U2Core/GUrl.h>
#include <QtGui/QMessageBox>

namespace U2 {

class GUITest;
class Document;

class GTUtilsProject {
public:
    class OpenFileSettings {
    public:
        enum OpenMethod {DragDrop} openMethod;
    };

    class CloseProjectSettings {
    public:
        CloseProjectSettings() : saveOnCloseButton(QMessageBox::No){}

        // QMessageBox::No, QMessageBox::Yes, QMessageBox::Cancel
        QMessageBox::StandardButton saveOnCloseButton;
    };

    class ExitProjectSettings {
    public:
        ExitProjectSettings() : saveNoCloseButton(QMessageBox::No){}

        QMessageBox::StandardButton saveNoCloseButton;

    };
    /*
        opens files using settings, checks if the document is loaded
    */
    static void openFiles(U2OpStatus &os, const QList<QUrl> &urls, const OpenFileSettings& s = OpenFileSettings());
    static void openFiles(U2OpStatus &os, const GUrl &path, const OpenFileSettings& s = OpenFileSettings());

    static void exportProject(U2OpStatus &os, const QString &projectFolder, const QString &projectName = "");
    static void createAnnotation(U2OpStatus &os, const QString &groupName = "", const QString &annotationName = "", const QString &location = "");
    static void exportProjectCheck(U2OpStatus &os, const QString &projectName);

    static void exportToSequenceFormat(U2OpStatus &os, const QString &projectName, const QString &path,
                                       const QString &name, GTGlobals::UseMethod = GTGlobals::UseMouse);
    static void exportSequenceAsAlignment(U2OpStatus &os, const QString projectName, const QString &path,
                                          const QString &name, GTUtilsDialogRunnables::ExportSequenceAsAlignmentFiller::FormatToUse,
                                          bool addDocToProject = false, GTGlobals::UseMethod = GTGlobals::UseMouse);

    static void exportSequenceOfSelectedAnnotations(U2OpStatus &os, const QString &itemToClick, const QString &path,
                                                    GTUtilsDialogRunnables::ExportSequenceOfSelectedAnnotationsFiller::FormatToUse format,
                                                    GTUtilsDialogRunnables::ExportSequenceOfSelectedAnnotationsFiller::MergeOptions options,
                                                    int gapLength, bool addDocToProject = true, bool exportWithAnnotations = false, GTGlobals::UseMethod = GTGlobals::UseMouse);
    static void exportAnnotations(U2OpStatus &os, const QString &itemToClick, const QString &exportToFile, GTUtilsDialogRunnables::ExportAnnotationsFiller::fileFormat format, 
                                  bool saveSequencesUnderAnnotations = true, bool saveSequenceNames = true, GTGlobals::UseMethod = GTGlobals::UseMouse);
    static void createDocument(U2OpStatus &os, const QString &pasteDataHere, const QString &documentLocation, GTUtilsDialogRunnables::CreateDocumentFiller::documentFormat format,
                                const QString &sequenceName, GTGlobals::UseMethod = GTGlobals::UseMouse);
    static void insertSequence(U2OpStatus &os, const QString &pasteDataHere,
                                     const QString &documentLocation, GTUtilsDialogRunnables::InsertSequenceFiller::documentFormat format,
                                     bool saveToNewFile, bool mergeAnnotations,
                                     GTGlobals::UseMethod = GTGlobals::UseMouse);
    static void saveProjectAs(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile);
    static void closeProject(U2OpStatus &os, const CloseProjectSettings& settings = CloseProjectSettings());
    static void exitProject(U2OpStatus &os, const ExitProjectSettings& exitSettings = ExitProjectSettings());

    enum CheckType {Exists, Empty};
    static void checkProject(U2OpStatus &os, CheckType checkType = Exists);

    // sets filter in a project view widget by finding "nameFilterEdit" QLineEdit
    static void setFilter(U2OpStatus &os, const QString& filter);

protected:
    static void openFilesDrop(U2OpStatus &os, const QList<QUrl>& urls);
};

} // U2

#endif
