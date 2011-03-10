#ifndef _U2_MWTOOLBAR_IMPL_H_
#define _U2_MWTOOLBAR_IMPL_H_

#include <U2Gui/MainWindow.h>

#include <QtGui/QToolBar>
#include <QtGui/QMainWindow>

namespace U2 {

class MWToolBarManagerImpl : public QObject {
public:	
	MWToolBarManagerImpl(QMainWindow* mw);
	~MWToolBarManagerImpl();

	QToolBar* getToolbar(const QString& sysName) const;

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private:
	QToolBar* createToolBar(const QString& sysName);
	void updateToolbarVisibilty();

	QMainWindow*		mw;
	QList<QToolBar*>	toolbars;
};

}//namespace

#endif
