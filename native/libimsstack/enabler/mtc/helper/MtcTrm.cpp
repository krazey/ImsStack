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

#include "ServiceEvent.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "ServicePhoneInfo.h"

#include "helper/ICallStateProxy.h"
#include "helper/MtcTrm.h"
#include "IuMtcService.h"

__IMS_TRACE_TAG_COM_UC__;

/* ------------------------------------------------------------------------------------------------
    Constructor, Destructor
------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
UCTRM::UCTRM() :
        ImsActivityEx(AString::ConstNull())
{
    IMS_TRACE_MEM("uc", "uc_M : UCTRM[%" PFLS_u "][%" PFLS_x "]", sizeof(UCTRM), this, 0);

    m_pIMutex = MutexService::GetMutexService()->CreateMutex();

    m_lstCT_Handler = IMSList<CT_Handler*>();
    m_lstTRM = IMSList<TRM*>();
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL UCTRM::~UCTRM()
{
    IMS_TRACE_MEM("uc", "uc_F : UCTRM[%" PFLS_u "][%" PFLS_x "]", sizeof(UCTRM), this, 0);

    for (IMS_UINT32 index = 0; index < m_lstCT_Handler.GetSize(); index++)
    {
        CT_Handler* pHandler = m_lstCT_Handler.GetAt(index);
        delete pHandler;
        pHandler = IMS_NULL;
    }
    m_lstCT_Handler.Clear();

    for (IMS_UINT32 index = 0; index < m_lstTRM.GetSize(); index++)
    {
        TRM* pTRM = m_lstTRM.GetAt(index);
        delete pTRM;
        pTRM = IMS_NULL;
    }
    m_lstTRM.Clear();

    MutexService::GetMutexService()->DestroyMutex(m_pIMutex);
    m_pIMutex = IMS_NULL;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC GLOBAL UCTRM* UCTRM::GetInstance()
{
    static UCTRM* s_pUCTRM = IMS_NULL;

    if (s_pUCTRM == IMS_NULL)
    {
        s_pUCTRM = new UCTRM();
    }

    return s_pUCTRM;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCTRM::Init(IN IMS_SINT32 nSlotID)
{
    LockGuard objLock(m_pIMutex);

    IMS_TRACE_D("Init : Slot[%d]", nSlotID, 0, 0);

    ITrm* pITRM = PhoneInfoService::GetPhoneInfoService()->GetTrm();
    if (pITRM == IMS_NULL || !(pITRM->IsTrmSupported()))
    {
        return;
    }

    openCT_Handler(nSlotID);
    openTRM(nSlotID);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCTRM::DeInit(IN IMS_SINT32 nSlotID)
{
    LockGuard objLock(m_pIMutex);

    IMS_TRACE_D("DeInit : Slot[%d]", nSlotID, 0, 0);

    closeTRM(nSlotID);
    closeCT_Handler(nSlotID);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_BOOL UCTRM::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnMessage[%d]", objMSG.nMSG, 0, 0);

    switch (objMSG.nMSG)
    {
        default:
            break;
    }

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCTRM::SetTRM(IN IMS_SINT32 nSlotID, IN IMS_UINT32 eType, IN IMS_BOOL bSet)
{
    TRM* pTRM = IMS_NULL;
    IMS_BOOL bSetTRM = IMS_FALSE;
    IMS_BOOL bOnCall = IMS_FALSE;

    pTRM = getTRM(nSlotID);
    if (pTRM == IMS_NULL)
    {
        IMS_TRACE_D("SetTRM : Invalid TRM", 0, 0, 0);
        return;
    }

    // TODO, MTC BUILD
    // if (eType == TRM_TYPE_NORMAL_CALL)
    // {
    //     bOnCall = CallStateProxy::GetInstance()->IsNormalCall(nSlotID);
    // }
    // else if (eType == TRM_TYPE_EMERGENCY_CALL)
    // {
    //     bOnCall = CallStateProxy::GetInstance()->IsEmergencyCall(nSlotID);
    // }
    // else
    // {
    //     bOnCall = CallStateProxy::GetInstance()->IsNormalCall(nSlotID);
    // }

    if (bOnCall && !bSet)
    {
        IMS_TRACE_D("SetTRM : OnCall - Skip Off", 0, 0, 0);
        return;
    }

    if (eType == TRM_TYPE_NORMAL_CALL)
    {
        bSetTRM = pTRM->SetNTRM(bSet);
    }
    else if (eType == TRM_TYPE_EMERGENCY_CALL)
    {
        bSetTRM = pTRM->SetETRM(bSet);
    }
    else
    {
        bSetTRM = pTRM->SetNTRM(bSet);
    }

    IMS_TRACE_D("SetTRM :  slotID[%d] [%d] [%s]", nSlotID, eType, PS_BOOL(bSetTRM));
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_BOOL UCTRM::IsTRM(IN IMS_SINT32 nSlotID, IN IMS_UINT32 eType)
{
    TRM* pTRM = IMS_NULL;
    IMS_BOOL bTRM = IMS_FALSE;

    pTRM = getTRM(nSlotID);
    if (pTRM == IMS_NULL)
    {
        IMS_TRACE_D("IsTRM : Invalid TRM", 0, 0, 0);
        return bTRM;
    }

    if (eType == TRM_TYPE_NORMAL_CALL)
    {
        bTRM = pTRM->m_bNSet;
    }
    else if (eType == TRM_TYPE_EMERGENCY_CALL)
    {
        bTRM = pTRM->m_bESet;
    }
    else
    {
        bTRM = pTRM->m_bNSet;
    }

    IMS_TRACE_D("IsTRM :  slotID[%d] [%d] [%s]", nSlotID, eType, PS_BOOL(bTRM));
    return bTRM;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::openCT_Handler(IN IMS_SINT32 nSlotID)
{
    CT_Handler* pHandler = new CT_Handler(nSlotID);
    m_lstCT_Handler.Append(pHandler);

    IMS_TRACE_D("openCT_Handler : slotID[%d] Size[%d]", nSlotID, m_lstCT_Handler.GetSize(), 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::closeCT_Handler(IN IMS_SINT32 nSlotID)
{
    IMS_UINT32 size = m_lstCT_Handler.GetSize();
    IMS_UINT32 index = 0;

    if (size <= 0)
    {
        IMS_TRACE_I("closeCT_Handler : slotID[%d] size[%d]", nSlotID, size, 0);
        return;
    }

    for (index = 0; index < size; index++)
    {
        CT_Handler* pHandler = m_lstCT_Handler.GetAt(index);

        if (pHandler->m_nSlotID == nSlotID)
        {
            delete pHandler;
            pHandler = IMS_NULL;
            m_lstCT_Handler.RemoveAt(index);
            break;
        }
    }

    IMS_TRACE_I("closeCT_Handler : slotID[%d] size[%d] index[%d]", nSlotID,
            m_lstCT_Handler.GetSize(), index);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
UCTRM::CT_Handler* UCTRM::getCT_Handler(IN IMS_SINT32 nSlotID)
{
    IMS_UINT32 size = m_lstCT_Handler.GetSize();

    if (size <= 0)
    {
        IMS_TRACE_D("getCT_Handler : slotID[%d] size[%d]", nSlotID, size, 0);
        return IMS_NULL;
    }

    for (IMS_UINT32 index = 0; index < size; index++)
    {
        CT_Handler* pHandler = m_lstCT_Handler.GetAt(index);

        if (pHandler->m_nSlotID == nSlotID)
        {
            return pHandler;
        }
    }

    IMS_TRACE_D("getCT_Handler : slotID[%d] size[%d] - Not Found ", nSlotID, size, 0);
    return IMS_NULL;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::openTRM(IN IMS_SINT32 nSlotID)
{
    TRM* pTRM = new TRM(nSlotID);
    m_lstTRM.Append(pTRM);

    IMS_TRACE_D("openTRM : slotID[%d] Size[%d]", nSlotID, m_lstTRM.GetSize(), 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::closeTRM(IN IMS_SINT32 nSlotID)
{
    IMS_UINT32 size = m_lstTRM.GetSize();
    IMS_UINT32 index = 0;

    if (size <= 0)
    {
        IMS_TRACE_I("closeTRM : slotID[%d] size[%d]", nSlotID, size, 0);
        return;
    }

    for (index = 0; index < size; index++)
    {
        TRM* pTRM = m_lstTRM.GetAt(index);

        if (pTRM->m_nSlotID == nSlotID)
        {
            delete pTRM;
            pTRM = IMS_NULL;
            m_lstTRM.RemoveAt(index);
            break;
        }
    }

    IMS_TRACE_I("closeTRM : slotID[%d] size[%d] index[%d]", nSlotID, m_lstTRM.GetSize(), index);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
UCTRM::TRM* UCTRM::getTRM(IN IMS_SINT32 nSlotID)
{
    IMS_UINT32 size = m_lstTRM.GetSize();

    if (size <= 0)
    {
        IMS_TRACE_D("getTRM : slotID[%d] size[%d]", nSlotID, size, 0);
        return IMS_NULL;
    }

    for (IMS_UINT32 index = 0; index < size; index++)
    {
        TRM* pTRM = m_lstTRM.GetAt(index);

        if (pTRM->m_nSlotID == nSlotID)
        {
            return pTRM;
        }
    }

    IMS_TRACE_D("getTRM : slotID[%d] size[%d] - Not Found ", nSlotID, size, 0);
    return IMS_NULL;
}

/* ------------------------------------------------------------------------------------------------
    SUBCLASS
------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------
    Constructor, Destructor
------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
UCTRM::CT_Handler::CT_Handler(IN IMS_SINT32 nSlotID)
{
    IMS_TRACE_MEM("uc", "uc_M : CT_Handler[%" PFLS_u "][%" PFLS_x "]", sizeof(CT_Handler), this, 0);

    m_nSlotID = nSlotID;

    // TODO, MTC BUILD
    // CallStateProxy::GetInstance()->AddListener(m_nSlotID, this);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL UCTRM::CT_Handler::~CT_Handler()
{
    IMS_TRACE_MEM("uc", "uc_F : CT_Handler[%" PFLS_u "][%" PFLS_x "]", sizeof(CT_Handler), this, 0);

    // TODO, MTC BUILD
    // CallStateProxy::GetInstance()->RemoveListener(m_nSlotID, this);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_BOOL UCTRM::CT_Handler::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnMessage[%d]", objMSG.nMSG, 0, 0);

    switch (objMSG.nMSG)
    {
        case TRM_CHANGED_STATE:
            handleCallChangedState(objMSG);
            break;
        case TRM_CHANGED_TOTALSTATE:
            handleCallChangedTotalState(objMSG);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallChangedState(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallChangedTotalState(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallStateIdle(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallStateOffhook(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallStateRingback(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallStateRinging(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallStateAlerting(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallStateTerminating(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallTotalStateIdle(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallTotalStateOffhook(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallTotalStateRingback(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallTotalStateRinging(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallTotalStateAlerting(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE
void UCTRM::CT_Handler::handleCallTotalStateTerminating(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
UCTRM::TRM::TRM(IN IMS_SINT32 nSlotID)
{
    IMS_TRACE_MEM("uc", "uc_M : TRM[%" PFLS_u "][%" PFLS_x "]", sizeof(TRM), this, 0);

    m_nSlotID = nSlotID;
    m_bNSet = IMS_FALSE;
    m_bESet = IMS_FALSE;

    m_pITRM = PhoneInfoService::GetPhoneInfoService()->GetTrm();
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL UCTRM::TRM::~TRM()
{
    IMS_TRACE_MEM("uc", "uc_F : TRM[%" PFLS_u "][%" PFLS_x "]", sizeof(TRM), this, 0);

    if (m_bNSet)
    {
        m_pITRM->SetService(m_nSlotID, ITrm::SERVICE_VOLTE, ITrm::MODE_END);
    }

    if (m_bESet)
    {
        m_pITRM->SetEmergencyService(m_nSlotID, ITrm::SERVICE_VOLTE, ITrm::MODE_END);
    }

    m_pITRM = IMS_NULL;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_BOOL UCTRM::TRM::SetNTRM(IN IMS_BOOL bSet)
{
    IMS_TRACE_I("SetNTRM : [%s][%s]", PS_BOOL(m_bNSet), PS_BOOL(bSet), 0);

    if (m_bNSet == bSet)
    {
        return IMS_FALSE;
    }

    m_pITRM->SetService(m_nSlotID, ITrm::SERVICE_VOLTE, (bSet) ? ITrm::MODE_START : ITrm::MODE_END);

    m_bNSet = bSet;
    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_BOOL UCTRM::TRM::SetETRM(IN IMS_BOOL bSet)
{
    IMS_TRACE_I("SetETRM: [%s][%s]", PS_BOOL(m_bESet), PS_BOOL(bSet), 0);

    if (m_bESet == bSet)
    {
        return IMS_FALSE;
    }

    m_pITRM->SetEmergencyService(
            m_nSlotID, ITrm::SERVICE_VOLTE, (bSet) ? ITrm::MODE_START : ITrm::MODE_END);

    m_bESet = bSet;
    return IMS_TRUE;
}
