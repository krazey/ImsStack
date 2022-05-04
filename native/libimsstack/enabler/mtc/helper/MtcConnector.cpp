#include "ServiceTrace.h"

#include "helper/CallStateProxy.h"
#include "IMtcCallStateListener.h"
#include "IMtcContext.h"
#include "MtcConnector.h"
#include "MtcContextRepository.h"

PUBLIC GLOBAL void MtcConnector::AddCallStateListener(
        IN IMS_SINT32 nSlotId, IN IMtcCallStateListener* pListener)
{
    //----------------------------------------------------------------------------------------------

    IMtcContext* piContext = MtcContextRepository::GetContext(nSlotId);
    if (piContext)
    {
        piContext->GetCallStateProxy().AddListener(pListener);
    }
}

PUBLIC GLOBAL void MtcConnector::RemoveCallStateListener(
        IN IMS_SINT32 nSlotId, IN IMtcCallStateListener* pListener)
{
    //----------------------------------------------------------------------------------------------

    IMtcContext* piContext = MtcContextRepository::GetContext(nSlotId);
    if (piContext)
    {
        piContext->GetCallStateProxy().RemoveListener(pListener);
    }
}
