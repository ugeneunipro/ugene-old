#ifndef _U2_SCRIPT_EDITOR_DIALOG_H_
#define _U2_SCRIPT_EDITOR_DIALOG_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>
#include <QtGui/QSyntaxHighlighter>

class Ui_ScriptEditorDialog;

namespace U2 {
class U2MISC_EXPORT ScriptEditorDialog : public QDialog {
    Q_OBJECT
public:
    ScriptEditorDialog(QWidget* p, const QString& roHeaderText, const QString & text = QString());
    ~ScriptEditorDialog();

    void setScriptText(const QString& text);

    void setScriptPath(const QString& path);

    QString getScriptText() const;

    static QString getScriptsFileFilter();

private slots:
    void sl_checkSyntax();
    void sl_openScript();
    void sl_saveScript();
    void sl_saveAsScript();
    void sl_nameChanged(const QString&);
    void sl_scriptChanged();

private:
    void updateState();
    void save(const QString& url);
    Ui_ScriptEditorDialog* ui;
};


class ScriptHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    ScriptHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};


} //namespace

#endif

