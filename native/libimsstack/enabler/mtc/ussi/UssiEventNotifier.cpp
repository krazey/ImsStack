#include "ServiceTrace.h"

#include "call/IMtcCallContext.h"
#include "call/MtcUiNotifier.h"
#include "ussi/UssiDef.h"
#include "ussi/UssiEventNotifier.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UssiEventNotifier::UssiEventNotifier(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
    IMS_TRACE_I("+UssiEventNotifier", 0, 0, 0);
}

PUBLIC VIRTUAL UssiEventNotifier::~UssiEventNotifier()
{
    IMS_TRACE_I("~UssiEventNotifier", 0, 0, 0);
}

PUBLIC
void UssiEventNotifier::NotifyUssiError(IN AString strUssdString)
{
    IMS_TRACE_D("NotifyUssiError", 0, 0, 0);
    m_objContext.GetUiNotifier().SendNotifyInfo(
            INFO_TYPE_USSI, strUssdString, (IMS_SINT32)UssiModeType::ERROR);
}

PUBLIC
void UssiEventNotifier::NotifyUssiResult(IN AString strUssdString, IN UssiModeType eType)
{
    IMS_TRACE_D("NotifyUssiResult", 0, 0, 0);
    m_objContext.GetUiNotifier().SendNotifyInfo(INFO_TYPE_USSI, strUssdString, (IMS_SINT32)eType);
}
