/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "CreateAnnotationWidgetController.h"
#include <ui/ui_CreateAnnotationWidget.h>

#include "GObjectComboBoxController.h"
#include <U2Gui/DialogUtils.h>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/Settings.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>


#include <U2Formats/GenbankLocationParser.h>
#include <U2Formats/GenbankFeatures.h>

#include <U2Core/GObjectUtils.h>

#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/GUIUtils.h>
#include <U2Core/TextUtils.h>

#include <memory>

#include <QtGui/QFileDialog>

//#define SETTINGS_LAST_USED_ANNOTATION_NAME "create_annotation/last_name"
#define SETTINGS_LASTDIR "create_annotation/last_dir"

namespace U2 {
/* TRANSLATOR U2::CreateAnnotationWidgetController */

CreateAnnotationModel::CreateAnnotationModel() : defaultIsNewDoc(false), hideLocation(false), hideAnnotationName(false) {
	data = new AnnotationData();
    useUnloadedObjects = false;
}

AnnotationTableObject* CreateAnnotationModel::getAnnotationObject() const {
    GObject* res = GObjectUtils::selectObjectByReference(annotationObjectRef, UOF_LoadedOnly);
    AnnotationTableObject* aobj = qobject_cast<AnnotationTableObject*>(res);
    assert(aobj!=NULL);
    return aobj;
}


CreateAnnotationWidgetController::CreateAnnotationWidgetController(const CreateAnnotationModel& m, QObject* p)
: QObject(p), model(m)
{
    this->setObjectName("CreateAnnotationWidgetController");
    assert(AppContext::getProject()!=NULL);
    assert(model.sequenceObjectRef.isValid());
    w = new QWidget();
    ui = new Ui_CreateAnnotationWidget;
    ui->setupUi(w);

    if (model.hideLocation) {
        ui->locationLabel->hide();
        ui->locationEdit->hide();
        ui->complementButton->hide();
    }
    if( model.hideAnnotationName ) {
        ui->annotationNameEdit->hide();
        ui->annotationNameLabel->hide();
        ui->showNameGroupsButton->hide();
    } else {
        QMenu* menu = createAnnotationNamesMenu(w, this);
        ui->showNameGroupsButton->setMenu(menu);
        ui->showNameGroupsButton->setPopupMode(QToolButton::InstantPopup);
    }


    QString dir = AppContext::getSettings()->getValue(SETTINGS_LASTDIR).toString();
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
    ui->newFileEdit->setText(url);

    GROUP_NAME_AUTO = CreateAnnotationWidgetController::tr("<auto>");

    GObjectComboBoxControllerConstraints occc;
    occc.relationFilter.ref = model.sequenceObjectRef;
    occc.relationFilter.role = GObjectRelationRole::SEQUENCE;
    occc.typeFilter = GObjectTypes::ANNOTATION_TABLE;
    occc.onlyWritable = true;
    occc.uof = model.useUnloadedObjects ? UOF_LoadedAndUnloaded : UOF_LoadedOnly;
    occ = new GObjectComboBoxController(this, occc, ui->existingObjectCombo);
    if (model.annotationObjectRef.isValid()) {
        occ->setSelectedObject(model.annotationObjectRef);
    } 

    //default field values
    if (!model.data->name.isEmpty()) {
        ui->annotationNameEdit->setText(model.data->name);
    } else if (!model.hideAnnotationName) {
        //QString name = AppContext::getSettings()->getValue(SETTINGS_LAST_USED_ANNOTATION_NAME, QString("misc_feature")).toString();
        ui->annotationNameEdit->setText("misc_feature");
    }
    ui->annotationNameEdit->selectAll();

    ui->groupNameEdit->setText(model.groupName.isEmpty() ? GROUP_NAME_AUTO : model.groupName);

    if (!model.data->location->isEmpty()) {
        QString locationString = Genbank::LocationParser::buildLocationString(model.data);
        ui->locationEdit->setText(locationString);
    }

    if (model.defaultIsNewDoc || ui->existingObjectCombo->count() == 0) {
        ui->existingObjectRB->setCheckable(false);
        ui->existingObjectRB->setDisabled(true);
        ui->existingObjectCombo->setDisabled(true);
        ui->existingObjectButton->setDisabled(true);
        ui->newFileRB->setChecked(true);
    }

    connect(ui->newFileButton, SIGNAL(clicked()), SLOT(sl_onNewDocClicked()));
    connect(ui->existingObjectButton, SIGNAL(clicked()), SLOT(sl_onLoadObjectsClicked()));
    connect(ui->groupNameButton, SIGNAL(clicked()), SLOT(sl_groupName()));
    connect(ui->complementButton, SIGNAL(clicked()), SLOT(sl_complementLocation()));
}

void CreateAnnotationWidgetController::sl_onNewDocClicked() {
    QString openUrl = QFileInfo(ui->newFileEdit->text()).absoluteDir().absolutePath();
    QString filter = DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::PLAIN_GENBANK, false);
    QString name = QFileDialog::getSaveFileName(NULL, tr("Save file"), openUrl, filter);
    if (!name.isEmpty()) {
        ui->newFileEdit->setText(name);
        AppContext::getSettings()->setValue(SETTINGS_LASTDIR, QFileInfo(name).absoluteDir().absolutePath());
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
        assert(qobject_cast<AnnotationTableObject*>(obj)!=NULL);
        return obj->isStateLocked();
    }
    bool allowUnloaded;
};


void CreateAnnotationWidgetController::sl_onLoadObjectsClicked() {
    ProjectTreeControllerModeSettings s;
    s.allowMultipleSelection = false;
    s.objectTypesToShow.append(GObjectTypes::ANNOTATION_TABLE);
    s.groupMode = ProjectTreeGroupMode_Flat;
    GObjectRelation rel(model.sequenceObjectRef, GObjectRelationRole::SEQUENCE);
    std::auto_ptr<PTCAnnotationObjectFilter> filter(new PTCAnnotationObjectFilter(rel, model.useUnloadedObjects));
    s.objectFilter = filter.get();
    QList<GObject*> objs = ProjectTreeItemSelectorDialog::selectObjects(s, w);
    if (objs.isEmpty()) {
        return;
    }
    assert(objs.size() == 1);
    GObject* obj = objs.first();
    occ->setSelectedObject(obj);
}

QString CreateAnnotationWidgetController::validate() {
	updateModel();

	if (!model.annotationObjectRef.isValid()) {
        if (model.newDocUrl.isEmpty()) {
            return tr("Select annotation saving parameters");
        }
        if (AppContext::getProject()->findDocumentByURL(model.newDocUrl)!=NULL) {
            return tr("Document is already added to the project: '%1'").arg(model.newDocUrl);
        }
        QFileInfo fi(model.newDocUrl);
        QString dirUrl = QFileInfo(ui->newFileEdit->text()).absoluteDir().absolutePath();
        QDir dir(dirUrl);
        if (!dir.exists()) {
            return tr("Illegal folder: %1").arg(dirUrl);
        }
    }

	if (model.data->name.isEmpty() && !model.hideAnnotationName ) {
        ui->annotationNameEdit->setFocus();
		return tr("Annotation name is empty");
	}

    if (model.data->name.length() > GBFeatureUtils::MAX_KEY_LEN) {
        ui->annotationNameEdit->setFocus();
        return tr("Annotation name is too long!\nMaximum allowed size: %1 (Genbank format compatibility issue)").arg(GBFeatureUtils::MAX_KEY_LEN);
	}

    if (!Annotation::isValidAnnotationName(model.data->name) && !model.hideAnnotationName) {
        ui->annotationNameEdit->setFocus();
        return tr("Illegal annotation name");
    }

	if (model.groupName.isEmpty()) {
        ui->groupNameEdit->setFocus();
		return tr("Group name is empty");
	}

    if (!AnnotationGroup::isValidGroupName(model.groupName, true)) {
        ui->groupNameEdit->setFocus();
        return tr("Illegal group name");
    }
    
    
    static const QString INVALID_LOCATION = tr("Invalid location! Location must be in GenBank format.\nSimple examples:\n1..10\njoin(1..10,15..45)\ncomplement(5..15)");
    
    if (!model.hideLocation && model.data->location->isEmpty()) {
        ui->locationEdit->setFocus();
		return INVALID_LOCATION;
	}
    foreach(U2Region reg, model.data->getRegions()){
        if( reg.endPos() > model.sequenceLen || reg.startPos < 0 || reg.endPos() < reg.startPos) {
            return INVALID_LOCATION;
        }
    }

//    AppContext::getSettings()->setValue(SETTINGS_LAST_USED_ANNOTATION_NAME, model.data->name);

	return QString::null;
}


void CreateAnnotationWidgetController::updateModel() {
	model.data->name = ui->annotationNameEdit->text();

	model.groupName = ui->groupNameEdit->text();
	if (model.groupName == GROUP_NAME_AUTO) {
		model.groupName = model.data->name;
	}

	model.data->location->reset();
	
    if (!model.hideLocation) {
        QByteArray locEditText = ui->locationEdit->text().toAscii();
        Genbank::LocationParser::parseLocation(	locEditText.constData(), ui->locationEdit->text().length(), model.data->location);
    }

    if (ui->existingObjectRB->isChecked()) {
        model.annotationObjectRef = occ->getSelectedObject();
        model.newDocUrl = "";
    } else {
        model.annotationObjectRef = GObjectReference();
        model.newDocUrl = ui->newFileEdit->text();
    }
}

void CreateAnnotationWidgetController::prepareAnnotationObject() {
    QString v = validate();
    assert(v.isEmpty());
    if (!model.annotationObjectRef.isValid() && ui->newFileRB->isChecked()) {
        assert(!model.newDocUrl.isEmpty());
        assert(AppContext::getProject()->findDocumentByURL(model.newDocUrl)==NULL);
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
        U2OpStatus2Log os;
        Document* d = df->createNewLoadedDocument(iof, model.newDocUrl, os);
        CHECK_OP(os, );
        AnnotationTableObject* aobj = new AnnotationTableObject("Annotations");
        aobj->addObjectRelation(GObjectRelation(model.sequenceObjectRef, GObjectRelationRole::SEQUENCE));
        d->addObject(aobj);
        AppContext::getProject()->addDocument(d);
        model.annotationObjectRef = aobj;
    }
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
    ui->annotationNameEdit->setText(text);
}

void CreateAnnotationWidgetController::sl_groupName() {
    GObject* obj = occ->getSelectedObject();
    QStringList groupNames; 
    groupNames << GROUP_NAME_AUTO;
    if (obj != NULL && !obj->isUnloaded()) {
        AnnotationTableObject* ao = qobject_cast<AnnotationTableObject*>(obj);
        ao->getRootGroup()->getSubgroupPaths(groupNames);
    }
    assert(groupNames.size() >= 1);
    if (groupNames.size() == 1) {
        ui->groupNameEdit->setText(groupNames.first());
        return;
    }
    qSort(groupNames);

    QMenu m(w);
    QPoint menuPos = ui->groupNameButton->mapToGlobal(ui->groupNameButton->rect().bottomLeft());
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
    ui->groupNameEdit->setText(name);
}

void CreateAnnotationWidgetController::sl_complementLocation() {
    QString text = ui->locationEdit->text();
    if (text.startsWith("complement(") && text.endsWith(")")) {
        ui->locationEdit->setText(text.mid(11, text.length()-12));
    } else {
        ui->locationEdit->setText("complement(" + text + ")");
    }
}

CreateAnnotationWidgetController::~CreateAnnotationWidgetController()
{
    delete ui;
}

bool CreateAnnotationWidgetController::isNewObject() const
{
    return ui->newFileRB->isChecked();
}

void CreateAnnotationWidgetController::setFocusToNameEdit()
{
    ui->annotationNameEdit->setFocus();
}
} // namespace
