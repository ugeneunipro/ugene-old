#ifndef _U2_LOGVIEW_IMPL_
#define _U2_LOGVIEW_IMPL_

#include <LogSettings.h>

#include <QtCore/QHash>
#include <QtCore/QRegExp>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QShortcut>
#include <QtGui/QSyntaxHighlighter>

namespace U2 {

class LogCache;
class LogMessage;
class LogFilter;

class SearchHighlighter: public QSyntaxHighlighter {
public:
    SearchHighlighter(QTextDocument *doc): QSyntaxHighlighter(doc) {}

    QRegExp reg_exp;
    void highlightBlock(const QString &text);
};


enum LogViewSearchBoxMode {
    LogViewSearchBox_Visible,
    LogViewSearchBox_Auto, //TODO: not supported today
    LogViewSearchBox_Hidden
};

class U2GUI_EXPORT LogViewWidget : public QWidget, public LogSettingsHolder {
    Q_OBJECT
public:
    /** If categoriesFilter is not-empty LogViewWidget shows log messages 
        only from categories listed in categoriesFilter
        LogSettings are ignored in this case
     */
    LogViewWidget(LogCache* c);
    LogViewWidget(const LogFilter& filter);
    
    void resetView();
    
    bool isShown(const LogMessage& msg);
    bool isShown(const QString& txt);
    /** returns first category in the msg.categories that match 'show-filter' criteria*/
    QString getEffectiveCategory(const LogMessage& msg) const;

    virtual void setSettings(const LogSettings& s); 

    void setSearchBoxMode(LogViewSearchBoxMode mode);

protected:
    void addMessage(const LogMessage& msg);
    void addText(const QString& text);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);

private slots:

    void sl_onMessage(const LogMessage& msg);
    void sl_onTextEdited(const QString& text);
    void popupMenu(const QPoint &pos);
    void sl_openSettingsDialog();
    void sl_logSettingsChanged();
    void sl_dumpCounters();
    void sl_clear();
    void sl_addSeparator();
    void sl_showHideEdit();
    void searchPopupMenu(const QPoint &pos);
    void setSearchCaseSensitive();
    void useRegExp();

private:
    struct EntryStruct {
        bool is_plain_text;
        LogMessage msg;
        EntryStruct(const QString &txt): is_plain_text(true) { msg.text = txt; }
        EntryStruct(const LogMessage &_msg): is_plain_text(false), msg(_msg) {}
    };

    QString prepareText(const LogMessage& msg) const;
    void init();
    void resetText();


    QPlainTextEdit*          edit;
    QLineEdit*               searchEdit;
    QList<EntryStruct>       original_text;
    QShortcut*               shortcut;
    SearchHighlighter*       highlighter;
    bool                     caseSensitive, useRegexp;
    
	int                 messageCounter;
    LogCache*           cache;
    QAction*            showViewAction;
    QAction*            showSettingsAction;
    QAction*            dumpCountersAction;
    QAction*            clearAction;
    QAction*            addSeparatorAction;

    bool connected; //for debug only

};

} //namespace

#endif
