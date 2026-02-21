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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SipDState.h"
#include "SipDebug.h"
#include "SipDialogEx.h"
#include "SipDialogInviteUsage.h"
#include "SipDialogSubscribeUsage.h"
#include "SipMessageInfo.h"
#include "SipPrivate.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipDialogEx::SipDialogEx(IN SipDialogState* pDState) :
        SipDialogBase(pDState),
        m_bIsPermanentDialog(IMS_FALSE),
        m_bInitWithDelay(IMS_FALSE),
        m_bIsDialogTerminated(IMS_FALSE),
        m_pDialogUsage(IMS_NULL)
{
}

PUBLIC
SipDialogEx::SipDialogEx(IN const SipDialogEx& other) :
        SipDialogBase(other),
        m_bIsPermanentDialog(other.m_bIsPermanentDialog),
        m_bInitWithDelay(other.m_bInitWithDelay),
        m_bIsDialogTerminated(other.m_bIsDialogTerminated),
        m_pDialogUsage(IMS_NULL)
{
    // NOTE: If reference count is not used, you MUST implement this copy constructor.
}

PUBLIC VIRTUAL SipDialogEx::~SipDialogEx()
{
    if (m_bIsPermanentDialog && !m_bIsDialogTerminated)
    {
        SipDialogState* pDState = GetDialogState();

        if (pDState != IMS_NULL)
        {
            pDState->RemoveDialogUsage(this);
        }
    }

    if (m_pDialogUsage != IMS_NULL)
    {
        delete m_pDialogUsage;
        m_pDialogUsage = IMS_NULL;
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipDialogEx(%s)",
            SipDebug::GetCharA1(GetDialogState()->GetCallId().GetStr(), 8, '@'), 0, 0);
#endif
}

PUBLIC
SipDialogEx& SipDialogEx::operator=(IN const SipDialogEx& other)
{
    if (this != &other)
    {
        SipDialogBase::operator=(other);
        m_bIsPermanentDialog = other.m_bIsPermanentDialog;
        m_bInitWithDelay = other.m_bInitWithDelay;
        m_bIsDialogTerminated = other.m_bIsDialogTerminated;
        // NOTE: If reference count is not used, this MUST be newly created.
        if (other.m_pDialogUsage == IMS_NULL)
        {
            m_pDialogUsage = IMS_NULL;
        }
        else
        {
            m_pDialogUsage = other.m_pDialogUsage->Clone();
        }
    }

    return (*this);
}

// For an initial requests
PUBLIC
IMS_BOOL SipDialogEx::InitDialog(IN const SipMethod& objMethod)
{
    if (objMethod.Equals(SipMethod::INVITE))
    {
        m_pDialogUsage = new SipDialogInviteUsage(this);
    }
    else if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER))
    {
        SipDialogSubscribeUsage* pSubscribeUsage = new SipDialogSubscribeUsage(this);

        if (pSubscribeUsage == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!pSubscribeUsage->InitDialogUsage(objMethod))
        {
            IMS_TRACE_E(0, "Initializing a dialog usage (%s) failed", objMethod.ToString().GetStr(),
                    0, 0);

            delete pSubscribeUsage;
            return IMS_FALSE;
        }

        m_pDialogUsage = pSubscribeUsage;
    }
    else
    {
        m_pDialogUsage = new SipDialogUsage(this);
    }

    if (m_pDialogUsage == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "Instantiating a dialog usage (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // Before sending a request message to the network,
    // if the method, "InitDialogWithDelay(...)" is invoked, this flag will be used to identify
    // whether the operation will be performed or not.
    m_bInitWithDelay = IMS_TRUE;

    return IMS_TRUE;
}

// For a dialog request or incoming requests
PUBLIC
IMS_BOOL SipDialogEx::InitDialog(IN const SipMessageInfo& objMsgInfo)
{
    const SipMethod& objMethod = objMsgInfo.GetMethod();

    if (objMethod.Equals(SipMethod::INVITE))
    {
        m_pDialogUsage = new SipDialogInviteUsage(this);
    }
    // For a forked NOTIFY request, adds NOTIFY method checking
    else if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER) ||
            objMethod.Equals(SipMethod::NOTIFY))
    {
        m_pDialogUsage = new SipDialogSubscribeUsage(this);
    }
    else
    {
        m_pDialogUsage = new SipDialogUsage(this);
    }

    if (m_pDialogUsage == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "Instantiating a dialog usage (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (!m_pDialogUsage->InitDialogUsage(objMsgInfo))
    {
        IMS_TRACE_E(
                0, "Initializing a dialog usage (%s) failed", objMethod.ToString().GetStr(), 0, 0);

        delete m_pDialogUsage;
        m_pDialogUsage = IMS_NULL;

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipDialogEx::InitDialogWithDelay(IN const SipMessageInfo& objMsgInfo)
{
    // If we don't need to initialize a dialog, it will be ignored in here.
    if (!m_bInitWithDelay)
    {
        return IMS_TRUE;
    }

    if (m_pDialogUsage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pDialogUsage->InitDialogUsage(objMsgInfo);
}

PUBLIC
IMS_BOOL SipDialogEx::CompareTo(IN const SipMessageInfo& objMsgInfo) const
{
    if (m_pDialogUsage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pDialogUsage->CompareTo(objMsgInfo);
}

PUBLIC
IMS_BOOL SipDialogEx::Equals(IN SipDialogEx* pDialogEx) const
{
    if (pDialogEx == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if ((m_pDialogUsage == IMS_NULL) || (pDialogEx->m_pDialogUsage == IMS_NULL))
    {
        return IMS_FALSE;
    }

    return m_pDialogUsage->Equals(pDialogEx->m_pDialogUsage);
}

PUBLIC
IMS_BOOL SipDialogEx::IsInviteUsage() const
{
    if (m_pDialogUsage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (m_pDialogUsage->GetType() == SipDialogUsage::TYPE_INVITE);
}

PUBLIC
void SipDialogEx::TerminateDialogUsage()
{
    IMS_TRACE_I("TerminateDialogUsage: %s",
            ((m_pDialogUsage != IMS_NULL) ? m_pDialogUsage->ToString().GetStr() : "(null)"), 0, 0);

    OnTerminated();
}

PUBLIC
IMS_SINT32 SipDialogEx::UpdateDialogDetails(IN const SipMessageInfo& objMsgInfo)
{
    if (m_pDialogUsage == IMS_NULL)
    {
        return SipPrivate::MESSAGE_FAILED;
    }

    return m_pDialogUsage->UpdateUsageDetails(objMsgInfo);
}

PUBLIC GLOBAL SipDialogEx* SipDialogEx::CreateDialog(IN const SipMethod& objMethod)
{
    SipDialogState* pDState = new SipDialogState();

    if (pDState == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "Instantiating a dialog state (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    SipDialogEx* pDialogEx = CreateDialog(pDState, objMethod);

    if (pDialogEx == IMS_NULL)
    {
        delete pDState;

        IMS_TRACE_E(0, "Instantiating a dialog (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    return pDialogEx;
}

PUBLIC GLOBAL SipDialogEx* SipDialogEx::CreateDialog(
        IN SipDialogState* pDState, IN const SipMethod& objMethod)
{
    SipDialogEx* pDialogEx = new SipDialogEx(pDState);

    if (pDialogEx == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a dialog (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (!pDialogEx->InitDialog(objMethod))
    {
        delete pDialogEx;
        return IMS_NULL;
    }

    return pDialogEx;
}

PUBLIC GLOBAL SipDialogEx* SipDialogEx::CreateDialog(
        IN SipDialogState* pDState, IN const SipMessageInfo& objMsgInfo)
{
    SipDialogEx* pDialogEx = new SipDialogEx(pDState);

    if (pDialogEx == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a dialog (%s) failed",
                objMsgInfo.GetMethod().ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (!pDialogEx->InitDialog(objMsgInfo))
    {
        delete pDialogEx;
        return IMS_NULL;
    }

    return pDialogEx;
}

PROTECTED VIRTUAL IMS_BOOL SipDialogEx::OnInit()
{
    if (m_bIsPermanentDialog == IMS_FALSE)
    {
        SipDialogState* pDState = GetDialogState();

        if (pDState == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!pDState->AddDialogUsage(this))
        {
            return IMS_FALSE;
        }

        m_bIsPermanentDialog = IMS_TRUE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void SipDialogEx::OnTerminated()
{
    if (m_bIsPermanentDialog == IMS_TRUE)
    {
        SipDialogState* pDState = GetDialogState();

        if (pDState != IMS_NULL)
        {
            pDState->RemoveDialogUsage(this);
        }

        m_bIsPermanentDialog = IMS_FALSE;
    }

    m_bIsDialogTerminated = IMS_TRUE;
}

PROTECTED VIRTUAL IMS_SINT32 SipDialogEx::OnUpdateDialogDetails(IN const SipMessageInfo& objMsgInfo,
        IN IMS_SINT32 nUsage, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger)
{
    IMS_SINT32 nUsageState = SipDState::STATE_MAX;

    switch (nUsage)
    {
        case SipDialogUsage::TYPE_INVITE:
            nUsageState = SipDialogInviteUsage::GetNextState(GetState(), nTrigger);
            break;

        case SipDialogUsage::TYPE_SUBSCRIBE:
            nUsageState = SipDialogSubscribeUsage::GetNextState(GetState(), nTrigger);
            break;

        default:
            break;
    }

    SipDialogState* pDState = GetDialogState();

    if (pDState == IMS_NULL)
    {
        return SipPrivate::MESSAGE_FAILED;
    }

    return pDState->UpdateDialogDetails(objMsgInfo, nUsageState, nAction, nTrigger);
}
