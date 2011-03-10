#ifndef _U2_CREATE_ANNOTATION_DIALOG_H_
#define _U2_CREATE_ANNOTATION_DIALOG_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>

namespace U2 {

class CreateAnnotationWidgetController;
class CreateAnnotationModel;

class U2GUI_EXPORT CreateAnnotationDialog : public QDialog {
Q_OBJECT
public:
    //saves the results to the provided model
	CreateAnnotationDialog(QWidget* p, CreateAnnotationModel& m);
    
private slots:
	void sl_onCreateClicked(bool);
	void sl_onCancelClicked(bool);

private:
    CreateAnnotationModel& model;
    CreateAnnotationWidgetController* annWidgetController;
    QPushButton* createButton;
	QPushButton* cancelButton;
};

} // namespace

#endif
