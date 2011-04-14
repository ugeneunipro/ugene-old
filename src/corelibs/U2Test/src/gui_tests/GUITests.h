#ifndef _U2_GUI_TESTS_H_
#define _U2_GUI_TESTS_H_

#include <U2Core/global.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>
#include <QtGui>
//#include <QtTest>

namespace U2 {

class U2TEST_EXPORT TestStatus {
public:
    TestStatus(): error(false), errorString("") {}
    bool hasError() const {
        return error;
    }
    QString getError() const {
        return errorString;
    }

    void setError(const QString &_error) {
        error = true;
        errorString = _error;
    }

private:
    bool error;
    QString errorString;
};

struct TestException {
    TestException(const QString &_msg = ""): msg(_msg) {}
    QString msg;
};


class U2TEST_EXPORT GUITest: public QObject{
    Q_OBJECT
public:
    GUITest(const QString &_name): QObject(NULL), name(_name) { //parent must bu NULL to push object to main thread
            connect(this, SIGNAL(runTask(Task*)),this, SLOT(sl_runTask(Task*)), Qt::BlockingQueuedConnection);
    } 

    void launch();
    virtual void execute() = 0;
    virtual void checkResult() = 0;

    bool isSuccessful() const {return !ts.hasError();}
    QString getError() const {return ts.getError();}
    void setError(const QString &error) {ts.setError(error);}

    QString getName() const {return name;}

protected:
    //mouse primitives
    void moveTo(const QString &widgetName, const QPoint &pos = QPoint());
    void mouseClick(const QString &widgetName, Qt::MouseButton button, const QPoint &pos = QPoint());
    void mouseDbClick(const QString &widgetName, const QPoint &pos = QPoint());
    void mousePress(const QString &widgetName, Qt::MouseButton button, const QPoint &pos = QPoint());
    void mouseRelease(const QString &widgetName, Qt::MouseButton button, const QPoint &pos = QPoint());

    //keyboard
    Qt::Key asciiToKey(char ascii);
    void keyPress(const QString &widgetName, int key, Qt::KeyboardModifiers modifiers = 0, const QString &text = "");
    void keyRelease(const QString &widgetName, int key, Qt::KeyboardModifiers modifiers = 0);
    void keyClick(const QString &widgetName, int key, Qt::KeyboardModifiers modifiers = 0, const QString &text = "");
    void keySequence(const QString &widgetName, const QString &sequence, Qt::KeyboardModifiers modifiers = 0);

    //menu
    void expandTopLevelMenu(const QString &menuName, const QString &parentMenu);
    void clickMenu(const QString &menuName, const QString &parentMenu, bool context = false);
    void contextMenu(const QString &widgetName, const QPoint &pos = QPoint());

    //tree model
    void mouseClickOnItem(const QString &widgetName, Qt::MouseButton button, const QPoint &pos = QPoint());
    void mouseDbClickOnItem(const QString &widgetName, const QPoint &pos = QPoint());
    void mousePressOnItem(const QString &widgetName, Qt::MouseButton button, const QPoint &pos = QPoint());
    void mouseReleaseOnItem(const QString &widgetName, Qt::MouseButton button, const QPoint &pos = QPoint());
    void contextMenuOnItem(const QString &widgetName, const QPoint &pos = QPoint());
    QPoint getItemPosition(const QString &itemName, const QString &treeName);
    bool isItemExists(const QString &itemName, const QString &treeName);
    void expandTreeItem(const QString &itemName, const QString &treeName);

    QWidget *findWidgetByName(const QString &widgetName, const QString &parentName = "") const;
    QWidget *findWidgetByTitle(const QString &widgetTitle) const;
    bool isWidgetExists(const QString &widgetName) const;
    QMenu   *getContextMenu() const {return static_cast<QMenu*>(QApplication::activePopupWidget());}
    QDialog *getActiveDialog() const {return static_cast<QDialog*>(QApplication::activeModalWidget());}
    QWidget *getWidgetInFocus() const {return QApplication::focusWidget();}

    void sendEvent(QObject *obj, QEvent *e);
    void sleep(int msec);

protected:
    TestStatus ts;

signals:
    void runTask(Task *t);

private slots:
    void sl_runTask(Task *t);

private:
    QString name;
    class Waiter: public QThread {
    public:
        static void await(int mseconds) {
            msleep(mseconds);
        }
    };
};


} //U2

#endif
