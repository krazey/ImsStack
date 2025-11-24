/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceTrace.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlockListener.h"
#include "interface/IAosBlockSilentListener.h"
#include "condition/AosBlock.h"

__IMS_TRACE_TAG_AOS__;

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosBlock::AosBlock(IN IAosAppContext* piAppContext) :
        m_piAppContext(piAppContext),
        BLOCK_ENABLED(1),
        REASON{},
        objServiceBlockReasons(ImsList<IMS_UINT32>())
{
    m_strTag.Sprintf("%d:%s", m_piAppContext->GetSlotId(), m_piAppContext->GetProfileId().GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosBlock = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosBlock), this);
}

PUBLIC VIRTUAL AosBlock::~AosBlock()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosBlock = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosBlock), this);

    m_objListeners.Clear();
}

PUBLIC VIRTUAL void AosBlock::SetListener(IN IAosBlockListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosBlockListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            A_IMS_TRACE_D(
                    APPPROFILE, "SetListener :: (%" PFLS_X ") is already set", piListener, 0, 0);
            return;
        }
    }

    m_objListeners.Append(piListener);

    A_IMS_TRACE_D(APPPROFILE, "SetListener :: (%" PFLS_X ") is set", piListener, 0, 0);
}

PUBLIC VIRTUAL void AosBlock::RemoveListener(IN IAosBlockListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosBlockListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objListeners.RemoveAt(i);

            A_IMS_TRACE_D(
                    APPPROFILE, "RemoveListener :: (%" PFLS_X ") is removed", piListener, 0, 0);
            return;
        }
    }
}

PUBLIC VIRTUAL void AosBlock::SetSilentListener(IN IAosBlockSilentListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objSilentListeners.GetSize(); ++i)
    {
        IAosBlockSilentListener* pTmpListener = m_objSilentListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            A_IMS_TRACE_D(APPPROFILE, "SetSilentListener :: (%" PFLS_X ") is already set",
                    piListener, 0, 0);
            return;
        }
    }

    m_objSilentListeners.Append(piListener);
    A_IMS_TRACE_D(APPPROFILE, "SetSilentListener :: (%" PFLS_X ") is set", piListener, 0, 0);
}

PUBLIC VIRTUAL void AosBlock::RemoveSilentListener(IN IAosBlockSilentListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objSilentListeners.GetSize(); ++i)
    {
        IAosBlockSilentListener* pTmpListener = m_objSilentListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objSilentListeners.RemoveAt(i);

            A_IMS_TRACE_D(APPPROFILE, "RemoveSilentListener :: (%" PFLS_X ") is removed",
                    piListener, 0, 0);
            return;
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosBlock::SetBlockReason(
        IN BLOCK_REASON eReason, IN IMS_BOOL bNotify /* = IMS_TRUE */)
{
    if (IsReasonBlocked(eReason))
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(APPPROFILE, "SetBlockReason :: (%s)", BlockReasonToString(eReason), 0, 0);

    IMS_UINT32 eType = GetBlockType(eReason);
    if (eType == BLOCK_CELLULAR)
    {
        m_objBlockCellular.SetAt(&REASON[eReason], &BLOCK_ENABLED);
    }
    else if (eType == BLOCK_WIFI)
    {
        m_objBlockWifi.SetAt(&REASON[eReason], &BLOCK_ENABLED);
    }
    else  // if (eType == BLOCK_COMMON)
    {
        m_objBlock.SetAt(&REASON[eReason], &BLOCK_ENABLED);
    }

    Notify(eReason, IMS_TRUE, bNotify);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosBlock::ResetBlockReason(
        IN BLOCK_REASON eReason, IN IMS_BOOL bNotify /* = IMS_TRUE */)
{
    if (!IsReasonBlocked(eReason))
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(APPPROFILE, "ResetBlockReason :: (%s)", BlockReasonToString(eReason), 0, 0);

    IMS_UINT32 eType = GetBlockType(eReason);
    if (eType == BLOCK_CELLULAR)
    {
        m_objBlockCellular.RemoveKey(&REASON[eReason]);
    }
    else if (eType == BLOCK_WIFI)
    {
        m_objBlockWifi.RemoveKey(&REASON[eReason]);
    }
    else  // if (eType == BLOCK_COMMON)
    {
        m_objBlock.RemoveKey(&REASON[eReason]);
    }

    Notify(eReason, IMS_FALSE, bNotify);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosBlock::ClearAllBlockReasons()
{
    A_IMS_TRACE_I(APPPROFILE, "ClearAllBlockReasons", 0, 0, 0);
    m_objBlock.RemoveAll();
    m_objBlockCellular.RemoveAll();
    m_objBlockWifi.RemoveAll();

    Notify(BLOCK_MAX, IMS_FALSE);
}

PUBLIC VIRTUAL void AosBlock::GetBlockReasonsString(OUT AString& strOutLog)
{
    AString strLogCom;
    for (IMS_UINT32 i = BLOCK_START; i <= BLOCK_END; i++)
    {
        if (m_objBlock.GetValueAt(&REASON[i]) == &BLOCK_ENABLED)
        {
            strLogCom.Append("[");
            strLogCom.Append(BlockReasonToString(i));
            strLogCom.Append("]");
        }
    }

    AString strLogCell;
    for (IMS_UINT32 i = BLOCK_CELLULAR_START; i <= BLOCK_CELLULAR_END; i++)
    {
        if (m_objBlockCellular.GetValueAt(&REASON[i]) == &BLOCK_ENABLED)
        {
            strLogCell.Append("[");
            strLogCell.Append(BlockReasonToString(i));
            strLogCell.Append("]");
        }
    }

    AString strLogWifi;
    for (IMS_UINT32 i = BLOCK_WIFI_START; i <= BLOCK_WIFI_END; i++)
    {
        if (m_objBlockWifi.GetValueAt(&REASON[i]) == &BLOCK_ENABLED)
        {
            strLogWifi.Append("[");
            strLogWifi.Append(BlockReasonToString(i));
            strLogWifi.Append("]");
        }
    }

    strOutLog.Sprintf("Common(%d):%s Cellular(%d):%s WiFi(%d):%s", m_objBlock.GetSize(),
            strLogCom.GetStr(), m_objBlockCellular.GetSize(), strLogCell.GetStr(),
            m_objBlockWifi.GetSize(), strLogWifi.GetStr());
}

PUBLIC VIRTUAL IMS_BOOL AosBlock::PrintBlockReasons()
{
    AString strLog;
    GetBlockReasonsString(strLog);
    A_IMS_TRACE_I(APPPROFILE, "PrintBlockReasons :: %s", strLog.GetStr(), 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosBlock::GetBlockReasons(
        OUT ImsList<IMS_UINT32>& objReasons, IN SERVICE_TYPE eType /* = SERVICE_WHOLE*/)
{
    objReasons.Clear();

    for (IMS_UINT32 i = BLOCK_START; i <= BLOCK_END; i++)
    {
        if (m_objBlock.GetValueAt(&REASON[i]) == &BLOCK_ENABLED)
        {
            objReasons.Append(i);
        }
    }

    if (eType == SERVICE_CELLULAR || eType == SERVICE_WHOLE)
    {
        for (IMS_UINT32 i = BLOCK_CELLULAR_START; i <= BLOCK_CELLULAR_END; i++)
        {
            if (m_objBlockCellular.GetValueAt(&REASON[i]) == &BLOCK_ENABLED)
            {
                objReasons.Append(i);
            }
        }
    }

    if (eType == SERVICE_WIFI || eType == SERVICE_WHOLE)
    {
        for (IMS_UINT32 i = BLOCK_WIFI_START; i <= BLOCK_WIFI_END; i++)
        {
            if (m_objBlockWifi.GetValueAt(&REASON[i]) == &BLOCK_ENABLED)
            {
                objReasons.Append(i);
            }
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosBlock::IsReasonBlocked(IN BLOCK_REASON eReason,
        IN IMS_BOOL bOnlyEnabled /* = IMS_FALSE */,
        IN SERVICE_TYPE eServiceType /* = SERVICE_CELLULAR*/)
{
    if (bOnlyEnabled)
    {
        if (eServiceType == SERVICE_CELLULAR &&
                m_objBlockCellular.GetSize() + m_objBlock.GetSize() != 1)
        {
            return IMS_FALSE;
        }
        if (eServiceType == SERVICE_WIFI && m_objBlockWifi.GetSize() + m_objBlock.GetSize() != 1)
        {
            return IMS_FALSE;
        }
        if (eServiceType == SERVICE_WHOLE &&
                m_objBlockCellular.GetSize() + m_objBlockWifi.GetSize() + m_objBlock.GetSize() != 1)
        {
            return IMS_FALSE;
        }
    }

    IMS_BOOL bResult = IMS_FALSE;

    IMS_UINT32 eReasonType = GetBlockType(eReason);
    if (eReasonType == BLOCK_COMMON)
    {
        bResult = (m_objBlock.GetValueAt(&REASON[eReason]) == &BLOCK_ENABLED);
    }
    else if (eReasonType == BLOCK_CELLULAR)
    {
        bResult = (m_objBlockCellular.GetValueAt(&REASON[eReason]) == &BLOCK_ENABLED);
    }
    else if (eReasonType == BLOCK_WIFI)
    {
        bResult = (m_objBlockWifi.GetValueAt(&REASON[eReason]) == &BLOCK_ENABLED);
    }

    A_IMS_TRACE_D(APPPROFILE, "IsReasonBlocked :: eReason (%s) - (%s)",
            BlockReasonToString(eReason), bResult ? "BLOCKED" : "NOT_BLOCKED", 0);

    return bResult;
}

PUBLIC VIRTUAL IMS_BOOL AosBlock::IsCleared(IN SERVICE_TYPE nType /* = SERVICE_CELLULAR*/)
{
    IMS_BOOL bResult = IMS_FALSE;

    if (nType == SERVICE_CELLULAR)
    {
        if (m_objBlockCellular.IsEmpty() && m_objBlock.IsEmpty())
        {
            bResult = IMS_TRUE;
        }
    }
    else if (nType == SERVICE_WIFI)
    {
        if (m_objBlockWifi.IsEmpty() && m_objBlock.IsEmpty())
        {
            bResult = IMS_TRUE;
        }
    }
    else if (nType == SERVICE_WHOLE)
    {
        if (m_objBlockCellular.IsEmpty() && m_objBlockWifi.IsEmpty() && m_objBlock.IsEmpty())
        {
            bResult = IMS_TRUE;
        }
    }

    A_IMS_TRACE_I(APPPROFILE, "IsCleared :: SERVICE_TYPE(%s), bResult(%s)",
            ServiceTypeToString(nType), _TRACE_B_(bResult), 0);
    return bResult;
}

PUBLIC GLOBAL const IMS_CHAR* AosBlock::BlockReasonToString(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case BLOCK_AC_INCOMPLETED:
            return "AC_INCOMPLETED";

        case BLOCK_AUTHENTICATION_FAILED:
            return "AUTHENTICATION_FAILED";

        case BLOCK_USIM_AUTHENTICATION_FAILED:
            return "USIM_AUTHENTICATION_FAILED";

        case BLOCK_AOS_INCOMPLETED:
            return "AOS_INCOMPLETED";

        case BLOCK_CSCALL_STARTED:
            return "CSCALL_STARTED";

        case BLOCK_PERMANENT_DATA_FAILED:
            return "PERMANENT_DATA_FAILED";

        case BLOCK_ENABLER_DETACHED:
            return "ENABLER_DETACHED";

        case BLOCK_IMS_DISABLED:
            return "IMS_DISABLED";

        case BLOCK_PERMANENT_REG_FAILED:
            return "PERMANENT_REG_FAILED";

        case BLOCK_POWER_OFF:
            return "POWER_OFF";

        case BLOCK_SERVICE_CONNECTING:
            return "SERVICE_CONNECTING";

        case BLOCK_SUBSCRIBER_INCOMPLETED:
            return "SUBSCRIBER_INCOMPLETED";

        case BLOCK_TTY_MODE_ON:
            return "TTY_MODE_ON";

        case BLOCK_TEMPORARY_DATA_DEACTIVATED:
            return "TEMPORARY_DATA_DEACTIVATED";

        case BLOCK_IMS_SERVICE_DISABLED:
            return "IMS_SERVICE_DISABLED";

        case BLOCK_EPS_FALLBACK_STARTED:
            return "EPS_FALLBACK_STARTED";

        case BLOCK_INVALID_CONNECTION:
            return "INVALID_CONNECTION";

        case BLOCK_CELLULAR_AIRPLANE_MODE_ON:
            return "CELLULAR_AIRPLANE_MODE_ON";

        case BLOCK_CELLULAR_NO_NETWORK:
            return "CELLULAR_NO_NETWORK";

        case BLOCK_CELLULAR_OUT_OF_SERVICE:
            return "CELLULAR_OUT_OF_SERVICE";

        case BLOCK_CELLULAR_RAT_BLOCK:
            return "CELLULAR_RAT_BLOCK";

        case BLOCK_CELLULAR_ROAMING:
            return "CELLULAR_ROAMING";

        case BLOCK_WIFI_BAD_CONNECTION:
            return "WIFI_BAD_CONNECTION";

        case BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE:
            return "WIFI_COUNTRY_CODE_UNAVAILABLE";

        case BLOCK_WIFI_AIRPLANE_MODE_ON:
            return "WIFI_AIRPLANE_MODE_ON";

        case BLOCK_WIFI_NO_WIFI:
            return "WIFI_NO_WIFI";

        case BLOCK_WIFI_REG_FORBIDDEN:
            return "WIFI_REG_FORBIDDEN";

        case BLOCK_WIFI_TEMPORARILY_BLOCKED:
            return "WIFI_TEMPORARILY_BLOCKED";

        default:
            return "INVALID";
    }
}

PROTECTED
void AosBlock::Notify(
        IN BLOCK_REASON eReason, IN IMS_BOOL bIsEnable, IN IMS_BOOL bNotify /*= IMS_TRUE*/)
{
    if (bNotify)
    {
        for (IMS_UINT32 nAt = 0; nAt < m_objListeners.GetSize(); nAt++)
        {
            IAosBlockListener* piListener = m_objListeners.GetAt(nAt);
            piListener->Block_Changed(eReason, (bIsEnable) ? 1 : 0);
        }
    }
    else
    {
        for (IMS_UINT32 nAt = 0; nAt < m_objSilentListeners.GetSize(); nAt++)
        {
            IAosBlockSilentListener* piListener = m_objSilentListeners.GetAt(nAt);
            piListener->Block_SilentChanged(eReason, (bIsEnable) ? 1 : 0);
        }
    }
}

PROTECTED GLOBAL IMS_UINT32 AosBlock::GetBlockType(IN BLOCK_REASON eReason)
{
    if (eReason >= BLOCK_CELLULAR_START && eReason <= BLOCK_CELLULAR_END)
    {
        return BLOCK_CELLULAR;
    }
    else if (eReason >= BLOCK_WIFI_START && eReason <= BLOCK_WIFI_END)
    {
        return BLOCK_WIFI;
    }
    else  // if (eReason >= BLOCK_START && eReason <= BLOCK_END)
    {
        return BLOCK_COMMON;
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosBlock::ServiceTypeToString(IN SERVICE_TYPE eType)
{
    switch (eType)
    {
        case SERVICE_CELLULAR:
            return "SERVICE_CELLULAR";

        case SERVICE_WIFI:
            return "SERVICE_WIFI";

        default:
            return "SERVICE_WHOLE";
    }
}
