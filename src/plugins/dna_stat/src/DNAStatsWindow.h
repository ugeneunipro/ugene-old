/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DNA_STATS_WINDOW_H_
#define _U2_DNA_STATS_WINDOW_H_

#include <QtWebKit/QWebView>

#include <U2Core/global.h>
#include <U2Gui/MainWindow.h>

namespace U2 {

class ADVSequenceObjectContext;
class DNAStatProfileTask;
class DNAStatsWindow;

class DNAStatsWebView : public QWebView {
    Q_OBJECT
public:
    DNAStatsWebView(DNAStatsWindow* parent) :  statsWindow(parent) {}
protected:
    virtual void contextMenuEvent(QContextMenuEvent*);
private:
    DNAStatsWindow* statsWindow;
};


class DNAStatsWindow: public MWMDIWindow {
    Q_OBJECT
public:
    DNAStatsWindow(ADVSequenceObjectContext* ctx);
private slots:
    void sl_onTaskStateChanged(Task* task);
private:
    void update();
    DNAStatsWebView* webView;
    ADVSequenceObjectContext* ctx;
    DNAStatProfileTask* updateTask;


};

}//namespace
#endif //_U2_DNA_STATS_WINDOW_H_
