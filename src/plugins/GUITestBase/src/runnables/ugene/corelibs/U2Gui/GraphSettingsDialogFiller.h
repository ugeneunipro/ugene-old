/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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


#ifndef _U2_GRAPH_SETTINGS_DIALOG_FILLER_H_
#define _U2_GRAPH_SETTINGS_DIALOG_FILLER_H_

#include "utils/GTUtilsDialog.h"

namespace U2 {
using namespace HI;

class GraphSettingsDialogFiller: public Filler
{
public:
    GraphSettingsDialogFiller(HI::GUITestOpStatus &os, int _window = -1, int _steps = -1, double _cutoff_min = 0, double _cutoff_max = 0,
                              int _r = -1, int _g = -1, int _b = -1)
        : Filler(os, "GraphSettingsDialog"), window(_window), steps(_steps), cutoff_min(_cutoff_min), cutoff_max(_cutoff_max),
    r(_r), g(_g), b(_b){}
    GraphSettingsDialogFiller(HI::GUITestOpStatus &os, CustomScenario *c): Filler(os, "GraphSettingsDialog", c){}
    virtual void commonScenario();
private:
    int window;
    int steps;
    double cutoff_min;
    double cutoff_max;
    int r, g, b;
};

}
#endif // GRAPHSETTINGSDIALOGFILLER_H
