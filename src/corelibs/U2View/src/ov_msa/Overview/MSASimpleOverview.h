/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_SIMPLE_OVERVIEW_H_
#define _U2_MSA_SIMPLE_OVERVIEW_H_

#include <U2Core/global.h>

#include <QtGui/QWidget>
#include "MSAOverview.h"


namespace U2 {

class MSAEditor;
class MSAEditorUI;
class MSAEditorSequenceArea;
class MSAColorScheme;


class U2VIEW_EXPORT MSASimpleOverview : public MSAOverview {
    Q_OBJECT
public:
    MSASimpleOverview(MSAEditorUI *ui);
    const static int FIXED_HEIGTH = 70;
    bool isValid();

public slots:
    void sl_visibleRangeChanged();
    void sl_selectionChanged();
    void sl_redraw();
    void sl_highlightingChanged();

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    void drawOverview(QPainter &p);
    void drawVisibleRange(QPainter &p);
    void drawSelection(QPainter &p);

    void moveVisibleRange(QPoint pos);

private:
    MSAColorScheme*         colorScheme;

    QPixmap cachedMSAOverview;

    bool    redrawMSAOverview;
};

} // namespace

#endif // _U2_MSA_SIMPLE_OVERVIEW_H_
