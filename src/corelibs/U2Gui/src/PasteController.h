/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PASTE_CONTROLLER_H_
#define _U2_PASTE_CONTROLLER_H_

#include <U2Core/ClipboardController.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/Task.h>

#include <QClipboard>

namespace U2 {

class U2GUI_EXPORT PasteTaskImpl : public PasteTask{
    Q_OBJECT
public:
    PasteTaskImpl(bool addToProject);
    void run();

    QList<GUrl>getUrls() const {return urls;}
    QList<Document*> getDocuments() const {return documents;}

protected:
   QList<GUrl> urls;
   bool addToProject;
   QList<Document*> documents;

protected:
   QList<Task*> onSubTaskFinished(Task* task);
};

class U2GUI_EXPORT PasteFactoryImpl : public PasteFactory {
    Q_OBJECT
public:
    PasteFactoryImpl(QObject *parent=0);

    PasteTask* pasteTask(bool useInSequenceWidget);

protected:
    void connectExclude(PasteTask* task);

protected:
    QSet<QString> excludedFilenames;

protected slots:
    void sl_excludeBack();
};

class PasteUrlsTask : public PasteTaskImpl{
    Q_OBJECT
public:
    PasteUrlsTask(const QList<QUrl> &urls, bool addToProject);
};

class PasteTextTask : public PasteTaskImpl {
    Q_OBJECT
public:
    PasteTextTask(const QClipboard* clipboard, QSet<QString>& excludedFilenames, bool addToProject);
};

} // U2

#endif // _U2_PASTE_CONTROLLER_H_
