/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090716  lovil@                    Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "SystemConfig.h"
#include "base/IMS.h"

PRIVATE GLOBAL IMS_SINT32* IMS::ERROR_CODE = IMS_NULL;

PUBLIC GLOBAL void IMS::Init()
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ERROR_CODE = new IMS_SINT32[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; i++)
    {
        Init(i);
    }
}

PUBLIC GLOBAL void IMS::Init(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return;
    }

    if (ERROR_CODE != IMS_NULL)
    {
        ERROR_CODE[nSlotId] = 0;
    }
}

PUBLIC GLOBAL void IMS::SetLastError(IN IMS_SINT32 nErrorCode)
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    SetLastError(nErrorCode, nSlotId);
}

PUBLIC GLOBAL IMS_SINT32 IMS::GetLastError()
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    return GetLastError(nSlotId);
}

PRIVATE GLOBAL void IMS::SetLastError(IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return;
    }

    if (ERROR_CODE != IMS_NULL)
    {
        ERROR_CODE[nSlotId] = nErrorCode;
    }
}

PRIVATE GLOBAL IMS_SINT32 IMS::GetLastError(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return 0;
    }

    return (ERROR_CODE != IMS_NULL) ? ERROR_CODE[nSlotId] : 0;
}
