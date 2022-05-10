#include "MtsClientFactory.h"
#include "AndroidJavaWms.h"

#include "ServiceTrace.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
MtsClientFactory::MtsClientFactory()
{
    IMS_TRACE_D("MtsClientFactory", 0, 0, 0);
}

PUBLIC VIRTUAL MtsClientFactory::~MtsClientFactory()
{
    IMS_TRACE_D("~MtsClientFactory", 0, 0, 0);
}

PUBLIC GLOBAL IMtsClient* MtsClientFactory::GetIMtsJavaClient(IN IMS_SINT32 nSlotID)
{
    IMS_TRACE_D("GetIMtsJavaClient: slot:[%d]", nSlotID, 0, 0);
    return AndroidJavaWMS::GetAndroidJavaWMS(nSlotID);
}

PUBLIC GLOBAL void MtsClientFactory::DestroyIMtsJavaClient(IN IMS_SINT32 nSlotID)
{
    IMS_TRACE_D("DestroyIMtsJavaClient: slot[%d]", nSlotID, 0, 0);
    AndroidJavaWMS::DestroyAndroidJavaWMS(nSlotID);
}
