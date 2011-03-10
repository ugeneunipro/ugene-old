#ifndef _U2_DOCUMENT_FORMAT_COMBOBOX_CONTROLLER_H_
#define _U2_DOCUMENT_FORMAT_COMBOBOX_CONTROLLER_H_

#include <U2Core/DocumentModel.h>

#include <QtGui/QComboBox>

namespace U2 {
    
class U2GUI_EXPORT DocumentFormatComboboxController : public QObject {
	Q_OBJECT
public:
	DocumentFormatComboboxController(QObject* p, QComboBox* combo, const DocumentFormatConstraints& c, 
		DocumentFormatId active = DocumentFormatId::null);

	void updateConstraints(const DocumentFormatConstraints& c);

	DocumentFormatId getActiveFormatId() const;
	void setActiveFormatId(DocumentFormatId);
	bool hasSelectedFormat() const {return !getActiveFormatId().isNull();}
	QList<DocumentFormatId> getFormatsInCombo();
    QComboBox* comboBox()  { return combo; }

    static void fill(QComboBox* combo, QList<DocumentFormatId>& ids, DocumentFormatId active);
    static DocumentFormatId getActiveFormatId(QComboBox* combo);
    static QList<DocumentFormatId> getFormatsInCombo(QComboBox* combo);

private slots:
	void sl_onDocumentFormatRegistered(DocumentFormat*);
	void sl_onDocumentFormatUnregistered(DocumentFormat*);

private:
	void updateCombo(DocumentFormatId active);

	QComboBox* combo;
	DocumentFormatConstraints c;
};

}

#endif
