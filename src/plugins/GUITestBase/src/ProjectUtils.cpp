#include "ProjectUtils.h"
#include "QtUtils.h"
#include <QDropEvent>
#include <QUrl>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Test/GUITestBase.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

void ProjectUtils::openFile(U2OpStatus &os, const GUrl &path, const OpenFileSettings& settings) {

	switch (settings.openMethod) {
		case OpenFileSettings::DRAGDROP:
		default:
			openFileDrop(os, path);
	}

	QtUtils::sleep(3000);

	checkProjectExists(os);
	Document* doc = checkDocumentExists(os, path);
	checkDocumentActive(os, doc);
}

void ProjectUtils::checkProjectExists(U2OpStatus &os) {

	// check if project exists
	CHECK_SET_ERR(AppContext::getProject() != NULL, "There is no project");
}

Document* ProjectUtils::checkDocumentExists(U2OpStatus &os, const GUrl &url) {

	Project *project = AppContext::getProject();
	if (!project) {
		return NULL;
	}

	// check if document with url exists and was shown
	Document *doc = project->findDocumentByURL(url);

	CHECK_SET_ERR_RESULT(doc != NULL, "There is no document", NULL);
	CHECK_SET_ERR_RESULT(doc->isLoaded(), "Document is not loaded", NULL);

	return doc;
}

void ProjectUtils::checkDocumentActive(U2OpStatus &os, Document *doc) {

	if (!doc) {
		return;
	}

	GObjectViewWindow* viewWindow = GObjectViewUtils::getActiveObjectViewWindow();

	MWMDIWindow *activeWindow = AppContext::getMainWindow()->getMDIManager()->getActiveWindow();
	CHECK_SET_ERR(activeWindow != NULL, "There is no active window");

	const QList<GObject*>& docObjects = doc->getObjects();
	CHECK_SET_ERR(!docObjects.isEmpty(), "Document has no objects");

	QList<GObjectViewWindow*> viewsList = GObjectViewUtils::findViewsWithAnyOfObjects(docObjects);
	CHECK_SET_ERR(!viewsList.isEmpty(), "View is not loaded");

	MWMDIWindow *documentWindow = viewsList.first();
	CHECK_SET_ERR(documentWindow == activeWindow, "documentWindow is not active");
}

void ProjectUtils::openFileDrop(U2OpStatus &os, const GUrl &path) {

	QWidget* widget = AppContext::getMainWindow()->getQMainWindow();
	QPoint widgetPos(widget->width()/2, widget->height()/2);

	QList<QUrl> urls;
	urls << path.getURLString();

	QMimeData *mimeData = new QMimeData();
	mimeData->setUrls(urls);

	Qt::DropActions dropActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
	Qt::MouseButtons mouseButtons = Qt::LeftButton;

	QDragEnterEvent* dragEnterEvent = new QDragEnterEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
	QtUtils::sendEvent(widget, dragEnterEvent);

	QDropEvent* dropEvent = new QDropEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
	QtUtils::sendEvent(widget, dropEvent);
}

} // U2
