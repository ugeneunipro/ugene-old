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

#include <U2Core/GUrl.h>
#include <U2Core/U2OpStatus.h>
#include <QtGui/QMessageBox>
#include <U2Test/GUITestBase.h>
#include "AppUtils.h"
#include "DocumentUtils.h"
#include "GUIDialogUtils.h"

namespace U2 {

class GUITest;
class Document;

class ProjectUtils {
public:
    class OpenFileSettings {
    public:
        enum OpenMethod {DRAGDROP} openMethod;
    };

    class CloseProjectSettings {
    public:
        enum SaveOnClose {NO, YES, CANCEL} saveOnClose;
    };

    class OpenProjectGUIAction : public GUIMultiTest {
    public:
        OpenProjectGUIAction(const GUrl& path, const QString& projectName, const QString& documentName) {
            add(new ProjectUtils::OpenFilesGUIAction(path));
            add(new AppUtils::CheckUGENETitleGUIAction(projectName));
            add(new DocumentUtils::CheckDocumentExistsGUIAction(documentName));
        }
    };

    /*
        opens files using settings, checks if the document is loaded
    */
    class OpenFilesGUIAction : public GUIMultiTest {
    public:
        OpenFilesGUIAction(const QList<QUrl> &_urls, const OpenFileSettings& _s = OpenFileSettings())
            : urls(_urls), s(_s){ addSubTests(); }

        OpenFilesGUIAction(const GUrl &path, const OpenFileSettings& _s = OpenFileSettings())
            : s(_s), urls(QList<QUrl>() << path.getURLString()){ addSubTests(); }

    protected:
        void addSubTests();
    private:
        QList<QUrl> urls;
        OpenFileSettings s;
    };

    class ExportProjectGUIAction : public GUIMultiTest {
    public:
        ExportProjectGUIAction(const QString &projectFolder, const QString &projectName = "") {
            add( new GUIDialogUtils::OpenExportProjectDialogGUIAction() );
            add( new GUIDialogUtils::FillInExportProjectDialogGUIAction(projectFolder, projectName) );
        }
    };

    class SaveProjectAsGUIAction : public GUIMultiTest {
    public:
        SaveProjectAsGUIAction(const QString &projectName, const QString &projectFolder, const QString &projectFile, bool overwriteExisting = true);
    };

    class CloseProjectGUIAction : public GUIMultiTest {
    public:
        CloseProjectGUIAction(const CloseProjectSettings& settings = CloseProjectSettings());
    };

    class CheckProjectGUIAction : public GUITest {
    public:
        enum CheckType {EXISTS, EMPTY};
        CheckProjectGUIAction(CheckType _checkType = EXISTS) : checkType(_checkType){}
    protected:
        virtual void execute(U2OpStatus &os);
        CheckType checkType;
    };

protected:
    class OpenFilesDropGUIAction : public GUITest {
    public:
        OpenFilesDropGUIAction(const QList<QUrl> &_urls) : urls(_urls){}
    protected:
        virtual void execute(U2OpStatus &os);
    private:
        QList<QUrl> urls;
    };

    static void checkProjectExists(U2OpStatus &os);

    static Document* checkDocumentExists(U2OpStatus &os, const GUrl &url);
    GENERATE_GUI_ACTION_1(CheckDocumentExistsGUIAction, checkDocumentExists);

    static void checkDocumentActive(U2OpStatus &os, Document *doc);

    static void closeProjectByHotkey(U2OpStatus &os);
};

} // U2

#endif
