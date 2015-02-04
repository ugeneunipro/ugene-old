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

#include <U2Formats/GenbankLocationParser.h>
#include <U2Formats/GenbankFeatures.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/ShowHideSubgroupWidget.h>

#include "CreateAnnotationWidget.h"
#include "CreateAnnotationWidgetController.h"
#include "GObjectComboBoxController.h"

#define SETTINGS_LASTDIR "create_annotation/last_dir"

namespace U2 {
/* TRANSLATOR U2::CreateAnnotationWidgetController */

CreateAnnotationModel::CreateAnnotationModel() :
    defaultIsNewDoc(false),
    hideLocation(false),
    hideAnnotationName(false),
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

CreateAnnotationWidgetController::CreateAnnotationWidgetController(const CreateAnnotationModel& m,
                                                                   QObject* p,
                                                                   AnnotationWidgetMode layoutMode) :
    QObject(p),
    model(m)
{
    this->setObjectName("CreateAnnotationWidgetController");
    assert(AppContext::getProject()!=NULL);
    assert(model.sequenceObjectRef.isValid());

    w = new CreateAnnotationWidget(layoutMode);

    GObjectComboBoxControllerConstraints occc;
    occc.relationFilter.ref = model.sequenceObjectRef;
    occc.relationFilter.role = ObjectRole_Sequence;
    occc.typeFilter = GObjectTypes::ANNOTATION_TABLE;
    occc.onlyWritable = true;
    occc.uof = model.useUnloadedObjects ? UOF_LoadedAndUnloaded : UOF_LoadedOnly;

    occ = new GObjectComboBoxController(this, occc, w->cbExistingTable);

    commonWidgetUpdate(model);

    connect(w->tbBrowseNewTable, SIGNAL(clicked()), SLOT(sl_onNewDocClicked()));
    connect(w->tbBrowseExistingTable, SIGNAL(clicked()), SLOT(sl_onLoadObjectsClicked()));
    connect(w->tbSelectGroupName, SIGNAL(clicked()), SLOT(sl_groupName()));
    connect(w->tbDoComplement, SIGNAL(clicked()), SLOT(sl_complementLocation()));
    connect(w->leGroupName, SIGNAL(textEdited(const QString&)), SLOT(sl_groupNameEdited()));
    connect(w->leGroupName, SIGNAL(textChanged(const QString&)), SLOT(sl_groupNameEdited()));
    connect(w->leAnnotationName, SIGNAL(textEdited(const QString&)), SLOT(sl_annotationNameEdited()));
    connect(w->leAnnotationName, SIGNAL(textChanged(const QString&)), SLOT(sl_annotationNameEdited()));
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

    connect(occ, SIGNAL(si_comboBoxChanged()), SLOT(sl_documentsComboUpdated()));

    commonWidgetUpdate(newModel);
}


void CreateAnnotationWidgetController::commonWidgetUpdate(const CreateAnnotationModel& model)
{
    if (model.hideLocation) {
        w->lblLocation->hide();
        w->leLocation->hide();
        w->tbDoComplement->hide();
    }
    else {
        w->lblLocation->show();
        w->leLocation->show();
        w->tbDoComplement->show();
    }

    if(model.hideAnnotationName) {
        w->leAnnotationName->hide();
        w->lblAnnotationName->hide();
        w->tbSelectAnnotationName->hide();
    } else {
        QMenu* menu = createAnnotationNamesMenu(w, this);
        w->tbSelectAnnotationName->setMenu(menu);
        w->tbSelectAnnotationName->setPopupMode(QToolButton::InstantPopup);
    }

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
    w->leNewTablePath->setText(url);

    GROUP_NAME_AUTO = CreateAnnotationWidgetController::tr("<auto>");

    if (model.annotationObjectRef.isValid()) {
        occ->setSelectedObject(model.annotationObjectRef);
    } 

    //default field values
    if ( !model.data.name.isEmpty( ) ) {
        w->leAnnotationName->setText( model.data.name );
    } else if (!model.hideAnnotationName && w->leAnnotationName->isEnabled()) {
        //QString name = AppContext::getSettings()->getValue(SETTINGS_LAST_USED_ANNOTATION_NAME, QString("misc_feature")).toString();
        w->leAnnotationName->setText("misc_feature");
    }
    
    w->leGroupName->setText(model.groupName.isEmpty() ? GROUP_NAME_AUTO : model.groupName);

    if ( !model.data.location->isEmpty( ) ) {
        QString locationString = Genbank::LocationParser::buildLocationString( &model.data );
        w->leLocation->setText(locationString);
    }

    if (model.defaultIsNewDoc || w->cbExistingTable->count() == 0) {
        w->rbExistingTable->setCheckable(false);
        w->rbExistingTable->setDisabled(true);
        w->cbExistingTable->setDisabled(true);
        w->tbBrowseExistingTable->setDisabled(true);
        w->rbCreateNewTable->setChecked(true);
    }
    else {
        w->rbExistingTable->setCheckable(true);
        w->rbExistingTable->setDisabled(false);
        w->cbExistingTable->setDisabled(false);
        w->tbBrowseExistingTable->setDisabled(false);
    }

    if (model.hideAutoAnnotationsOption) {
        w->rbUseAutoTable->hide();
        w->rbUseAutoTable->setChecked(false);
    } else {
        w->rbUseAutoTable->show();
        w->rbUseAutoTable->setChecked(true);
    }

    if (model.hideAnnotationParameters) {
        w->annotationParametersWidget->hide();
    } else {
        w->annotationParametersWidget->show();
    }

    w->chbUsePatternNames->setVisible(!model.hideUsePatternNames);

    w->adjustSize();
}

void CreateAnnotationWidgetController::sl_onNewDocClicked() {
    QString openUrl = QFileInfo(w->leNewTablePath->text()).absoluteDir().absolutePath();
    QString filter = DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::PLAIN_GENBANK, false);
    QString name = U2FileDialog::getSaveFileName(NULL, tr("Save file"), openUrl, filter);
    if (!name.isEmpty()) {
        w->leNewTablePath->setText(name);
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
    s.groupMode = ProjectTreeGroupMode_Flat;
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
        QFileInfo fi(model.newDocUrl);
        QString dirUrl = QFileInfo(w->leNewTablePath->text()).absoluteDir().absolutePath();
        QDir dir(dirUrl);
        if (!dir.exists()) {
            return tr("Illegal folder: %1").arg(dirUrl);
        }
    }

    if(w->leAnnotationName->isEnabled() && !w->chbUsePatternNames->isChecked()){ //races is here, so double condition
        if (model.data.name.isEmpty() && !model.hideAnnotationName ) {
            return tr("Annotation name is empty");
        }

        if (!Annotation::isValidAnnotationName(model.data.name) && !model.hideAnnotationName) {
            return tr("Illegal annotation name");
        }
    }

    if (model.groupName.isEmpty()) {
        w->leGroupName->setFocus();
        return tr("Group name is empty");
    }

    if (!AnnotationGroup::isValidGroupName(model.groupName, true)) {
        w->leGroupName->setFocus();
        return tr("Illegal group name");
    }
    
    
    static const QString INVALID_LOCATION = tr("Invalid location! Location must be in GenBank format.\nSimple examples:\n1..10\njoin(1..10,15..45)\ncomplement(5..15)");
    
    if (!model.hideLocation && model.data.location->isEmpty()) {
        w->leLocation->setFocus();
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
    SAFE_POINT(w->leAnnotationName != NULL, "CreateAnnotationWidgetController::updateModel no widget", );
    model.data.name = w->leAnnotationName->text();

    SAFE_POINT(w->leGroupName != NULL, "CreateAnnotationWidgetController::updateModel no widget", );
    model.groupName = w->leGroupName->text();
    if (model.groupName == GROUP_NAME_AUTO) {
        model.groupName = model.data.name;
    }

    model.data.location->reset();
    
    if (!model.hideLocation) {
        SAFE_POINT(w->leLocation != NULL, "CreateAnnotationWidgetController::updateModel no widget", );
        QByteArray locEditText = w->leLocation->text().toLatin1();
        Genbank::LocationParser::parseLocation(	locEditText.constData(),
            w->leLocation->text().length(), model.data.location, model.sequenceLen);
    }

    if (w->rbExistingTable->isChecked()) {
        model.annotationObjectRef = occ->getSelectedObject();
        model.newDocUrl = "";
    } else {
        if (!forValidation){
            model.annotationObjectRef = GObjectReference();
        }
        model.newDocUrl = w->leNewTablePath->text();
    }
}

bool CreateAnnotationWidgetController::prepareAnnotationObject() {
    updateModel(false);
    QString v = validate();
    SAFE_POINT(v.isEmpty(), "Annotation model is not valid", false);
    if (!model.annotationObjectRef.isValid() && w->rbCreateNewTable->isChecked()) {
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

static bool caseInsensitiveLessThan(const QString &s1, const QString &s2) {
    return s1.toLower() < s2.toLower();
}


QMenu* CreateAnnotationWidgetController::createAnnotationNamesMenu(QWidget* p, QObject* receiver) {
    assert(p!=NULL && receiver!=NULL);

    QMenu* m = new QMenu(p);
    const QMultiMap<QString, GBFeatureKey>& nameGroups = GBFeatureUtils::getKeyGroups();
    QStringList groupNames = nameGroups.uniqueKeys();
    qSort(groupNames.begin(), groupNames.end(), caseInsensitiveLessThan);
    foreach(const QString& groupName, groupNames) {
        QMenu* groupMenu = m->addMenu(groupName);
        QList<GBFeatureKey> keys = nameGroups.values(groupName);
        QStringList names;
        foreach(GBFeatureKey k, keys) {
            names.append(GBFeatureUtils::getKeyInfo(k).text);
        }
        qSort(names.begin(), names.end(), caseInsensitiveLessThan);
        foreach(const QString& name, names) {
            QAction* a = new QAction(name, groupMenu);
            connect(a, SIGNAL(triggered()), receiver, SLOT(sl_setPredefinedAnnotationName()));
            groupMenu->addAction(a);
        }
    }
    return m;
}
void CreateAnnotationWidgetController::sl_setPredefinedAnnotationName() {
    QAction* a = qobject_cast<QAction*>(sender());
    QString text = a->text();
    w->leAnnotationName->setText(text);
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
        w->leGroupName->setText(groupNames.first());
        return;
    }
    qSort(groupNames);

    QMenu m(w);
    QPoint menuPos = w->tbSelectGroupName->mapToGlobal(w->tbSelectGroupName->rect().bottomLeft());
    foreach(const QString& str, groupNames) {
        QAction* a = new QAction(str, &m);
        connect(a, SIGNAL(triggered()), SLOT(sl_setPredefinedGroupName()));
        m.addAction(a);
    }
    m.exec(menuPos);
}

void CreateAnnotationWidgetController::sl_setPredefinedGroupName() {
    QAction* a = qobject_cast<QAction*>(sender());
    QString name = a->text();
    w->leGroupName->setText(name);
}

void CreateAnnotationWidgetController::sl_complementLocation() {
    QString text = w->leLocation->text();
    if (text.startsWith("complement(") && text.endsWith(")")) {
        w->leLocation->setText(text.mid(11, text.length()-12));
    } else {
        w->leLocation->setText("complement(" + text + ")");
    }
}

CreateAnnotationWidgetController::~CreateAnnotationWidgetController()
{
    // QWidget* w is deleted recursively by deleteChildren when parent destructor of CreateAnnotationWidgetController is called
}

bool CreateAnnotationWidgetController::isNewObject() const
{
    return w->rbCreateNewTable->isChecked();
}

void CreateAnnotationWidgetController::setFocusToNameEdit()
{
    SAFE_POINT(w->leAnnotationName != NULL, "No annotation name line edit", );
    w->leAnnotationName->setFocus();
}

void CreateAnnotationWidgetController::setEnabledNameEdit( bool enbaled ){
    SAFE_POINT(w->leAnnotationName != NULL, "No annotation name line edit", );
    w->leAnnotationName->setEnabled(enbaled);
}

bool CreateAnnotationWidgetController::useAutoAnnotationModel() const
{
    return w->rbUseAutoTable->isChecked();
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

bool CreateAnnotationWidgetController::getEnabledNameEdit() const {
    return w->leAnnotationName->isEnabled();
}

QWidget *CreateAnnotationWidgetController::getWidget() const {
    return w;
}

QCheckBox *CreateAnnotationWidgetController::getUsePatternNameCheckBox() {
    return w->chbUsePatternNames;
}

QPair<QWidget*, QWidget*> CreateAnnotationWidgetController::getTaborderEntryAndExitPoints() const {
    return QPair<QWidget*, QWidget*>(w->rbExistingTable, w->tbDoComplement);
}

} // namespace
