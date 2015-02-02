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

#ifndef _U2_GT_RUNNABLES_POPUP_CHOOSER_FILLER_H_
#define _U2_GT_RUNNABLES_POPUP_CHOOSER_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

    class PopupChooser : public Filler {
        friend class PopupChecker;
        friend class PopupCheckerByText;
    public:
        PopupChooser(U2OpStatus &_os, const QStringList &_namePath, GTGlobals::UseMethod _useMethod = GTGlobals::UseKey) //UseKey need for Ubuntu
            :Filler(_os, GUIDialogWaiter::WaitSettings(QString(), GUIDialogWaiter::Popup)), namePath(_namePath), useMethod(_useMethod){}
        virtual void run();
    protected:
        QStringList namePath;
        GTGlobals::UseMethod useMethod;

    private:
        static void clickEsc();
    };

    class PopupChooserbyText : public Filler {
    public:
        PopupChooserbyText(U2OpStatus &_os, const QStringList &_namePath, GTGlobals::UseMethod _useMethod = GTGlobals::UseKey) //UseKey need for Ubuntu
            :Filler(_os, GUIDialogWaiter::WaitSettings(QString(), GUIDialogWaiter::Popup)), namePath(_namePath), useMethod(_useMethod){}
        virtual void run();
    protected:
        QStringList namePath;
        GTGlobals::UseMethod useMethod;
    };

    class PopupChecker : public Filler {
    public:
        enum CheckOption{
            NotExists = 0,
            Exists = 1,
            IsEnabled = 2 | Exists,
            IsDisabled = 4 | Exists,
            IsChecable = 8 | Exists,
            IsChecked = 16 | Exists | IsChecable
        };
        Q_DECLARE_FLAGS(CheckOptions, CheckOption)
        PopupChecker(U2OpStatus &os, CustomScenario *scenario);
        PopupChecker(U2OpStatus &_os, const QStringList &_namePath, CheckOptions _options = CheckOptions(IsEnabled),
                     GTGlobals::UseMethod _useMethod = GTGlobals::UseKey) //UseKey need for Ubuntu
            :Filler(_os, GUIDialogWaiter::WaitSettings(QString(), GUIDialogWaiter::Popup)), namePath(_namePath), options(_options), useMethod(_useMethod){}
        virtual void commonScenario();
    protected:
        QStringList namePath;
        CheckOptions options;
        GTGlobals::UseMethod useMethod;
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS(PopupChecker::CheckOptions)

    class PopupCheckerByText : public Filler {
    public:
        PopupCheckerByText(U2OpStatus &os, CustomScenario *scenario);
        PopupCheckerByText(U2OpStatus &_os, const QStringList &_namePath, PopupChecker::CheckOptions _options = PopupChecker::CheckOptions(PopupChecker::IsEnabled),
                     GTGlobals::UseMethod _useMethod = GTGlobals::UseKey) //UseKey need for Ubuntu
            :Filler(_os, GUIDialogWaiter::WaitSettings(QString(), GUIDialogWaiter::Popup)), textPath(_namePath), options(_options), useMethod(_useMethod){}
        virtual void commonScenario();
    protected:
        QStringList textPath;
        PopupChecker::CheckOptions options;
        GTGlobals::UseMethod useMethod;
    };
}

#endif
