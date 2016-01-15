/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "MultilingualHtmlView.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include <QDesktopServices>
#include <QFile>
#include <QWebElement>
#include <QWebElementCollection>
#include <QWebFrame>


namespace U2 {

MultilingualHtmlView::MultilingualHtmlView(const QString& htmlPath, QWidget* parent)
    : QWebView(parent) {
    setContextMenuPolicy(Qt::NoContextMenu);
    loadPage(htmlPath);
    page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
}

void MultilingualHtmlView::sl_loaded(bool ok) {
    disconnect(this, SIGNAL(loadFinished(bool)), this, SLOT(sl_loaded(bool)));
    SAFE_POINT(ok, "Can not load page", );

    Settings* s = AppContext::getSettings();
    SAFE_POINT(s != NULL, "AppContext settings is NULL", );
    QString lang = s->getValue("UGENE_CURR_TRANSL", "en").toString();

    QWebFrame* frame = page()->mainFrame();
    SAFE_POINT(frame != NULL, "MainFrame of webView page is NULL", );

    QWebElementCollection otherLangsCollection = frame->findAllElements(QString(":not(:lang(%1))[lang]").arg(lang));
    for (int i = 0; i < otherLangsCollection.count(); i++) {
        otherLangsCollection[i].setStyleProperty("display", "none");
    }
    emit si_loaded(ok);
}

void MultilingualHtmlView::sl_linkActivated(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

void MultilingualHtmlView::loadPage(const QString& htmlPath) {
    QFile file(htmlPath);
    bool opened = file.open(QIODevice::ReadOnly);
    SAFE_POINT(opened, "Can not load file", );

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString html = stream.readAll();
    file.close();

    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(sl_loaded(bool)));
    connect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(sl_linkActivated(QUrl)));
    page()->mainFrame()->setHtml(html);
}

} // namespace
