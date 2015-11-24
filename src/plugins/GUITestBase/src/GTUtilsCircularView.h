#ifndef _GT_UTILS_CIRCULAR_VIEW_H_
#define _GT_UTILS_CIRCULAR_VIEW_H_

#include <QAbstractButton>

#include "GTGlobals.h"


namespace U2{
using namespace HI;
class ADVSingleSequenceWidget;

class GTUtilsCv
{
public:
    /**
     * Returns "true", if CV widget exists in @seqWidget.
     * Returns "false", if it does not exist or in case of an error.
     */
    static bool isCvPresent(HI::GUITestOpStatus &os, ADVSingleSequenceWidget *seqWidget);

    /** Test utils for CV button on a sequence widget */
    class cvBtn {
        public:
        /** Returns "true", if there is CV button in @seqWidget */
        static bool isPresent(HI::GUITestOpStatus &os, ADVSingleSequenceWidget *seqWidget);

        /**
         * Returns "true", if there is CV button in @seqWidget, it is checkable and checked
         * Status @os is set to error if the button does not exist on @seqWidget or it is not checkable.
         */
        static bool isChecked(HI::GUITestOpStatus &os, ADVSingleSequenceWidget *seqWidget);

        /**
         * Clicks CV button in @seqWidget.
         * Status @os is set to error if the button does not exist.
         */
        static void click(HI::GUITestOpStatus &os, ADVSingleSequenceWidget *seqWidget);
    };

    /** Test utils for CV button on the Sequence View toolbar (common for several sequences) */
    class commonCvBtn {
        public:
        /** Status @os is set to an error if there is no CV button on the main toolbar */
        static void mustExist(HI::GUITestOpStatus& os);

        /**
         * Clicks CV button on the main toolbar.
         * Status @os is set to error if the button does not exist.
         */
        static void click(HI::GUITestOpStatus& os);
    };


private:
    static QAbstractButton * getCvButton(HI::GUITestOpStatus& os, ADVSingleSequenceWidget* seqWidget, bool setFailedIfNotFound);

    static const QString actionName;
};

}

#endif // _GT_UTILS_CIRCULAR_VIEW_H_
