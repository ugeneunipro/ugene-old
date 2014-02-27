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

#include "PMatrixFormat.h"
#include "WeightMatrixIO.h"
#include "ViewMatrixDialogController.h"

#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/RawDataUdrSchema.h>
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

U2::FormatCheckResult PFMatrixFormat::checkRawData( const QByteArray& rawData, const GUrl& /*url*/ ) const{
    const char* data = rawData.constData();
    int size = rawData.size();
    if(TextUtils::contains(TextUtils::BINARY, data, size)){
        return FormatDetection_NotMatched;
    }

    QString dataStr(rawData);
    QStringList qsl = dataStr.split("\n");
    qsl.removeAll(QString());
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
    DbiOperationsBlock opBlock(dbiRef, os);
    CHECK_OP(os, NULL);
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
    PFMatrixObject *mObj = PFMatrixObject::createInstance(m, QFileInfo(io->getURL().getURLString()).baseName(), dbiRef, os);
    CHECK_OP(os, NULL);
    objs.append(mObj);
    return new Document(this, io->getFactory(), io->getURL(), dbiRef, objs, fs);
}

namespace {
    template<class Serializer, class Matrix>
    U2EntityRef commit(const Matrix &matrix, const QString &objectName, const U2DbiRef &dbiRef, U2OpStatus &os) {
        U2RawData object(dbiRef);
        object.url = objectName;
        object.serializer = Serializer::ID;

        RawDataUdrSchema::createObject(dbiRef, object, os);
        CHECK_OP(os, U2EntityRef());

        const U2EntityRef entRef(dbiRef, object.id);
        const QByteArray data = Serializer::serialize(matrix);
        RawDataUdrSchema::writeContent(data, entRef, os);
        return entRef;
    }

    template<class Serializer, class Matrix>
    void retrieve(const U2EntityRef &entityRef, Matrix &matrix) {
        U2OpStatus2Log os;
        const QString serializer = RawDataUdrSchema::getObject(entityRef, os).serializer;
        CHECK_OP(os, );
        SAFE_POINT(Serializer::ID == serializer, "Unknown serializer id", );

        const QByteArray data = RawDataUdrSchema::readAllContent(entityRef, os);
        CHECK_OP(os, );
        matrix = Serializer::deserialize(data, os);
    }
}

//PFMatrixObject
//////////////////////////////////////////////////////////////////////////
PFMatrixObject * PFMatrixObject::createInstance(const PFMatrix &matrix, const QString &objectName, const U2DbiRef &dbiRef, U2OpStatus &os, const QVariantMap &hintsMap) {
    const U2EntityRef entRef = commit<FMatrixSerializer>(matrix, objectName, dbiRef, os);
    CHECK_OP(os, NULL);
    return new PFMatrixObject(matrix, objectName, entRef, hintsMap);
}

PFMatrixObject::PFMatrixObject(const QString &objectName, const U2EntityRef &matrixRef, const QVariantMap &hintsMap)
: GObject(TYPE, objectName, hintsMap)
{
    entityRef = matrixRef;
    retrieve<FMatrixSerializer>(entityRef, m);
}

PFMatrixObject::PFMatrixObject(const PFMatrix &matrix, const QString &objectName, const U2EntityRef &matrixRef, const QVariantMap &hintsMap)
: GObject(TYPE, objectName, hintsMap), m(matrix)
{
    entityRef = matrixRef;
}

const PFMatrix & PFMatrixObject::getMatrix() const {
    return m;
}

GObject * PFMatrixObject::clone(const U2DbiRef &dstRef, U2OpStatus &os) const{
    const U2RawData dstObject = RawDataUdrSchema::cloneObject(entityRef, dstRef, os);
    CHECK_OP(os, NULL);

    const U2EntityRef dstEntRef(dstRef, dstObject.id);
    PFMatrixObject *dst = new PFMatrixObject(getGObjectName(), dstEntRef, getGHintsMap());
    dst->setIndexInfo(getIndexInfo());
    return dst;
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
    qsl.removeAll(QString());
    if(qsl.size() > 5 || qsl.size() < 4){ //actually can be 5 or 6
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
    DbiOperationsBlock opBlock(dbiRef, os);
    CHECK_OP(os, NULL);
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
    PWMatrixObject *mObj = PWMatrixObject::createInstance(m, QFileInfo(io->getURL().getURLString()).baseName(), dbiRef, os);
    CHECK_OP(os, NULL);
    objs.append(mObj);
    return new Document(this, io->getFactory(), io->getURL(), dbiRef, objs, fs);
}

//PWMatrixObject
//////////////////////////////////////////////////////////////////////////
PWMatrixObject * PWMatrixObject::createInstance(const PWMatrix &matrix, const QString &objectName, const U2DbiRef &dbiRef, U2OpStatus &os, const QVariantMap &hintsMap) {
    const U2EntityRef entRef = commit<WMatrixSerializer>(matrix, objectName, dbiRef, os);
    CHECK_OP(os, NULL);
    return new PWMatrixObject(matrix, objectName, entRef, hintsMap);
}

PWMatrixObject::PWMatrixObject(const QString &objectName, const U2EntityRef &matrixRef, const QVariantMap &hintsMap)
: GObject(TYPE, objectName, hintsMap)
{
    entityRef = matrixRef;
    retrieve<WMatrixSerializer>(entityRef, m);
}

PWMatrixObject::PWMatrixObject(const PWMatrix &matrix, const QString &objectName, const U2EntityRef &matrixRef, const QVariantMap &hintsMap)
: GObject(TYPE, objectName, hintsMap), m(matrix)
{
    entityRef = matrixRef;
}

const PWMatrix & PWMatrixObject::getMatrix() const {
    return m;
}

GObject * PWMatrixObject::clone(const U2DbiRef &dstRef, U2OpStatus &os) const {
    const U2RawData dstObject = RawDataUdrSchema::cloneObject(entityRef, dstRef, os);
    CHECK_OP(os, NULL);

    const U2EntityRef dstEntRef(dstRef, dstObject.id);
    PWMatrixObject *dst = new PWMatrixObject(getGObjectName(), dstEntRef, getGHintsMap());
    dst->setIndexInfo(getIndexInfo());
    return dst;
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
