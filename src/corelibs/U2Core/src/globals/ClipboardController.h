/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CLIPBOARD_CONTROLLER_H_
#define _U2_CLIPBOARD_CONTROLLER_H_

#include <U2Core/DocumentUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/Task.h>

#include <QClipboard>

namespace U2 {

class U2CORE_EXPORT PasteTask : public Task{
    Q_OBJECT
public:
    PasteTask();

    virtual QList<GUrl> getUrls() const = 0;
    virtual QList<Document*> getDocuments() const = 0;
protected:
   virtual void processDocument(Document* doc){}
};

class U2CORE_EXPORT PasteFactory : public QObject {
    Q_OBJECT
public:
    PasteFactory(QObject *parent=0);

    virtual PasteTask* pasteTask(bool useInSequenceWidget) = 0;
};

class U2CORE_EXPORT PasteUtils : public QObject {
    Q_OBJECT
public:
    static QList<DNASequence> getSequences(const QList<Document*>& docs, U2OpStatus& os);
};


} // U2

#endif // _U2_CLIPBOARD_CONTROLLER_H_
