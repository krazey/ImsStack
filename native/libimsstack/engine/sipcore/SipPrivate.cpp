/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090904  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "SystemConfig.h"
#include "SipPrivate.h"

PRIVATE GLOBAL IMS_SINT32* SIPPrivate::ERROR_CODE = IMS_NULL;

PRIVATE GLOBAL IMS_SINT32* SIPPrivate::ENCODING_OPTIONS = IMS_NULL;

PUBLIC GLOBAL void SIPPrivate::Init()
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ERROR_CODE = new IMS_SINT32[nSimCount];
    ENCODING_OPTIONS = new IMS_SINT32[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; i++)
    {
        Init(i, OPTIONS_E | OPT_E_FULLFORM);
    }
}

PUBLIC GLOBAL void SIPPrivate::Init(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nEncodingOptions)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return;
    }

    if (ERROR_CODE != IMS_NULL)
    {
        ERROR_CODE[nSlotId] = 0;
    }

    if (ENCODING_OPTIONS != IMS_NULL)
    {
        ENCODING_OPTIONS[nSlotId] = nEncodingOptions;
    }
}

PUBLIC GLOBAL void SIPPrivate::SetLastError(IN IMS_SINT32 nErrorCode)
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    SetLastError(nErrorCode, nSlotId);
}

PUBLIC GLOBAL IMS_SINT32 SIPPrivate::GetLastError()
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    return GetLastError(nSlotId);
}

PUBLIC GLOBAL IMS_SINT32 SIPPrivate::GetEncodingOptions()
{
    IMS_SINT32 nSlotId = IMS_SLOT_0;

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    return GetEncodingOptions(nSlotId);
}

PRIVATE GLOBAL void SIPPrivate::SetLastError(IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSlotId)
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

PRIVATE GLOBAL IMS_SINT32 SIPPrivate::GetLastError(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return 0;
    }

    return (ERROR_CODE != IMS_NULL) ? ERROR_CODE[nSlotId] : 0;
}

PRIVATE GLOBAL IMS_SINT32 SIPPrivate::GetEncodingOptions(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return (OPTIONS_E | OPT_E_FULLFORM);
    }

    return (ENCODING_OPTIONS != IMS_NULL) ? ENCODING_OPTIONS[nSlotId]
                                          : (OPTIONS_E | OPT_E_FULLFORM);
}
