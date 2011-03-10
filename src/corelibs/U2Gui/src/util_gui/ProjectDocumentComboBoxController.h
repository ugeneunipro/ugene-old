#ifndef _U2_PROJECT_DOCUMENT_COMBOBOX_CONTROLLER
#define _U2_PROJECT_DOCUMENT_COMBOBOX_CONTROLLER

#include <U2Core/DocumentModel.h>

#include <QtCore/QObject>
#include <QtGui/QComboBox>

namespace U2 {

class Project;

class U2GUI_EXPORT ProjectDocumentComboBoxController : public QObject {
	Q_OBJECT
public:

	ProjectDocumentComboBoxController(Project* p, QComboBox* cb, QObject* parent, const DocumentFilter* f);
    ~ProjectDocumentComboBoxController();

	Document* getDocument() const;

	void selectDocument(Document* d);
    
    void selectDocument(const QString& url);

private slots:
	void sl_onDocumentAdded(Document* d);
	void sl_onDocumentRemoved(Document* d);

private:
	bool checkConstraints(Document* d);
	void addDocument(Document* d);
	void removeDocument(Document* d);

	Project* p;
	QComboBox* cb;
	const DocumentFilter* filter;
};

} // namespace

#endif
