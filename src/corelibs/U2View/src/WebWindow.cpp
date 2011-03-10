#include "WebWindow.h"

#include <QtGui/QVBoxLayout>
#include <QtWebKit/QWebView>

namespace U2 {

WebWindow::WebWindow(const QString& title, const QString& content)
: MWMDIWindow(title)
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    setLayout(l);

    QWebView* textEdit = new QWebView(this);
    textEdit->setHtml(content);

    l->addWidget(textEdit);
}

} //namespace
