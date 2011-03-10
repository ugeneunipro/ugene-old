#ifndef _U2_CREATE_ANNOTATION_WIDGET_CONTROLLER_H_
#define _U2_CREATE_ANNOTATION_WIDGET_CONTROLLER_H_

#include <U2Core/U2Region.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/AnnotationTableObject.h>

#include <QtGui/QMenu>

class Ui_CreateAnnotationWidget;

namespace U2 {

class GObjectComboBoxController;


class U2GUI_EXPORT CreateAnnotationModel {
public:
	CreateAnnotationModel();

    GObjectReference        sequenceObjectRef;  // this object is selected by default
    bool                    defaultIsNewDoc;    //new doc field is selected by default

    bool                    hideLocation;       // hides location field and does not check it in validate()
    bool                    hideAnnotationName; // hides annotation name field
    bool                    useUnloadedObjects;
	QString					groupName;          // default groupname. If empty -> <auto> value is used (annotationObject->name value).
	SharedAnnotationData	data;               // holds name and location of the annotation

    GObjectReference        annotationObjectRef; // the object to be loaded
    QString                 newDocUrl;        // the URL of new document with annotation table to be created
    int                     sequenceLen;        //length of target sequence for validation purposes

    AnnotationTableObject*  getAnnotationObject() const;
};

class U2GUI_EXPORT CreateAnnotationWidgetController : public QObject {
Q_OBJECT
public:
	
	CreateAnnotationWidgetController(const CreateAnnotationModel& m, QObject* p);
    ~CreateAnnotationWidgetController();
	
	// returns error message or empty string if no error found;
    // does not create any new objects
	QString validate(); 

    // Ensures that annotationObeject is valid
    // for a validated model creates new document (newDocUrl), adds annotation table object 
    // to the document created and stores the reference to the object to annotationObject
    // Does nothing if annotationObject is not NULL
    void prepareAnnotationObject();

    // add model property instead ??
    bool isNewObject() const;

    void setFocusToNameEdit();

	QWidget* getWidget() const {return w;}

    const CreateAnnotationModel&    getModel() const {return model;}
    
    //receiver object must have sl_setPredefinedAnnotationName(), TODO: move this utility to a separate class
    static QMenu* createAnnotationNamesMenu(QWidget* p, QObject* receiver);

private slots:
	void sl_onNewDocClicked();
    void sl_onLoadObjectsClicked();
    void sl_setPredefinedAnnotationName();
    void sl_groupName();
    void sl_complementLocation();
    void sl_setPredefinedGroupName();

private:
	void updateModel();

	CreateAnnotationModel       model;
	GObjectComboBoxController*  occ;
	QWidget*                    w;
    Ui_CreateAnnotationWidget*  ui;
    
    QString GROUP_NAME_AUTO;
};


} // namespace

#endif
