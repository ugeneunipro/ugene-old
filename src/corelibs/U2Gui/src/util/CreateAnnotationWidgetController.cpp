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

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QRadioButton>
#include <QToolButton>
#include <QVBoxLayout>

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationGroup.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Core/TextUtils.h>
#include <U2Core/Settings.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/GenbankFeatures.h>
#include <U2Formats/GenbankLocationParser.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/ShowHideSubgroupWidget.h>

#include "CreateAnnotationFullWidget.h"
#include "CreateAnnotationNormalWidget.h"
#include "CreateAnnotationOptionsPanelWidget.h"
#include "CreateAnnotationWidgetController.h"
#include "GObjectComboBoxController.h"

#define SETTINGS_LASTDIR "create_annotation/last_dir"

namespace U2 {
/* TRANSLATOR U2::CreateAnnotationWidgetController */

CreateAnnotationModel::CreateAnnotationModel() :
    defaultIsNewDoc(false),
    hideLocation(false),
    hideAnnotationType(false),
    hideAnnotationName(false),
    hideDescription(true),
    hideUsePatternNames(true),
    useUnloadedObjects(false),
    hideAutoAnnotationsOption(true),
    hideAnnotationParameters(false)
{
}

AnnotationTableObject * CreateAnnotationModel::getAnnotationObject( ) const {
    GObject *res = GObjectUtils::selectObjectByReference( annotationObjectRef, UOF_LoadedOnly );
    AnnotationTableObject *aobj = qobject_cast<AnnotationTableObject *>( res );
    SAFE_POINT( NULL != aobj, "Invalid annotation table detected!", NULL );
    return aobj;
}

const QString CreateAnnotationWidgetController::GROUP_NAME_AUTO = QObject::tr("<auto>");
const QString CreateAnnotationWidgetController::DESCRIPTION_QUALIFIER_KEY = "note";

CreateAnnotationWidgetController::CreateAnnotationWidgetController(const CreateAnnotationModel& m,
                                                                   QObject* p,
                                                                   AnnotationWidgetMode layoutMode) :
    QObject(p),
    model(m)
{
    this->setObjectName("CreateAnnotationWidgetController");
    assert(AppContext::getProject()!=NULL);
    assert(model.sequenceObjectRef.isValid());

    createWidget(layoutMode);

    GObjectComboBoxControllerConstraints occc;
    occc.relationFilter.ref = model.sequenceObjectRef;
    occc.relationFilter.role = ObjectRole_Sequence;
    occc.typeFilter = GObjectTypes::ANNOTATION_TABLE;
    occc.onlyWritable = true;
    occc.uof = model.useUnloadedObjects ? UOF_LoadedAndUnloaded : UOF_LoadedOnly;

    occ = w->createGObjectComboBoxController(occc);

    commonWidgetUpdate(model);

    connect(w, SIGNAL(si_selectNewTableRequest()), SLOT(sl_onNewDocClicked()));
    connect(w, SIGNAL(si_selectExistingTableRequest()), SLOT(sl_onLoadObjectsClicked()));
    connect(w, SIGNAL(si_selectGroupNameMenuRequest()), SLOT(sl_groupName()));
    connect(w, SIGNAL(si_groupNameEdited()), SLOT(sl_groupNameEdited()));
    connect(w, SIGNAL(si_annotationNameEdited()), SLOT(sl_annotationNameEdited()));
    connect(w, SIGNAL(si_usePatternNamesStateChanged()), SLOT(sl_usePatternNamesStateChanged()));
    connect(occ, SIGNAL(si_comboBoxChanged()), SLOT(sl_documentsComboUpdated()));
}


void CreateAnnotationWidgetController::updateWidgetForAnnotationModel(const CreateAnnotationModel& newModel)
{
    SAFE_POINT(newModel.sequenceObjectRef.isValid(),
        "Internal error: incorrect sequence object reference was supplied"
        "to the annotation widget controller.",);

    model = newModel;

    GObjectComboBoxControllerConstraints occc;
    occc.relationFilter.ref = newModel.sequenceObjectRef;
    occc.relationFilter.role = ObjectRole_Sequence;
    occc.typeFilter = GObjectTypes::ANNOTATION_TABLE;
    occc.onlyWritable = true;
    occc.uof = newModel.useUnloadedObjects ? UOF_LoadedAndUnloaded : UOF_LoadedOnly;

    occ->updateConstrains(occc);

    commonWidgetUpdate(newModel);
}


void CreateAnnotationWidgetController::commonWidgetUpdate(const CreateAnnotationModel& model) {
    w->setLocationVisible(!model.hideLocation);
    w->setAnnotationNameVisible(!model.hideAnnotationName);

    QString dir = AppContext::getSettings()->getValue(SETTINGS_LASTDIR, QString(""), true).toString();
    if (dir.isEmpty() || !QDir(dir).exists()) {
        dir = QDir::homePath();
        Project* prj = AppContext::getProject();
        if (prj != NULL) {
            const QString& prjUrl = prj->getProjectURL();
            if (!prjUrl.isEmpty()) {
                QFileInfo fi(prjUrl);
                const QDir& prjDir = fi.absoluteDir();
                dir = prjDir.absolutePath();
            }
        }
    }
    dir+="/";
    QString baseName = "MyDocument";
    QString ext = ".gb";
    QString url = dir + baseName + ext;
    for (int i=1; QFileInfo(url).exists() || AppContext::getProject()->findDocumentByURL(url)!= NULL; i++) {
        url = dir + baseName +"_"+QString::number(i) + ext;
    }
    w->setNewTablePath(url);

    if (model.annotationObjectRef.isValid()) {
        occ->setSelectedObject(model.annotationObjectRef);
    } 

    //default field values
    if (U2FeatureTypes::Invalid != model.data.type) {
        w->setAnnotationType(model.data.type);
    }

    w->setAnnotationName(model.data.name);
    w->setGroupName(model.groupName.isEmpty() ? GROUP_NAME_AUTO : model.groupName);

    if ( !model.data.location->isEmpty( ) ) {
        w->setLocation(model.data.location);
    }

    if (model.defaultIsNewDoc || w->isExistingTablesListEmpty()) {
        w->setExistingTableOptionEnable(false);
        w->selectNewTableOption();
    }
    else {
        w->setExistingTableOptionEnable(true);
    }

    w->setAutoTableOptionVisible(!model.hideAutoAnnotationsOption);
    if (!model.hideAutoAnnotationsOption) {
        w->selectAutoTableOption();
    }

    w->setDescriptionVisible(!model.hideDescription);
    w->setAnnotationTypeVisible(!model.hideAnnotationType);
    w->setAnnotationParametersVisible(!model.hideAnnotationParameters);
    w->setUsePatternNamesVisible(!model.hideUsePatternNames);
}

void CreateAnnotationWidgetController::sl_onNewDocClicked() {
    QString openUrl = QFileInfo(w->getNewTablePath()).absoluteDir().absolutePath();
    QString filter = DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::PLAIN_GENBANK, false);
    QString name = U2FileDialog::getSaveFileName(NULL, tr("Save file"), openUrl, filter);
    if (!name.isEmpty()) {
        w->setNewTablePath(name);
        AppContext::getSettings()->setValue(SETTINGS_LASTDIR, QFileInfo(name).absoluteDir().absolutePath(), true);
    }
}
class PTCAnnotationObjectFilter: public PTCObjectRelationFilter {
public:
    PTCAnnotationObjectFilter(const GObjectRelation& _rel, bool _allowUnloaded, QObject* p = NULL)
        : PTCObjectRelationFilter(_rel, p), allowUnloaded(_allowUnloaded){}

    bool filter(GObject* obj) const {
        if (PTCObjectRelationFilter::filter(obj)) {
            return true;
        }
        if (obj->isUnloaded()) {
            return !allowUnloaded;
        }
        SAFE_POINT( NULL != qobject_cast<AnnotationTableObject *>( obj ),
            "Invalid annotation table object!", false );
        return obj->isStateLocked();
    }
    bool allowUnloaded;
};

void CreateAnnotationWidgetController::sl_onLoadObjectsClicked() {
    ProjectTreeControllerModeSettings s;
    s.allowMultipleSelection = false;
    s.objectTypesToShow.insert(GObjectTypes::ANNOTATION_TABLE);
    s.groupMode = ProjectTreeGroupMode_ByDocument;
    GObjectRelation rel(model.sequenceObjectRef, ObjectRole_Sequence);
    QScopedPointer<PTCAnnotationObjectFilter> filter(new PTCAnnotationObjectFilter(rel, model.useUnloadedObjects));
    s.objectFilter = filter.data();
    QList<GObject*> objs = ProjectTreeItemSelectorDialog::selectObjects(s, w);
    if (objs.isEmpty()) {
        return;
    }
    assert(objs.size() == 1);
    GObject* obj = objs.first();
    occ->setSelectedObject(obj);
}

QString CreateAnnotationWidgetController::validate() {
    updateModel(true);
    if (!model.annotationObjectRef.isValid()) {
        if (model.newDocUrl.isEmpty()) {
            return tr("Select annotation saving parameters");
        }
        if (AppContext::getProject()->findDocumentByURL(model.newDocUrl)!=NULL) {
            return tr("Document is already added to the project: '%1'").arg(model.newDocUrl);
        }
        QString dirUrl = QFileInfo(w->getNewTablePath()).absoluteDir().absolutePath();
        QDir dir(dirUrl);
        if (!dir.exists()) {
            return tr("Illegal folder: %1").arg(dirUrl);
        }
    }

    if (!w->isUsePatternNamesChecked() && !model.hideAnnotationName && !Annotation::isValidAnnotationName(model.data.name)) {
        return tr("Illegal annotation name");
    }

    if (!AnnotationGroup::isValidGroupName(model.groupName, true)) {
        w->focusGroupName();
        return tr("Illegal group name");
    }
    
    static const QString INVALID_LOCATION = tr("Invalid location! Location must be in GenBank format.\nSimple examples:\n1..10\njoin(1..10,15..45)\ncomplement(5..15)");
    
    if (!model.hideLocation && model.data.location->isEmpty()) {
        w->focusLocation();
        return INVALID_LOCATION;
    }
    if (!model.hideLocation){
        foreach (const U2Region &reg, model.data.getRegions()) {
            if( reg.endPos() > model.sequenceLen || reg.startPos < 0 || reg.endPos() < reg.startPos) {
                return INVALID_LOCATION;
            }
        }
    }

    return QString::null;
}

void CreateAnnotationWidgetController::updateModel(bool forValidation) {
    model.data.type = U2FeatureTypes::getTypeByName(w->getAnnotationTypeString());

    model.data.name = w->getAnnotationName();
    if (model.data.name.isEmpty()) {
        model.data.name = U2FeatureTypes::getVisualName(model.data.type);
    }

    model.groupName = w->getGroupName();
    if (model.groupName == GROUP_NAME_AUTO || model.groupName.isEmpty()) {
        model.groupName = model.data.name;
    }

    model.data.location->reset();
    
    if (!model.hideLocation) {
        QByteArray locEditText = w->getLocationString().toLatin1();
        Genbank::LocationParser::parseLocation(locEditText.constData(),
            locEditText.length(), model.data.location, model.sequenceLen);
    }

    model.description = w->getDescription();
    if (forValidation) {
        model.data.qualifiers.clear();
        model.data.qualifiers << U2Qualifier(DESCRIPTION_QUALIFIER_KEY, model.description);
    }

    if (w->isExistingTableOptionSelected()) {
        model.annotationObjectRef = occ->getSelectedObject();
        model.newDocUrl = "";
    } else {
        if (!forValidation){
            model.annotationObjectRef = GObjectReference();
        }
        model.newDocUrl = w->getNewTablePath();
    }
}

void CreateAnnotationWidgetController::createWidget(CreateAnnotationWidgetController::AnnotationWidgetMode layoutMode) {
    switch (layoutMode) {
    case Full:
        w = new CreateAnnotationFullWidget();
        break;
    case Normal:
        w = new CreateAnnotationNormalWidget();
        break;
    case OptionsPanel:
        w = new CreateAnnotationOptionsPanelWidget();
        break;
    default:
        w = NULL;
        FAIL("Unexpected widget type", );
    }
}

bool CreateAnnotationWidgetController::prepareAnnotationObject() {
    updateModel(false);
    QString v = validate();
    SAFE_POINT(v.isEmpty(), "Annotation model is not valid", false);
    if (!model.annotationObjectRef.isValid() && w->isNewTableOptionSelected()) {
        SAFE_POINT(!model.newDocUrl.isEmpty(), "newDocUrl is empty", false);
        SAFE_POINT(AppContext::getProject()->findDocumentByURL(model.newDocUrl)==NULL, "cannot create a document that is already in the project", false);
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
        U2OpStatus2Log os;
        Document* d = df->createNewLoadedDocument(iof, model.newDocUrl, os);
        CHECK_OP(os, false);
        const U2DbiRef dbiRef = AppContext::getDbiRegistry( )->getSessionTmpDbiRef( os );
        SAFE_POINT_OP( os, false );
        AnnotationTableObject *aobj = new AnnotationTableObject( "Annotations", dbiRef );
        aobj->addObjectRelation(GObjectRelation(model.sequenceObjectRef, ObjectRole_Sequence));
        d->addObject(aobj);
        AppContext::getProject()->addDocument(d);
        model.annotationObjectRef = aobj;
    }

    return true;
}

void CreateAnnotationWidgetController::sl_groupName( ) {
    GObject* obj = occ->getSelectedObject( );
    QStringList groupNames; 
    groupNames << GROUP_NAME_AUTO;
    if ( NULL != obj && !obj->isUnloaded( ) ) {
        AnnotationTableObject* ao = qobject_cast<AnnotationTableObject *>( obj );
        ao->getRootGroup( ).getSubgroupPaths( groupNames );
    }
    SAFE_POINT( !groupNames.isEmpty( ), "Unable to find annotation groups!", );
    if (groupNames.size() == 1) {
        w->setGroupName(groupNames.first());
        return;
    }
    qSort(groupNames);

    QMenu menu(w);
    foreach(const QString& str, groupNames) {
        QAction* a = new QAction(str, &menu);
        connect(a, SIGNAL(triggered()), SLOT(sl_setPredefinedGroupName()));
        menu.addAction(a);
    }
    w->showSelectGroupMenu(menu);
}

void CreateAnnotationWidgetController::sl_setPredefinedGroupName() {
    QAction* a = qobject_cast<QAction*>(sender());
    QString name = a->text();
    w->setGroupName(name);
}

bool CreateAnnotationWidgetController::isNewObject() const {
    return w->isNewTableOptionSelected();
}

void CreateAnnotationWidgetController::setFocusToNameEdit() {
    w->focusAnnotationName();
}

void CreateAnnotationWidgetController::setEnabledNameEdit(bool enbaled) {
    w->setAnnotationNameEnabled(enbaled);
}

bool CreateAnnotationWidgetController::useAutoAnnotationModel() const {
    return w->isAutoTableOptionSelected();
}

void CreateAnnotationWidgetController::setFocusToAnnotationType() {
    w->focusAnnotationType();
}

void CreateAnnotationWidgetController::sl_documentsComboUpdated(){
    commonWidgetUpdate(model);
}

void CreateAnnotationWidgetController::sl_annotationNameEdited(){
    emit si_annotationNamesEdited();
}

void CreateAnnotationWidgetController::sl_groupNameEdited(){
    emit si_annotationNamesEdited();
}

void CreateAnnotationWidgetController::sl_usePatternNamesStateChanged() {
    emit si_usePatternNamesStateChanged();
}

QWidget *CreateAnnotationWidgetController::getWidget() const {
    return w;
}

QPair<QWidget*, QWidget*> CreateAnnotationWidgetController::getTaborderEntryAndExitPoints() const {
    return w->getTabOrderEntryAndExitPoints();
}

} // namespace
