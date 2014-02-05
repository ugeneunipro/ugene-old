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

#ifndef _U2_CREATE_ANNOTATION_WIDGET_CONTROLLER_H_
#define _U2_CREATE_ANNOTATION_WIDGET_CONTROLLER_H_

#include <U2Core/AnnotationData.h>
#include <U2Core/U2Region.h>
#include <U2Core/DocumentModel.h>

#include <QtGui/QMenu>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QToolButton>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>

#include <U2Gui/ShowHideSubgroupWidget.h>

namespace U2 {

class GObjectComboBoxController;
class AnnotationTableObject;

class U2GUI_EXPORT CreateAnnotationModel {
public:
    CreateAnnotationModel();

    GObjectReference        sequenceObjectRef;  // this object is selected by default
    bool                    defaultIsNewDoc;    //new doc field is selected by default

    bool                    hideLocation;       // hides location field and does not check it in validate()
    bool                    hideAnnotationName; // hides annotation name field
    bool                    useUnloadedObjects;
    QString                 groupName;          // default groupname. If empty -> <auto> value is used (annotationObject->name value).
    AnnotationData          data;               // holds name and location of the annotation

    GObjectReference        annotationObjectRef; // the object to be loaded
    QString                 newDocUrl;        // the URL of new document with annotation table to be created
    qint64                  sequenceLen;        //length of target sequence for validation purposes

    bool                    hideAutoAnnotationsOption; // show automated highlighting for new annotation if possible
    bool                    hideAnnotationParameters;   // hides annotation parameters groupbox

    AnnotationTableObject *   getAnnotationObject( ) const;
};

// Layout mode of the annotation widget
enum AnnotationWidgetMode {normal, compact, optPanel};

class U2GUI_EXPORT CreateAnnotationWidgetController : public QObject {
Q_OBJECT
public:
    
    // useCompact defines the layout of the widget (normal or compact for the Options Panel)
    CreateAnnotationWidgetController(const CreateAnnotationModel& m, QObject* p, AnnotationWidgetMode layoutMode = normal);
    ~CreateAnnotationWidgetController();
    
    // returns error message or empty string if no error found;
    // does not create any new objects
    QString validate(); 

    // Ensures that annotationObeject is valid
    // for a validated model creates new document (newDocUrl), adds annotation table object 
    // to the document created and stores the reference to the object to annotationObject
    // Does nothing if annotationObject is not NULL
    // returns true if annotation object is prepared
    // return false otherwise
    bool prepareAnnotationObject();

    // add model property instead ??
    bool isNewObject() const;

    // property of GUI
    bool useAutoAnnotationModel() const;

    void setFocusToNameEdit();

    void setEnabledNameEdit(bool enbaled);

    QWidget* getWidget() const {return w;}

    const CreateAnnotationModel&    getModel() const {return model;}

    QCheckBox* getUsePatternNameCheckBox() { return usePatternNamesCheckBox; }
    
    //receiver object must have sl_setPredefinedAnnotationName(), TODO: move this utility to a separate class
    static QMenu* createAnnotationNamesMenu(QWidget* p, QObject* receiver);

    void updateWidgetForAnnotationModel(const CreateAnnotationModel& model);

    /** It is called from the constructor and updateWidgetForAnnotationModel(...) */
    void commonWidgetUpdate(const CreateAnnotationModel& model);

    void setUsePatternNameCheckBoxVisible() { usePatternNamesCheckBox->setVisible(true); }

signals:
    void si_annotationNamesEdited();

private slots:
    void sl_onNewDocClicked();
    void sl_onLoadObjectsClicked();
    void sl_setPredefinedAnnotationName();
    void sl_groupName();
    void sl_complementLocation();
    void sl_setPredefinedGroupName();

    void sl_documentsComboUpdated();

    //edit slots
    void sl_annotationNameEdited();
    void sl_groupNameEdited();

private:
    void initLayout(AnnotationWidgetMode layoutMode);
    void updateModel(bool forValidation);

    CreateAnnotationModel       model;
    GObjectComboBoxController*  occ;
    QWidget*                    w;
    
    QString GROUP_NAME_AUTO;

    // Widget layout
    ShowHideSubgroupWidget* annotParamsWidget;
    QLineEdit* newFileEdit;
    QLineEdit* annotationNameEdit;
    QLineEdit* groupNameEdit;
    QLineEdit* locationEdit;
    QRadioButton* newFileRB;
    QRadioButton* existingObjectRB;
    QRadioButton* useAutoAnnotationsRB;
    QToolButton* groupNameButton;
    QToolButton* existingObjectButton;
    QToolButton* showNameGroupsButton;
    QToolButton* complementButton;
    QToolButton* newFileButton;
    QComboBox* existingObjectCombo;
    QLabel* annotationNameLabel;
    QLabel* groupNameLabel;
    QLabel* locationLabel;
    QCheckBox* usePatternNamesCheckBox;
};

} // namespace U2

#endif
