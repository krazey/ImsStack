#include "ServiceTrace.h"
#include "AString.h"
#include "IMtcContext.h"
#include "dialingplan/EmergencyDialingPlan.h"
#include "dialingplan/EmergencyNumberList.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL AString& EmergencyDialingPlan::GetTranslatedUri(
        IN IMtcContext& objContext, IN_OUT AString& strNumber)
{
    if (IsTestNumber(strNumber))
    {
        // TODO: vzw
        return strNumber;
    }

    EmergencyNumberList* pEnl = new EmergencyNumberList(objContext.GetSlotId());
    strNumber = pEnl->GetEmergencyServiceURN(strNumber);  // TODO:
    delete pEnl;

    return strNumber;
}

PRIVATE GLOBAL IMS_BOOL EmergencyDialingPlan::IsTestNumber(IN const AString& /*strNumber*/)
{
    // TODO: vzw.
    return IMS_FALSE;
}
