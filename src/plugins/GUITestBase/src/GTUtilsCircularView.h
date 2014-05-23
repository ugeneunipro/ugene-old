#ifndef _GT_UTILS_CIRCULAR_VIEW_H_
#define _GT_UTILS_CIRCULAR_VIEW_H_

#include <QAbstractButton>

#include "api/GTGlobals.h"


namespace U2{

class ADVSingleSequenceWidget;

class GTUtilsCv
{
public:
    /**
     * Returns "true", if CV widget exists in @seqWidget.
     * Returns "false", if it does not exist or in case of an error.
     */
    static bool isCvPresent(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);

    /** Test utils for CV button on a sequence widget */
    class cvBtn {
        public:
        /** Returns "true", if there is CV button in @seqWidget */
        static bool isPresent(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);

        /**
         * Returns "true", if there is CV button in @seqWidget, it is checkable and checked
         * Status @os is set to error if the button does not exist on @seqWidget or it is not checkable.
         */
        static bool isChecked(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);

        /**
         * Clicks CV button in @seqWidget.
         * Status @os is set to error if the button does not exist.
         */
        static void click(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);
    };

    /** Test utils for CV button on the Sequence View toolbar (common for several sequences) */
    class commonCvBtn {
        public:
        /** Status @os is set to an error if there is no CV button on the main toolbar */
        static void mustExist(U2OpStatus& os);

        /**
         * Clicks CV button on the main toolbar.
         * Status @os is set to error if the button does not exist.
         */
        static void click(U2OpStatus& os);
    };


private:
    static QAbstractButton * getCvButton(U2OpStatus& os, ADVSingleSequenceWidget* seqWidget, bool setFailedIfNotFound);

    static const QString actionName;
};

}

#endif // _GT_UTILS_CIRCULAR_VIEW_H_


#ifndef _GT_UTILS_CIRCULAR_VIEW_H_
#define _GT_UTILS_CIRCULAR_VIEW_H_

#include <QAbstractButton>

#include "api/GTGlobals.h"


namespace U2{

class ADVSingleSequenceWidget;

class GTUtilsCv
{
public:
    /**
     * Returns "true", if CV widget exists in @seqWidget.
     * Returns "false", if it does not exist or in case of an error.
     */
    static bool isCvPresent(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);

    /** Test utils for CV button on a sequence widget */
    class cvBtn {
        public:
        /** Returns "true", if there is CV button in @seqWidget */
        static bool isPresent(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);

        /**
         * Returns "true", if there is CV button in @seqWidget, it is checkable and checked
         * Status @os is set to error if the button does not exist on @seqWidget or it is not checkable.
         */
        static bool isChecked(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);

        /**
         * Clicks CV button in @seqWidget.
         * Status @os is set to error if the button does not exist.
         */
        static void click(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget);
    };

    /** Test utils for CV button on the Sequence View toolbar (common for several sequences) */
    class commonCvBtn {
        public:
    //    /**
    //     * 
    //     */
    //    static void mustExist(U2OpStatus& os, ADVSingleSequenceWidget* w);

    //    /**
    //     * 
    //     */
    //    static void push(U2OpStatus& os, ADVSingleSequenceWidget* w);
    //};


private:
    static QAbstractButton * getCvButton(U2OpStatus& os, ADVSingleSequenceWidget* seqWidget, bool setFailedIfNotFound);

    static const QString actionName;
};

}

#endif // _GT_UTILS_CIRCULAR_VIEW_H_
