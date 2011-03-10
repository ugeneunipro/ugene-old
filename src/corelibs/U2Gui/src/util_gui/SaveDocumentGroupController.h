#ifndef _U2_SAVE_DOCUMENT_GROUP_CONTROLLER_H_
#define _U2_SAVE_DOCUMENT_GROUP_CONTROLLER_H_

#include <U2Core/DocumentModel.h>

#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QToolButton>

namespace U2{


class DocumentFormatComboboxController;

class SaveDocumentGroupControllerConfig {
public:
	DocumentFormatConstraints	dfc;                // format constraints applied for formats combo
	QToolButton*				fileDialogButton;   // a button to open save file dialog
	QString						saveTitle;          // a title for save file dialog
	QString                     defaultFileName;    // filename set by default
    QComboBox*					formatCombo;        // combo widget to list formats
	QLineEdit*					fileNameEdit;       // edit for file name
	DocumentFormatId			defaultFormatId;    // format selected by default
	QWidget*					parentWidget;       // parent widget for file dialog
};

/* Controls 'save document' fields :
	- combo with document format selection
    - edit field with document name
*/
class U2GUI_EXPORT SaveDocumentGroupController : public QObject {
    Q_OBJECT
public:
    SaveDocumentGroupController(const SaveDocumentGroupControllerConfig& conf, QObject* parent);
    
	QString getSaveFileName() const {return conf.fileNameEdit->text();}
	
	DocumentFormatId getFormatIdToSave() const;
	
	DocumentFormat* getFormatToSave() const;
    
    QString getDefaultFileName() const {return conf.defaultFileName;}
    
private slots:
    void sl_fileNameChanged(const QString& v);
	void sl_saveButtonClicked();
    void sl_formatChanged(const QString& newFormat);

private:
	DocumentFormatComboboxController* comboController;
	SaveDocumentGroupControllerConfig conf;
};

}//namespace

#endif

