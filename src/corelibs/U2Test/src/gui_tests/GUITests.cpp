#include "GUITests.h"

#include <U2Core/Task.h>

namespace U2 {

void GUITest::launch(){
    moveToThread(QApplication::instance()->thread());
    execute();
    checkResult();
}

QWidget* GUITest::findWidgetByName(const QString &widgetName, const QString &parentName) const {
    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    QWidget *w = NULL;
    if(parentName.isEmpty()) {
        w= mw->findChild<QWidget*>(widgetName);
        
    } else {
        QWidget *parent = mw->findChild<QWidget*>(parentName);
        if(parent) {
            w = parent->findChild<QWidget*>(widgetName);
        }
    }
    if(!w) {
        TestException t(tr("Widget %1 not found").arg(widgetName));
        throw t;
    }
    return w;
}

bool GUITest::isWidgetExists(const QString &widgetName) const {
    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    QWidget *w = mw->findChild<QWidget*>(widgetName);
    return (w != NULL);
}

QWidget* GUITest::findWidgetByTitle(const QString &title) const {
    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    QList<QWidget *>wList = mw->findChildren<QWidget*>();
    foreach(QWidget *w, wList) {
        QString t = w->windowTitle();
        if(w->windowTitle() == title) {
            return w;
        }
    }
    throw TestException(tr("Widget %1 not found").arg(title));
}


void GUITest::moveTo(const QString &widgetName, const QPoint &_pos) {
    QWidget * w = findWidgetByName(widgetName);
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QPoint begin = QCursor::pos();
    QPoint end = w->mapToGlobal(pos);

    float k = (float)(end.ry() - begin.ry())/(end.rx() - begin.rx());
    float b = begin.ry() - k * begin.rx();


    int x1 = begin.rx();
    int x2 = end.rx();
    
    if(x1 < x2) {
        for(int i = x1; i <= x2; i++) {
            int y = k*i + b;
            sleep(1);
            QCursor::setPos(i, y);
        }
    } else {
        for(int i = x1; i >= x2; i--) {
            int y = k*i + b;
            sleep(1);
            QCursor::setPos(i, y);
        }
    }
}

void GUITest::mousePress(const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    QWidget * w = findWidgetByName(widgetName);
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonPress, pos, button, button, 0); 
    sendEvent(w, me);
}

void GUITest::mouseRelease(const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    QWidget * w = findWidgetByName(widgetName);
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonRelease, pos, button, button, 0); 
    sendEvent(w, me);
}

void GUITest::mouseClick(const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    mousePress(widgetName, button, _pos);
    mouseRelease(widgetName, button, _pos);
}

void GUITest::mouseDbClick(const QString &widgetName, const QPoint &_pos) {
    QWidget * w = findWidgetByName(widgetName);
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(w, me);
}

void GUITest::keyPress(const QString &widgetName, int key, Qt::KeyboardModifiers modifiers, const QString &text) {
    QWidget *w = findWidgetByName(widgetName);
    QKeyEvent *ke = new QKeyEvent(QEvent::KeyPress, key, modifiers, text);
    QCoreApplication::postEvent(w, ke);
    QCoreApplication::processEvents();
}

void GUITest::keyRelease(const QString &widgetName, int key, Qt::KeyboardModifiers modifiers) {
    QWidget *w = findWidgetByName(widgetName);
    QKeyEvent *ke = new QKeyEvent(QEvent::KeyRelease, key, modifiers);
    QCoreApplication::postEvent(w, ke);
    QCoreApplication::processEvents();
}

void GUITest::keyClick(const QString &widgetName, int key, Qt::KeyboardModifiers modifiers, const QString &text) {
    keyPress(widgetName, key, modifiers, text);
    keyRelease(widgetName, key, modifiers);
}

Qt::Key GUITest::asciiToKey(char ascii) { // = asciiToKey(char ascii) from QTest
    switch ((unsigned char)ascii) {
    case 0x08: return Qt::Key_Backspace;
    case 0x09: return Qt::Key_Tab;
    case 0x0b: return Qt::Key_Backtab;
    case 0x0d: return Qt::Key_Return;
    case 0x1b: return Qt::Key_Escape;
    case 0x20: return Qt::Key_Space;
    case 0x21: return Qt::Key_Exclam;
    case 0x22: return Qt::Key_QuoteDbl;
    case 0x23: return Qt::Key_NumberSign;
    case 0x24: return Qt::Key_Dollar;
    case 0x25: return Qt::Key_Percent;
    case 0x26: return Qt::Key_Ampersand;
    case 0x27: return Qt::Key_Apostrophe;
    case 0x28: return Qt::Key_ParenLeft;
    case 0x29: return Qt::Key_ParenRight;
    case 0x2a: return Qt::Key_Asterisk;
    case 0x2b: return Qt::Key_Plus;
    case 0x2c: return Qt::Key_Comma;
    case 0x2d: return Qt::Key_Minus;
    case 0x2e: return Qt::Key_Period;
    case 0x2f: return Qt::Key_Slash;
    case 0x30: return Qt::Key_0;
    case 0x31: return Qt::Key_1;
    case 0x32: return Qt::Key_2;
    case 0x33: return Qt::Key_3;
    case 0x34: return Qt::Key_4;
    case 0x35: return Qt::Key_5;
    case 0x36: return Qt::Key_6;
    case 0x37: return Qt::Key_7;
    case 0x38: return Qt::Key_8;
    case 0x39: return Qt::Key_9;
    case 0x3a: return Qt::Key_Colon;
    case 0x3b: return Qt::Key_Semicolon;
    case 0x3c: return Qt::Key_Less;
    case 0x3d: return Qt::Key_Equal;
    case 0x3e: return Qt::Key_Greater;
    case 0x3f: return Qt::Key_Question;
    case 0x40: return Qt::Key_At;
    case 0x41: return Qt::Key_A;
    case 0x42: return Qt::Key_B;
    case 0x43: return Qt::Key_C;
    case 0x44: return Qt::Key_D;
    case 0x45: return Qt::Key_E;
    case 0x46: return Qt::Key_F;
    case 0x47: return Qt::Key_G;
    case 0x48: return Qt::Key_H;
    case 0x49: return Qt::Key_I;
    case 0x4a: return Qt::Key_J;
    case 0x4b: return Qt::Key_K;
    case 0x4c: return Qt::Key_L;
    case 0x4d: return Qt::Key_M;
    case 0x4e: return Qt::Key_N;
    case 0x4f: return Qt::Key_O;
    case 0x50: return Qt::Key_P;
    case 0x51: return Qt::Key_Q;
    case 0x52: return Qt::Key_R;
    case 0x53: return Qt::Key_S;
    case 0x54: return Qt::Key_T;
    case 0x55: return Qt::Key_U;
    case 0x56: return Qt::Key_V;
    case 0x57: return Qt::Key_W;
    case 0x58: return Qt::Key_X;
    case 0x59: return Qt::Key_Y;
    case 0x5a: return Qt::Key_Z;
    case 0x5b: return Qt::Key_BracketLeft;
    case 0x5c: return Qt::Key_Backslash;
    case 0x5d: return Qt::Key_BracketRight;
    case 0x5e: return Qt::Key_AsciiCircum;
    case 0x5f: return Qt::Key_Underscore;
    case 0x60: return Qt::Key_QuoteLeft;
    case 0x61: return Qt::Key_A;
    case 0x62: return Qt::Key_B;
    case 0x63: return Qt::Key_C;
    case 0x64: return Qt::Key_D;
    case 0x65: return Qt::Key_E;
    case 0x66: return Qt::Key_F;
    case 0x67: return Qt::Key_G;
    case 0x68: return Qt::Key_H;
    case 0x69: return Qt::Key_I;
    case 0x6a: return Qt::Key_J;
    case 0x6b: return Qt::Key_K;
    case 0x6c: return Qt::Key_L;
    case 0x6d: return Qt::Key_M;
    case 0x6e: return Qt::Key_N;
    case 0x6f: return Qt::Key_O;
    case 0x70: return Qt::Key_P;
    case 0x71: return Qt::Key_Q;
    case 0x72: return Qt::Key_R;
    case 0x73: return Qt::Key_S;
    case 0x74: return Qt::Key_T;
    case 0x75: return Qt::Key_U;
    case 0x76: return Qt::Key_V;
    case 0x77: return Qt::Key_W;
    case 0x78: return Qt::Key_X;
    case 0x79: return Qt::Key_Y;
    case 0x7a: return Qt::Key_Z;
    case 0x7b: return Qt::Key_BraceLeft;
    case 0x7c: return Qt::Key_Bar;
    case 0x7d: return Qt::Key_BraceRight;
    case 0x7e: return Qt::Key_AsciiTilde;

        // Latin 1 codes adapted from X: keysymdef.h,v 1.21 94/08/28 16:17:06
    case 0xa0: return Qt::Key_nobreakspace;
    case 0xa1: return Qt::Key_exclamdown;
    case 0xa2: return Qt::Key_cent;
    case 0xa3: return Qt::Key_sterling;
    case 0xa4: return Qt::Key_currency;
    case 0xa5: return Qt::Key_yen;
    case 0xa6: return Qt::Key_brokenbar;
    case 0xa7: return Qt::Key_section;
    case 0xa8: return Qt::Key_diaeresis;
    case 0xa9: return Qt::Key_copyright;
    case 0xaa: return Qt::Key_ordfeminine;
    case 0xab: return Qt::Key_guillemotleft;
    case 0xac: return Qt::Key_notsign;
    case 0xad: return Qt::Key_hyphen;
    case 0xae: return Qt::Key_registered;
    case 0xaf: return Qt::Key_macron;
    case 0xb0: return Qt::Key_degree;
    case 0xb1: return Qt::Key_plusminus;
    case 0xb2: return Qt::Key_twosuperior;
    case 0xb3: return Qt::Key_threesuperior;
    case 0xb4: return Qt::Key_acute;
    case 0xb5: return Qt::Key_mu;
    case 0xb6: return Qt::Key_paragraph;
    case 0xb7: return Qt::Key_periodcentered;
    case 0xb8: return Qt::Key_cedilla;
    case 0xb9: return Qt::Key_onesuperior;
    case 0xba: return Qt::Key_masculine;
    case 0xbb: return Qt::Key_guillemotright;
    case 0xbc: return Qt::Key_onequarter;
    case 0xbd: return Qt::Key_onehalf;
    case 0xbe: return Qt::Key_threequarters;
    case 0xbf: return Qt::Key_questiondown;
    case 0xc0: return Qt::Key_Agrave;
    case 0xc1: return Qt::Key_Aacute;
    case 0xc2: return Qt::Key_Acircumflex;
    case 0xc3: return Qt::Key_Atilde;
    case 0xc4: return Qt::Key_Adiaeresis;
    case 0xc5: return Qt::Key_Aring;
    case 0xc6: return Qt::Key_AE;
    case 0xc7: return Qt::Key_Ccedilla;
    case 0xc8: return Qt::Key_Egrave;
    case 0xc9: return Qt::Key_Eacute;
    case 0xca: return Qt::Key_Ecircumflex;
    case 0xcb: return Qt::Key_Ediaeresis;
    case 0xcc: return Qt::Key_Igrave;
    case 0xcd: return Qt::Key_Iacute;
    case 0xce: return Qt::Key_Icircumflex;
    case 0xcf: return Qt::Key_Idiaeresis;
    case 0xd0: return Qt::Key_ETH;
    case 0xd1: return Qt::Key_Ntilde;
    case 0xd2: return Qt::Key_Ograve;
    case 0xd3: return Qt::Key_Oacute;
    case 0xd4: return Qt::Key_Ocircumflex;
    case 0xd5: return Qt::Key_Otilde;
    case 0xd6: return Qt::Key_Odiaeresis;
    case 0xd7: return Qt::Key_multiply;
    case 0xd8: return Qt::Key_Ooblique;
    case 0xd9: return Qt::Key_Ugrave;
    case 0xda: return Qt::Key_Uacute;
    case 0xdb: return Qt::Key_Ucircumflex;
    case 0xdc: return Qt::Key_Udiaeresis;
    case 0xdd: return Qt::Key_Yacute;
    case 0xde: return Qt::Key_THORN;
    case 0xdf: return Qt::Key_ssharp;
    case 0xe5: return Qt::Key_Aring;
    case 0xe6: return Qt::Key_AE;
    case 0xf7: return Qt::Key_division;
    case 0xf8: return Qt::Key_Ooblique;
    case 0xff: return Qt::Key_ydiaeresis;
    default: assert(false); return Qt::Key(0);
    }
}

void GUITest::keySequence(const QString &widgetName, const QString &sequence, Qt::KeyboardModifiers modifiers) {
    for(int i = 0; i< sequence.length();i++) {
        keyClick(widgetName, asciiToKey(sequence.at(i).toLatin1()), modifiers, QString(sequence.at(i).toLatin1()));
    }
}

void GUITest::expandTopLevelMenu(const QString &menuName, const QString &parentMenu) {
    QMenuBar *parMenu = static_cast<QMenuBar*>(findWidgetByName(parentMenu));
    if(!parMenu->isVisible()) {
        return;
    }
    QList<QAction*> actions = parMenu->actions();
    QAction *curAction = NULL;
    foreach(QAction *a, actions) {
        if(a->text() == menuName) {
            curAction = a;
            break;
        }
    }
    if(!curAction) {
        throw TestException(tr("Can't find action %1").arg(menuName));
    }

    QPoint pos = parMenu->actionGeometry(curAction).center();
    moveTo(parentMenu, pos);
    mouseClick(parentMenu, Qt::LeftButton, pos);
}

void GUITest::clickMenu(const QString &menuName, const QString &parentMenu, bool context) {
    QMenu* parMenu;
    if(!context) {
        parMenu = (QMenu*)findWidgetByName(parentMenu);
    } else {
        parMenu = getContextMenu();
    }
    QList<QAction*> actions = parMenu->actions();
    QAction *curAction = NULL;
    foreach(QAction *a, actions) {
        if(a->text() == menuName) {
            curAction = a;
            break;
        }
    }
    if(!curAction) {
        throw TestException(tr("Can't find action %1").arg(menuName));
    }

    QPoint pos = parMenu->actionGeometry(curAction).center();
    QCursor::setPos(parMenu->mapToGlobal(pos));
    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(parMenu, me);
    QMouseEvent *me1 = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(parMenu, me1);
}

void GUITest::contextMenu(const QString &widgetName, const QPoint &_pos) {
    QWidget * w = findWidgetByName(widgetName);
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QContextMenuEvent *cme = new QContextMenuEvent(QContextMenuEvent::Mouse, pos);
    mousePress(widgetName, Qt::RightButton, pos); //Select item in tree
    sendEvent(w, cme);
}

void GUITest::sendEvent(QObject *obj, QEvent *e) {
    QCoreApplication::postEvent(obj, e, Qt::HighEventPriority);
    QCoreApplication::processEvents();
}

void GUITest::sleep( int msec ){
    Waiter::await(msec);
}


void GUITest::mousePressOnItem(const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    QTreeView * w = static_cast<QTreeView *>(findWidgetByName(widgetName));
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonPress, pos, button, button, 0); 
    sendEvent(w->viewport(), me);
}

void GUITest::mouseReleaseOnItem(const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    QTreeView * w = static_cast<QTreeView *>(findWidgetByName(widgetName));
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonRelease, pos, button, button, 0); 
    sendEvent(w->viewport(), me);
}

void GUITest::mouseClickOnItem(const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    mousePressOnItem(widgetName, button, _pos);
    mouseReleaseOnItem(widgetName, button, _pos);
}

void GUITest::mouseDbClickOnItem(const QString &widgetName, const QPoint &_pos) {
    QTreeView * w = static_cast<QTreeView *>(findWidgetByName(widgetName));
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(w->viewport(), me);
}

void GUITest::contextMenuOnItem(const QString &widgetName, const QPoint &_pos) {
    QTreeView * w = static_cast<QTreeView *>(findWidgetByName(widgetName));
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QContextMenuEvent *cme = new QContextMenuEvent(QContextMenuEvent::Mouse, pos);
    mousePress(widgetName, Qt::RightButton, pos); //Select item in tree
    sendEvent(w->viewport(), cme);
}

QPoint GUITest::getItemPosition(const QString &itemName, const QString &treeName) {
    QTreeWidget *tree = static_cast<QTreeWidget*>(findWidgetByName(treeName));
    if(tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive).isEmpty()) {
        throw TestException(tr("Item %1 not found").arg(itemName));
    }
    QTreeWidgetItem *item = tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive).first();
    if(!item) {
        return QPoint();
    }

    return tree->visualItemRect(item).center();
}

bool GUITest::isItemExists(const QString &itemName, const QString &treeName) {
    QTreeWidget *tree = static_cast<QTreeWidget*>(findWidgetByName(treeName));
    if(tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive).isEmpty()) {
        return false;
    }
    return true;
}

void GUITest::sl_runTask(Task *t) {
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void GUITest::expandTreeItem( const QString &itemName, const QString &treeName ){
    QTreeWidget *tree = static_cast<QTreeWidget*>(findWidgetByName(treeName));
    if(tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive).isEmpty()) {
        throw TestException(tr("Item %1 not found").arg(itemName));
    }
    QTreeWidgetItem *item = tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive).first();
    QPoint pos = tree->visualItemRect(item).topLeft();
    moveTo(treeName, pos);
    mouseClickOnItem(treeName, Qt::LeftButton, pos);
    keyClick(tree->viewport()->objectName(), Qt::Key_Direction_R);    
}




}
