#ifndef _U2_ADD_NEW_DOCUMENT_DIALOG_CTRL_H_
#define _U2_ADD_NEW_DOCUMENT_DIALOG_CTRL_H_

#include <U2Core/global.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class U2GUI_EXPORT AddNewDocumentDialogModel {
public:
    AddNewDocumentDialogModel(const QString& _url = QString::null, DocumentFormatId _format = DocumentFormatId::null)
        : url(_url), io(IOAdapterId::null), format(_format), successful(false){}

    QString				url;
    IOAdapterId         io;
    DocumentFormatId	format;
    bool				successful;

};

class U2GUI_EXPORT AddNewDocumentDialogController {
public:
	static void run(QWidget* p, AddNewDocumentDialogModel& m, const DocumentFormatConstraints& c);
};


}//namespace

#endif
