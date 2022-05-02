/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110528  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SipFeatures.h"
#include "SIPFactoryProxy.h"
#include "SIPIPSecState.h"
#include "SIPKeepAliveHelper.h"
#include "SIPMessageTracker.h"
#include "SIPPacketTracker.h"
#include "SipRoutingRejectNotifier.h"
#include "SIPRTConfigHelper.h"
#include "SIPTransportHelper.h"
#include "SIPUtil.h"
#include "SipFactory.h"



/*

Remarks

*/
PUBLIC GLOBAL
ISIPKeepAliveHelper* SIPFactory::CreateKeepAliveHelper(IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    return new SIPKeepAliveHelper(nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL
void SIPFactory::GenerateCallId(IN const AString &strHost, OUT AString &strCallId)
{
    strCallId = SIPUtil::GenerateCallId(strHost);
}

/*

Remarks
 HEADER_REQ_SESSION-ID
*/
PUBLIC GLOBAL
void SIPFactory::GenerateSessionId(IN IMS_SINT32 nSlotId,
        IN const AString &strCallId, OUT AString &strSessionId)
{
    if (SIPFeatures::IsHeaderSessionIdRequired(nSlotId))
    {
        strSessionId = SIPUtil::GenerateSessionId(nSlotId, strCallId);
        return;
    }

    strSessionId = AString::ConstNull();
}

/*

Remarks

*/
PUBLIC GLOBAL
ISIPIPSecState* SIPFactory::GetIPSecState(IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    return SIPFactoryProxy::GetInstance()->GetIPSecState(nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL
ISIPMessageTracker* SIPFactory::GetMessageTracker(IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    return SIPFactoryProxy::GetInstance()->GetMessageTracker(nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL
ISIPRoutingRejectNotifier* SIPFactory::GetRoutingRejectNotifier(
        IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    return SIPFactoryProxy::GetInstance()->GetRoutingRejectNotifier(nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL
ISIPRTConfigHelper* SIPFactory::GetRTConfigHelper(IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    return SIPFactoryProxy::GetInstance()->GetRTConfigHelper(nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL
ISIPTransportHelper* SIPFactory::GetTransportHelper(IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    return SIPFactoryProxy::GetInstance()->GetTransportHelper(nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL
ISIPPacketTracker* SIPFactory::GetPacketTracker(IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    return SIPFactoryProxy::GetInstance()->GetPacketTracker(nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL
void SIPFactory::SetTokenGenerator(IN IMS_SINT32 nSlotId,
        IN ISIPTokenGenerator *piTokenGenerator)
{
    SIPFactoryProxy::GetInstance()->SetTokenGenerator(nSlotId, piTokenGenerator);
}
