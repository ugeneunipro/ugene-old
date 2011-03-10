#ifndef _U2_MWMENU_MANAGER_IMPL_H_
#define _U2_MWMENU_MANAGER_IMPL_H_

#include <U2Gui/MainWindow.h>

#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtCore/QEvent>

namespace U2 {

class MWMenuManagerImpl: public QObject {
    Q_OBJECT
public:
	MWMenuManagerImpl(QObject* p, QMenuBar* mb);

	QMenu* getTopLevelMenu(const QString& sysName) const ;

protected:
	bool eventFilter(QObject *obj, QEvent *event);
private:
	void unlinkTopLevelMenu(QMenu*);
	void linkTopLevelMenu(QMenu*);
	QMenu* createTopLevelMenu(const QString& sysName, const QString& title, const QString& afterSysName = QString::null);
	void updateTopLevelMenuVisibility(QMenu* m);
	QMenuBar* menuBar;
	QList<QMenu*> toplevelMenus;
};

}//namespace

#endif
