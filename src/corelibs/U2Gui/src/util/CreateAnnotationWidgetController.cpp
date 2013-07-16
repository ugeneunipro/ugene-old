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

#include "CreateAnnotationWidgetController.h"

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
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

//#define SETTINGS_LAST_USED_ANNOTATION_NAME "create_annotation/last_name"
#define SETTINGS_LASTDIR "create_annotation/last_dir"

namespace U2 {
/* TRANSLATOR U2::CreateAnnotationWidgetController */

CreateAnnotationModel::CreateAnnotationModel() : defaultIsNewDoc(false), hideLocation(false), hideAnnotationName(false) {
    data = new AnnotationData();
    useUnloadedObjects = false;
    hideAutoAnnotationsOption = true;
    hideAnnotationParameters = false;
}

AnnotationTableObject* CreateAnnotationModel::getAnnotationObject() const {
    GObject* res = GObjectUtils::selectObjectByReference(annotationObjectRef, UOF_LoadedOnly);
    AnnotationTableObject* aobj = qobject_cast<AnnotationTableObject*>(res);
    assert(aobj!=NULL);
    return aobj;
}


CreateAnnotationWidgetController::CreateAnnotationWidgetController(
    const CreateAnnotationModel& m,
    QObject* p,
    AnnotationWidgetMode layoutMode)
: QObject(p), model(m)
{
    this->setObjectName("CreateAnnotationWidgetController");
    assert(AppContext::getProject()!=NULL);
    assert(model.sequenceObjectRef.isValid());
    w = new QWidget();
    initLayout(layoutMode);

    GObjectComboBoxControllerConstraints occc;
    occc.relationFilter.ref = model.sequenceObjectRef;
    occc.relationFilter.role = GObjectRelationRole::SEQUENCE;
    occc.typeFilter = GObjectTypes::ANNOTATION_TABLE;
    occc.onlyWritable = true;
    occc.uof = model.useUnloadedObjects ? UOF_LoadedAndUnloaded : UOF_LoadedOnly;

    occ = new GObjectComboBoxController(this, occc, existingObjectCombo);

    commonWidgetUpdate(model);

    connect(newFileButton, SIGNAL(clicked()), SLOT(sl_onNewDocClicked()));
    connect(existingObjectButton, SIGNAL(clicked()), SLOT(sl_onLoadObjectsClicked()));
    connect(groupNameButton, SIGNAL(clicked()), SLOT(sl_groupName()));
    connect(complementButton, SIGNAL(clicked()), SLOT(sl_complementLocation()));
}


void CreateAnnotationWidgetController::updateWidgetForAnnotationModel(const CreateAnnotationModel& newModel)
{
    SAFE_POINT(newModel.sequenceObjectRef.isValid(),
        "Internal error: incorrect sequence object reference was supplied"
        "to the annotation widget controller.",);

    model = newModel;

    GObjectComboBoxControllerConstraints occc;
    occc.relationFilter.ref = newModel.sequenceObjectRef;
    occc.relationFilter.role = GObjectRelationRole::SEQUENCE;
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
        locationLabel->hide();
        locationEdit->hide();
        complementButton->hide();
    }
    else {
        locationLabel->show();
        locationEdit->show();
        complementButton->show();
    }

    if(model.hideAnnotationName) {
        annotationNameEdit->hide();
        annotationNameLabel->hide();
        showNameGroupsButton->hide();
    } else {
        QMenu* menu = createAnnotationNamesMenu(w, this);
        showNameGroupsButton->setMenu(menu);
        showNameGroupsButton->setPopupMode(QToolButton::InstantPopup);
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
    newFileEdit->setText(url);

    GROUP_NAME_AUTO = CreateAnnotationWidgetController::tr("<auto>");

    if (model.annotationObjectRef.isValid()) {
        occ->setSelectedObject(model.annotationObjectRef);
    } 

    //default field values
    if (!model.data->name.isEmpty()) {
        annotationNameEdit->setText(model.data->name);
    } else if (!model.hideAnnotationName) {
        //QString name = AppContext::getSettings()->getValue(SETTINGS_LAST_USED_ANNOTATION_NAME, QString("misc_feature")).toString();
        annotationNameEdit->setText("misc_feature");
    }
    
    groupNameEdit->setText(model.groupName.isEmpty() ? GROUP_NAME_AUTO : model.groupName);

    if (!model.data->location->isEmpty()) {
        QString locationString = Genbank::LocationParser::buildLocationString(model.data);
        locationEdit->setText(locationString);
    }

    if (model.defaultIsNewDoc || existingObjectCombo->count() == 0) {
        existingObjectRB->setCheckable(false);
        existingObjectRB->setDisabled(true);
        existingObjectCombo->setDisabled(true);
        existingObjectButton->setDisabled(true);
        newFileRB->setChecked(true);
    }
    else {
        existingObjectRB->setCheckable(true);
        existingObjectRB->setDisabled(false);
        existingObjectCombo->setDisabled(false);
        existingObjectButton->setDisabled(false);
    }

    if (model.hideAutoAnnotationsOption) {
        useAutoAnnotationsRB->hide();
        useAutoAnnotationsRB->setChecked(false);
    } else {
        useAutoAnnotationsRB->show();
        useAutoAnnotationsRB->setChecked(true);
    }

    if (model.hideAnnotationParameters) {
        groupAnnotParams->hide();
    } else {
        groupAnnotParams->show();
    }
}


void CreateAnnotationWidgetController::initLayout(AnnotationWidgetMode layoutMode)
{
    QSizePolicy sizePolicy;

    if (layoutMode == normal) {
        w->resize(492, 218);
        sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        w->setMinimumSize(QSize(400, 58));
    } else if (layoutMode == optPanel) {
        w->resize(170, 369);
        sizePolicy = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        w->setMinimumSize(QSize(1, 0));
    }else{
        w->resize(170, 369);
        sizePolicy = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        w->setMinimumSize(QSize(170, 0));
    }
    w->setSizePolicy(sizePolicy);

    w->setWindowTitle(tr("Create Annotations"));

    QVBoxLayout* mainLayout = new QVBoxLayout(w);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Save annotations group
    QGroupBox* groupSaveAnnots = new QGroupBox(w);
    groupSaveAnnots->setTitle(tr("Save annotation(s) to"));
    QGridLayout* gridLayout = new QGridLayout(groupSaveAnnots);

    existingObjectRB = new QRadioButton(groupSaveAnnots);
    existingObjectRB ->setObjectName("existingObjectRB");

    existingObjectRB->setChecked(true);
    existingObjectRB->setText(tr("Existing table"));
    existingObjectCombo = new QComboBox(groupSaveAnnots);
    existingObjectCombo->setObjectName("existingObjectCombo");

    existingObjectButton = new QToolButton(groupSaveAnnots);
    existingObjectButton->setObjectName("existingObjectButton");

    QIcon loadSelectedDocIcon;
    loadSelectedDocIcon.addFile(
        QString::fromUtf8(":/core/images/load_selected_documents.png"),
        QSize(),
        QIcon::Normal,
        QIcon::Off);
    existingObjectButton->setIcon(loadSelectedDocIcon);
    existingObjectButton->setText("...");
    newFileRB = new QRadioButton(groupSaveAnnots);
    newFileRB->setObjectName("newFileRB");
    newFileRB->setText(tr("Create new table"));
    newFileEdit = new QLineEdit(groupSaveAnnots);
    newFileEdit->setEnabled(false);
    newFileEdit->setObjectName("newFilePath");
    newFileButton = new QToolButton(groupSaveAnnots);
    newFileButton->setEnabled(false);
    newFileButton->setText("...");
    useAutoAnnotationsRB = new QRadioButton(groupSaveAnnots);
    useAutoAnnotationsRB->setText(tr("Use auto-annotations table"));

    if (layoutMode == normal) {
        mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

        gridLayout->addWidget(existingObjectRB, 1, 0, 1, 1);
        gridLayout->addWidget(existingObjectCombo, 1, 1, 1, 1);
        gridLayout->addWidget(existingObjectButton, 1, 2, 1, 1);
        gridLayout->addWidget(newFileRB, 2, 0, 1, 1);
        gridLayout->addWidget(newFileEdit, 2, 1, 1, 1);
        gridLayout->addWidget(newFileButton, 2, 2, 1, 1);
        gridLayout->addWidget(useAutoAnnotationsRB, 3, 0, 1, 1);
    }
    else {
        QSpacerItem* horizontalSpacer = new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
        QHBoxLayout* layoutExistAnnot = new QHBoxLayout();

        layoutExistAnnot->addItem(horizontalSpacer);
        layoutExistAnnot->addWidget(existingObjectCombo);
        layoutExistAnnot->addWidget(existingObjectButton);

        QSpacerItem* horizontalSpacer2 = new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
        QHBoxLayout* layoutCreateTable = new QHBoxLayout();

        layoutCreateTable->addItem(horizontalSpacer2);
        layoutCreateTable->addWidget(newFileEdit);
        layoutCreateTable->addWidget(newFileButton);

        gridLayout->addWidget(existingObjectRB, 0, 0, 1, 1);
        gridLayout->addLayout(layoutExistAnnot, 1, 0, 1, 1);
        gridLayout->addWidget(newFileRB, 2, 0, 1, 1);
        gridLayout->addLayout(layoutCreateTable, 3, 0, 1, 1);
        gridLayout->addWidget(useAutoAnnotationsRB, 4, 0, 1, 1);
    }

    // Annotation parameters group
    groupAnnotParams = new QGroupBox(w);
    groupAnnotParams->setTitle(tr("Annotation parameters"));

    QGridLayout* gridLayoutParams = new QGridLayout(groupAnnotParams);
    groupNameLabel = new QLabel(groupAnnotParams);
    QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(groupNameLabel->sizePolicy().hasHeightForWidth());
    groupNameLabel->setSizePolicy(sizePolicy1);

    groupNameEdit = new QLineEdit(groupAnnotParams);
    groupNameEdit->setObjectName("groupNameEdit");
    connect(groupNameEdit, SIGNAL(textEdited(const QString&)), SLOT(sl_groupNameEdited(const QString&)));
    connect(groupNameEdit, SIGNAL(textChanged(const QString&)), SLOT(sl_groupNameEdited(const QString&)));

    groupNameButton = new QToolButton(groupAnnotParams);
    QIcon iconGroupAuto;
    iconGroupAuto.addFile(QString::fromUtf8(":/core/images/group_auto.png"), QSize(), QIcon::Normal, QIcon::Off);
    groupNameButton->setText("...");
    groupNameButton->setToolTip(tr("Predefined group names"));
    groupNameButton->setIcon(iconGroupAuto);

    annotationNameLabel = new QLabel(groupAnnotParams);

    annotationNameEdit = new QLineEdit(groupAnnotParams);
    annotationNameEdit->setObjectName("annotationNameEdit");
    connect(annotationNameEdit, SIGNAL(textEdited(const QString&)), SLOT(sl_annotationNameEdited(const QString&)));
    connect(annotationNameEdit, SIGNAL(textChanged(const QString&)), SLOT(sl_annotationNameEdited(const QString&)));

    annotationNameEdit->setMaxLength(100);

    showNameGroupsButton = new QToolButton(groupAnnotParams);
    QIcon iconPredefAnnot;
    iconPredefAnnot.addFile(QString::fromUtf8(":/core/images/predefined_annotation_groups.png"), QSize(), QIcon::Normal, QIcon::Off);
    showNameGroupsButton->setIcon(iconPredefAnnot);
    showNameGroupsButton->setToolTip(tr("Predefined annotation names"));
    showNameGroupsButton->setText(QString());
    
    locationLabel = new QLabel(groupAnnotParams);

    locationEdit = new QLineEdit(groupAnnotParams);
    locationEdit->setObjectName("locationEdit");

    locationEdit->setToolTip(tr("Annotation location in GenBank format"));

    complementButton = new QToolButton(groupAnnotParams);
    QIcon iconComplement;
    iconComplement.addFile(QString::fromUtf8(":/core/images/do_complement.png"), QSize(), QIcon::Normal, QIcon::Off);
    complementButton->setToolTip(tr("Add/remove complement flag"));
    complementButton->setText("...");
    complementButton->setIcon(iconComplement);

    if (layoutMode ==  normal) {
        gridLayoutParams->setSizeConstraint(QLayout::SetMinAndMaxSize);

        groupNameLabel->setText(tr("Group name"));
        annotationNameLabel->setText(tr("Annotation name"));
        locationLabel->setText(tr("Location"));

        gridLayoutParams->addWidget(groupNameLabel, 0, 0, 1, 1);
        gridLayoutParams->addWidget(groupNameEdit, 0, 1, 1, 1);
        gridLayoutParams->addWidget(groupNameButton, 0, 2, 1, 1);
        gridLayoutParams->addWidget(annotationNameLabel, 1, 0, 1, 1);
        gridLayoutParams->addWidget(annotationNameEdit, 1, 1, 1, 1);
        gridLayoutParams->addWidget(showNameGroupsButton, 1, 2, 1, 1);
        gridLayoutParams->addWidget(locationLabel, 2, 0, 1, 1);
        gridLayoutParams->addWidget(locationEdit, 2, 1, 1, 1);
        gridLayoutParams->addWidget(complementButton, 2, 2, 1, 1);
    }
    else {
        groupNameLabel->setText(tr("Group name:"));
        annotationNameLabel->setText(tr("Annotation name:"));
        locationLabel->setText(tr("Location:"));

        gridLayoutParams->addWidget(groupNameLabel, 0, 0, 1, 1);
        gridLayoutParams->addWidget(groupNameEdit, 1, 0, 1, 1);
        gridLayoutParams->addWidget(groupNameButton, 1, 1, 1, 1);
        gridLayoutParams->addWidget(annotationNameLabel, 2, 0, 1, 1);
        gridLayoutParams->addWidget(annotationNameEdit, 3, 0, 1, 1);
        gridLayoutParams->addWidget(showNameGroupsButton, 3, 1, 1, 1);
        gridLayoutParams->addWidget(locationLabel, 4, 0, 1, 1);
        gridLayoutParams->addWidget(locationEdit, 5, 0, 1, 1);
        gridLayoutParams->addWidget(complementButton, 5, 1, 1, 1);
    }

    // Set layout, connect slots
    mainLayout->addWidget(groupSaveAnnots);
    mainLayout->addWidget(groupAnnotParams);
    w->setLayout(mainLayout);

    QWidget::setTabOrder(groupNameEdit, groupNameButton);
    QWidget::setTabOrder(groupNameButton, annotationNameEdit);
    QWidget::setTabOrder(annotationNameEdit, showNameGroupsButton);
    QWidget::setTabOrder(showNameGroupsButton, locationEdit);

    QObject::connect(newFileRB, SIGNAL(toggled(bool)), newFileEdit, SLOT(setEnabled(bool)));
    QObject::connect(newFileRB, SIGNAL(toggled(bool)), newFileButton, SLOT(setEnabled(bool)));
    QObject::connect(existingObjectRB, SIGNAL(toggled(bool)), existingObjectCombo, SLOT(setEnabled(bool)));
    QObject::connect(existingObjectRB, SIGNAL(toggled(bool)), existingObjectButton, SLOT(setEnabled(bool)));
    QObject::connect(useAutoAnnotationsRB, SIGNAL(toggled(bool)), groupAnnotParams, SLOT(setDisabled(bool)));
}


void CreateAnnotationWidgetController::sl_onNewDocClicked() {
    QString openUrl = QFileInfo(newFileEdit->text()).absoluteDir().absolutePath();
    QString filter = DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::PLAIN_GENBANK, false);
    QString name = QFileDialog::getSaveFileName(NULL, tr("Save file"), openUrl, filter);
    if (!name.isEmpty()) {
        newFileEdit->setText(name);
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
    updateModel(true);
    if (!model.annotationObjectRef.isValid()) {
        if (model.newDocUrl.isEmpty()) {
            return tr("Select annotation saving parameters");
        }
        if (AppContext::getProject()->findDocumentByURL(model.newDocUrl)!=NULL) {
            return tr("Document is already added to the project: '%1'").arg(model.newDocUrl);
        }
        QFileInfo fi(model.newDocUrl);
        QString dirUrl = QFileInfo(newFileEdit->text()).absoluteDir().absolutePath();
        QDir dir(dirUrl);
        if (!dir.exists()) {
            return tr("Illegal folder: %1").arg(dirUrl);
        }
    }

    if (model.data->name.isEmpty() && !model.hideAnnotationName ) {
        annotationNameEdit->setFocus();
        return tr("Annotation name is empty");
    }

    if (model.data->name.length() > GBFeatureUtils::MAX_KEY_LEN) {
        annotationNameEdit->setFocus();
        return tr("Annotation name is too long!\nMaximum allowed size: %1 (Genbank format compatibility issue)").arg(GBFeatureUtils::MAX_KEY_LEN);
    }

    if (!Annotation::isValidAnnotationName(model.data->name) && !model.hideAnnotationName) {
        annotationNameEdit->setFocus();
        return tr("Illegal annotation name");
    }

    if (model.groupName.isEmpty()) {
        groupNameEdit->setFocus();
        return tr("Group name is empty");
    }

    if (!AnnotationGroup::isValidGroupName(model.groupName, true)) {
        groupNameEdit->setFocus();
        return tr("Illegal group name");
    }
    
    
    static const QString INVALID_LOCATION = tr("Invalid location! Location must be in GenBank format.\nSimple examples:\n1..10\njoin(1..10,15..45)\ncomplement(5..15)");
    
    if (!model.hideLocation && model.data->location->isEmpty()) {
        locationEdit->setFocus();
        return INVALID_LOCATION;
    }
    if (!model.hideLocation){
        foreach(U2Region reg, model.data->getRegions()){
            if( reg.endPos() > model.sequenceLen || reg.startPos < 0 || reg.endPos() < reg.startPos) {
                return INVALID_LOCATION;
            }
        }
    }
    
//    AppContext::getSettings()->setValue(SETTINGS_LAST_USED_ANNOTATION_NAME, model.data->name);

    return QString::null;
}


void CreateAnnotationWidgetController::updateModel(bool forValidation) {
    SAFE_POINT(model.data != NULL, "Model data is null", );

    model.data->name = annotationNameEdit->text();

    model.groupName = groupNameEdit->text();
    if (model.groupName == GROUP_NAME_AUTO) {
        model.groupName = model.data->name;
    }

    model.data->location->reset();
    
    if (!model.hideLocation) {
        QByteArray locEditText = locationEdit->text().toLatin1();
        Genbank::LocationParser::parseLocation(	locEditText.constData(), locationEdit->text().length(), model.data->location, model.sequenceLen);
    }

    if (existingObjectRB->isChecked()) {
        model.annotationObjectRef = occ->getSelectedObject();
        model.newDocUrl = "";
    } else {
        if (!forValidation){
            model.annotationObjectRef = GObjectReference();
        }
        model.newDocUrl = newFileEdit->text();
    }
}

bool CreateAnnotationWidgetController::prepareAnnotationObject() {
    updateModel(false);
    QString v = validate();
    SAFE_POINT(v.isEmpty(), "Annotation model is not valid", false);
    if (!model.annotationObjectRef.isValid() && newFileRB->isChecked()) {
        SAFE_POINT(!model.newDocUrl.isEmpty(), "newDocUrl is empty", false);
        SAFE_POINT(AppContext::getProject()->findDocumentByURL(model.newDocUrl)==NULL, "cannot create a document that is already in the project", false);
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
        U2OpStatus2Log os;
        Document* d = df->createNewLoadedDocument(iof, model.newDocUrl, os);
        CHECK_OP(os, false);
        AnnotationTableObject* aobj = new AnnotationTableObject("Annotations");
        aobj->addObjectRelation(GObjectRelation(model.sequenceObjectRef, GObjectRelationRole::SEQUENCE));
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
    annotationNameEdit->setText(text);
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
        groupNameEdit->setText(groupNames.first());
        return;
    }
    qSort(groupNames);

    QMenu m(w);
    QPoint menuPos = groupNameButton->mapToGlobal(groupNameButton->rect().bottomLeft());
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
    groupNameEdit->setText(name);
}

void CreateAnnotationWidgetController::sl_complementLocation() {
    QString text = locationEdit->text();
    if (text.startsWith("complement(") && text.endsWith(")")) {
        locationEdit->setText(text.mid(11, text.length()-12));
    } else {
        locationEdit->setText("complement(" + text + ")");
    }
}

CreateAnnotationWidgetController::~CreateAnnotationWidgetController()
{
    // QWidget* w is deleted recursively by deleteChildren when parent destructor of CreateAnnotationWidgetController is called
}

bool CreateAnnotationWidgetController::isNewObject() const
{
    return newFileRB->isChecked();
}

void CreateAnnotationWidgetController::setFocusToNameEdit()
{
    SAFE_POINT(annotationNameEdit != NULL, "No annotation name line edit", );
    annotationNameEdit->setFocus();
}

void CreateAnnotationWidgetController::setEnabledNameEdit( bool enbaled ){
    SAFE_POINT(annotationNameEdit != NULL, "No annotation name line edit", );
    annotationNameEdit->setEnabled(enbaled);
}

bool CreateAnnotationWidgetController::useAutoAnnotationModel() const
{
    return useAutoAnnotationsRB->isChecked();
}

void CreateAnnotationWidgetController::sl_documentsComboUpdated(){
    commonWidgetUpdate(model);
}

void CreateAnnotationWidgetController::sl_annotationNameEdited( const QString& text ){
    emit si_annotationNamesEdited();
}

void CreateAnnotationWidgetController::sl_groupNameEdited( const QString& text ){
    emit si_annotationNamesEdited();
}


} // namespace
