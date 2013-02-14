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

#include "PMatrixFormat.h"
#include "WeightMatrixIO.h"
#include "ViewMatrixDialogController.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/PWMatrix.h>
#include <U2Core/SelectionUtils.h>
#include <U2Core/IOAdapterUtils.h>

namespace U2{

const DocumentFormatId PFMatrixFormat::FORMAT_ID = "PFMatrix";

const GObjectType PFMatrixObject::TYPE("pfm-obj");

PFMatrixFormat::PFMatrixFormat(QObject* p): DocumentFormat(p, DocumentFormatFlag_SingleObjectFormat, QStringList("pfm")) {
    formatName = tr("Position frequency matrix");
    supportedObjectTypes += PFMatrixObject::TYPE;
    formatDescription = tr("Position frequency matrix file.");
}

Document* PFMatrixFormat::createNewLoadedDocument( IOAdapterFactory* io, const QString& url, const QVariantMap& fs /*= QVariantMap()*/ ){
    U2OpStatus2Log os;
    Document* d = DocumentFormat::createNewLoadedDocument(io, url, os, fs);
    return d;
}

U2::FormatCheckResult PFMatrixFormat::checkRawData( const QByteArray& rawData, const GUrl& url ) const{
    const char* data = rawData.constData();
    int size = rawData.size();
    if(TextUtils::contains(TextUtils::BINARY, data, size)){
        return FormatDetection_NotMatched;
    }

    QString dataStr(rawData);
    QStringList qsl = dataStr.split("\n");
    if(qsl.size() > 5 || qsl.size() < 4){ //actually can be 4 or 5
        return FormatDetection_NotMatched;
    }
    foreach(QString str, qsl){
        QStringList line = str.split(QRegExp("\\s+"));
        foreach(QString word, line){
            if (!word.isEmpty()){
                bool isInt;
                word.toInt(&isInt);
                if (!isInt){
                    return FormatDetection_NotMatched;
                }
            }
        }
    }
    
    return FormatDetection_Matched;
}

Document* PFMatrixFormat::loadDocument( IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os ){
    QList<GObject*> objs;
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(io->getAdapterId());
    TaskStateInfo siPFM;
    PFMatrix m = WeightMatrixIO::readPFMatrix(iof, io->getURL().getURLString(), siPFM);
    if (siPFM.hasError()) {
        os.setError("Given file is not PFM");
    } else {
       if (m.getLength() == 0) {
            os.setError("Zero length or corrupted model\nMaybe model data is not enough for selected algorithm");
        }
    }
    objs.append(new PFMatrixObject(m, QFileInfo(io->getURL().getURLString()).baseName()));
    return new Document(this, io->getFactory(), io->getURL(), dbiRef, objs, fs);
}

//Factory
//////////////////////////////////////////////////////////////////////////
const PFMatrixViewFactoryId PFMatrixViewFactory::ID("pfm-view-factory");


bool PFMatrixViewFactory::canCreateView( const MultiGSelection& multiSelection ){
    foreach(GObject* go, SelectionUtils::findObjects(PFMatrixObject::TYPE, &multiSelection, UOF_LoadedOnly)) {
        QString cname = go->metaObject()->className();
        if (cname == "U2::PFMatrixObject") {
            return true;
        }
    }
    return false;
}

Task* PFMatrixViewFactory::createViewTask( const MultiGSelection& multiSelection, bool single /*= false*/ ){
    QSet<Document*> documents = SelectionUtils::findDocumentsWithObjects(PFMatrixObject::TYPE, &multiSelection, UOF_LoadedAndUnloaded, true);
    if (documents.size() == 0) {
        return NULL;
    }
    Task* result = (single || documents.size() == 1) ? NULL : new Task(tr("Open multiple views"), TaskFlag_NoRun);
    foreach(Document* d, documents) {
        Task* t = new OpenPFMatrixViewTask(d);
        if (result == NULL) {
            return t;
        } 
        result->addSubTask(t);
    }
    return result;
}


OpenPFMatrixViewTask::OpenPFMatrixViewTask( Document* doc ): ObjectViewTask(PFMatrixViewFactory::ID), document(doc){
    if(!doc->isLoaded()) {
        documentsToLoad.append(doc);
    } else {
        foreach(GObject* go, doc->findGObjectByType(PFMatrixObject::TYPE)) {
            selectedObjects.append(go) ;
        }
        assert(!selectedObjects.isEmpty());
    }
}

void OpenPFMatrixViewTask::open(){
    if (stateInfo.hasError()) {
        return;
    }
    if (!documentsToLoad.isEmpty()) {
        foreach(GObject* go, documentsToLoad.first()->findGObjectByType(PFMatrixObject::TYPE)) {
            selectedObjects.append(go);
        }
    }
    foreach(QPointer<GObject> po, selectedObjects) {
        PFMatrixObject* o = qobject_cast<PFMatrixObject*>(po);
        MatrixViewController *view = new MatrixViewController(o->getMatrix());
        AppContext::getMainWindow()->getMDIManager()->addMDIWindow(view);
        AppContext::getMainWindow()->getMDIManager()->activateWindow(view);
    }
}

///PWM

const DocumentFormatId PWMatrixFormat::FORMAT_ID = "PWMatrix";

const GObjectType PWMatrixObject::TYPE("pwm-obj");

PWMatrixFormat::PWMatrixFormat( QObject* p ): DocumentFormat(p, DocumentFormatFlag_SingleObjectFormat, QStringList("pwm")) {
    formatName = tr("Position weight matrix");
    supportedObjectTypes += PFMatrixObject::TYPE;
    formatDescription = tr("Position weight matrix file.");
}

Document* PWMatrixFormat::createNewLoadedDocument( IOAdapterFactory* io, const QString& url, const QVariantMap& fs){
    U2OpStatus2Log os;
    Document* d = DocumentFormat::createNewLoadedDocument(io, url, os, fs);
    return d;
}

U2::FormatCheckResult PWMatrixFormat::checkRawData( const QByteArray& rawData, const GUrl& ) const {
    const char* data = rawData.constData();
    int size = rawData.size();
    if(TextUtils::contains(TextUtils::BINARY, data, size)){
        return FormatDetection_NotMatched;
    }

    QString dataStr(rawData);
    QStringList qsl = dataStr.split("\n");
    if(qsl.size() > 6 || qsl.size() < 5){ //actually can be 5 or 6
        return FormatDetection_NotMatched;
    }
    qsl.pop_front(); //skip first line
    foreach(QString str, qsl){
        QStringList line = str.split(QRegExp("\\s+"));
        if(line.at(0).isEmpty()){
            break;
        }
        if(line.at(0).at(1) != ':'){
            return FormatDetection_NotMatched;
        }
        line.pop_front();
        foreach(QString word, line){
            if (!word.isEmpty()){
                bool isFloat;
                word.toFloat(&isFloat);
                if (!isFloat){
                    return FormatDetection_NotMatched;
                }
            }
        }
    }

    return FormatDetection_Matched;
}

Document* PWMatrixFormat::loadDocument( IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os ){
    QList<GObject*> objs;
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(io->getAdapterId());
    TaskStateInfo siPWM;
    PWMatrix m = WeightMatrixIO::readPWMatrix(iof, io->getURL().getURLString(), siPWM);
    if (siPWM.hasError()) {
        os.setError("Given file is not PWM");
    } else {
        if (m.getLength() == 0) {
            os.setError("Zero length or corrupted model\nMaybe model data is not enough for selected algorithm");
        }
    }
    objs.append(new PWMatrixObject(m, QFileInfo(io->getURL().getURLString()).baseName()));
    return new Document(this, io->getFactory(), io->getURL(), dbiRef, objs, fs);
}

//Factory
//////////////////////////////////////////////////////////////////////////
const PWMatrixViewFactoryId PWMatrixViewFactory::ID("pwm-view-factory");

bool PWMatrixViewFactory::canCreateView( const MultiGSelection& multiSelection ){
    foreach(GObject* go, SelectionUtils::findObjects(PWMatrixObject::TYPE, &multiSelection, UOF_LoadedOnly)) {
        QString cname = go->metaObject()->className();
        if (cname == "U2::PWMatrixObject") {
            return true;
        }
    }
    return false;
}

Task* PWMatrixViewFactory::createViewTask( const MultiGSelection& multiSelection, bool single /*= false*/ ){
    QSet<Document*> documents = SelectionUtils::findDocumentsWithObjects(PWMatrixObject::TYPE, &multiSelection, UOF_LoadedAndUnloaded, true);
    if (documents.size() == 0) {
        return NULL;
    }
    Task* result = (single || documents.size() == 1) ? NULL : new Task(tr("Open multiple views"), TaskFlag_NoRun);
    foreach(Document* d, documents) {
        Task* t = new OpenPWMatrixViewTask(d);
        if (result == NULL) {
            return t;
        } 
        result->addSubTask(t);
    }
    return result;
}


OpenPWMatrixViewTask::OpenPWMatrixViewTask( Document* doc ): ObjectViewTask(PWMatrixViewFactory::ID), document(doc){
    if(!doc->isLoaded()) {
        documentsToLoad.append(doc);
    } else {
        foreach(GObject* go, doc->findGObjectByType(PWMatrixObject::TYPE)) {
            selectedObjects.append(go) ;
        }
        assert(!selectedObjects.isEmpty());
    }
}

void OpenPWMatrixViewTask::open(){
    if (stateInfo.hasError()) {
        return;
    }
    if (!documentsToLoad.isEmpty()) {
        foreach(GObject* go, documentsToLoad.first()->findGObjectByType(PWMatrixObject::TYPE)) {
            selectedObjects.append(go);
        }
    }
    foreach(QPointer<GObject> po, selectedObjects) {
        PWMatrixObject* o = qobject_cast<PWMatrixObject*>(po);
        MatrixViewController *view = new MatrixViewController(o->getMatrix());
        AppContext::getMainWindow()->getMDIManager()->addMDIWindow(view);
        AppContext::getMainWindow()->getMDIManager()->activateWindow(view);
    }
}

}