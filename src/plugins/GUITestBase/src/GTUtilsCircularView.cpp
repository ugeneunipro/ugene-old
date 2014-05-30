#include "GTUtilsCircularView.h"

#include <api/GTAction.h>
#include <api/GTGlobals.h>
#include <api/GTWidget.h>

#include <U2Core/U2SafePoints.h>

#include <U2View/ADVSingleSequenceWidget.h>


namespace U2{
const QString GTUtilsCv::actionName = "CircularViewAction";

#define GT_CLASS_NAME "GTUtilsCv"


//////////////////////////////////////////////////////////////////////////
// CV common test utils
//////////////////////////////////////////////////////////////////////////
#define GT_METHOD_NAME "cvBtn::isPresent"
bool GTUtilsCv::isCvPresent(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget) {
    CHECK_SET_ERR_RESULT(NULL != seqWidget, "NULL sequence widget!", false);

    QString cvWidgetName = "CV_" + seqWidget->objectName();
    QWidget *cvWidget = GTWidget::findWidget(os, cvWidgetName, NULL, GTGlobals::FindOptions(false));
    CHECK_SET_ERR_RESULT(!os.isCoR(), "Error getting CV widget!", false);

    return NULL != cvWidget;
}
#undef GT_METHOD_NAME


//////////////////////////////////////////////////////////////////////////
// GTUtilsCv::cvBtn
//////////////////////////////////////////////////////////////////////////
#define GT_METHOD_NAME "cvBtn::isPresent"
bool GTUtilsCv::cvBtn::isPresent(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget) {
    QAbstractButton *cvButton = getCvButton(os, seqWidget, false);
    CHECK_OP_SET_ERR_RESULT(os, "Error getting CV button!", false);

    return NULL != cvButton;
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "cvBtn::isChecked"
bool GTUtilsCv::cvBtn::isChecked(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget) {
    QAbstractButton *cvButton = getCvButton(os, seqWidget, true /* CV button must exist */ );

    CHECK_OP_SET_ERR_RESULT(os, "Error getting CV button!", false);
    SAFE_POINT(NULL != cvButton, "cvButton is NULL!", false);

    CHECK_SET_ERR_RESULT(cvButton->isCheckable(), "CV button is not checkable!", false);

    return cvButton->isChecked();
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "cvBtn::click"
void GTUtilsCv::cvBtn::click(U2OpStatus &os, ADVSingleSequenceWidget* seqWidget) {
    QAbstractButton *cvButton = getCvButton(os, seqWidget, true /* CV button must exist */ );

    CHECK_OP_SET_ERR(os, "Error getting CV button!");
    SAFE_POINT(NULL != cvButton, "cvButton is NULL!",);

    GTWidget::click(os, cvButton);
    CHECK_OP_SET_ERR(os, "Error clicking CV button!");
}
#undef GT_METHOD_NAME


//////////////////////////////////////////////////////////////////////////
// GTUtilsCv::commonCvBtn
//////////////////////////////////////////////////////////////////////////
#define GT_METHOD_NAME "commonCvBtn::mustExist"
void GTUtilsCv::commonCvBtn::mustExist(U2OpStatus &os){
    GTWidget::findWidget(os, "globalToggleViewAction_widget");
    CHECK_OP_SET_ERR(os, "Error getting global CV button!");
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "commonCvBtn::click"
void GTUtilsCv::commonCvBtn::click(U2OpStatus& os){
    QWidget* button = GTWidget::findWidget(os, "globalToggleViewAction_widget");
    CHECK_OP_SET_ERR(os, "Error getting global CV button!");

    if(!button->isVisible()){
        QWidget* ext_button = GTWidget::findWidget(os, "qt_toolbar_ext_button", GTWidget::findWidget(os, "mwtoolbar_activemdi"), GTGlobals::FindOptions(false));
        if(ext_button != NULL){
            GTWidget::click(os, ext_button);
        }
        GTGlobals::sleep(500);
    }
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME


////////////////////////////////////////////////////////////////////////
// Helper methods
////////////////////////////////////////////////////////////////////////

QAbstractButton * GTUtilsCv::getCvButton(U2OpStatus &os, ADVSingleSequenceWidget *seqWidget, bool setFailedIfNotFound) {
    if ((NULL == seqWidget) && (!os.isCoR())) {
        os.setError("NULL sequence widget!");
        return NULL;
    }
    QAbstractButton* cvButton = GTAction::button(os, actionName, seqWidget, GTGlobals::FindOptions(setFailedIfNotFound));
    return cvButton;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME
}

