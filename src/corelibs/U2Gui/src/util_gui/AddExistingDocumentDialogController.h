#ifndef _U2_ADD_EXISTING_DOCUMENT_DIALOG_CTRL_H_
#define _U2_ADD_EXISTING_DOCUMENT_DIALOG_CTRL_H_

#include <U2Core/global.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class U2GUI_EXPORT AddExistingDocumentDialogModel {
public:
	AddExistingDocumentDialogModel(const QString& _url = QString::null,  
		DocumentFormatId _format = DocumentFormatId::null, bool _readOnly = false)
		: url(_url), io(IOAdapterId::null), format(_format), readOnly(_readOnly), successful(false){}
	
	QString				url;
	IOAdapterId         io;
	DocumentFormatId	format;	
	bool				readOnly;
    QVariantMap         formatSettings;
	bool				successful;
    QString             dwnldPath;    
};

class U2GUI_EXPORT AddExistingDocumentDialogController {
public:
	static void run(QWidget* p, AddExistingDocumentDialogModel& m_, bool runFileDialog = false);
};

} //namespace

#endif

