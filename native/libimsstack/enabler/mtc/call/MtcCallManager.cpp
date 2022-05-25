#include "call/IMtcCall.h"
#include "call/MtcCall.h"
#include "call/MtcCallManager.h"
#include "call/MtcSession.h"
#include "call/NullCall.h"
#include "helper/CallStateProxy.h"
#include "interface/mtc/IMtcCallStateListener.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTC__;

NullCall* const MtcCallManager::s_pNullCall = new NullCall();

PUBLIC
MtcCallManager::MtcCallManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_lstCalls(IMSList<MtcCall*>())
{
}

PUBLIC VIRTUAL MtcCallManager::~MtcCallManager()
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_lstCalls.GetSize(); nIndex++)
    {
        delete m_lstCalls.GetAt(nIndex);
    }
    m_lstCalls.Clear();

    m_objContext.GetCallStateProxy().RemoveListener(this);
}

PUBLIC
void MtcCallManager::Init()
{
    m_objContext.GetCallStateProxy().AddListener(this);
}

PUBLIC VIRTUAL IMtcCall* MtcCallManager::CreateCall(
        IN ServiceType eServiceType, IN CallInfo& objCallInfo)
{
    IMtcService* pService = m_objContext.GetServiceByType(eServiceType);
    if (pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCall : Service is null - type[%" PFLS_d "]",
                static_cast<IMS_SINT32>(eServiceType), 0, 0);
        return s_pNullCall;
    }

    MtcCall* pCall = new MtcCall(m_objContext, *pService, objCallInfo);
    m_lstCalls.Append(pCall);
    IMS_TRACE_D("CreateCall : call count[%d]", m_lstCalls.GetSize(), 0, 0);

    return pCall;
}

PUBLIC VIRTUAL void MtcCallManager::RemoveCall(IN CallKey nCallKey)
{
    IMS_SINT32 nIndex = GetFirstIndexByFilter(
            [nCallKey](MtcCall* pCall)
            {
                return pCall->GetKey() == nCallKey;
            });

    if (nIndex >= 0)
    {
        MtcCall* pCall = m_lstCalls.GetAt(nIndex);
        delete pCall;
        m_lstCalls.RemoveAt(nIndex);
    }

    IMS_TRACE_D("RemoveCall : call count[%d]", m_lstCalls.GetSize(), 0, 0);
}

PUBLIC VIRTUAL IMtcCall* MtcCallManager::GetCallByCallKey(IN CallKey nCallKey)
{
    IMS_SINT32 nIndex = GetFirstIndexByFilter(
            [nCallKey](MtcCall* pCall)
            {
                return pCall->GetKey() == nCallKey;
            });

    IMS_TRACE_D("GetCallByCallKey index[%d]", nIndex, 0, 0);
    if (nIndex >= 0)
    {
        return m_lstCalls.GetAt(nIndex);
    }
    else
    {
        return s_pNullCall;
    }
}

PUBLIC VIRTUAL IMSList<IMtcCall*> MtcCallManager::GetCalls()
{
    return GetCallsByFilter(
            [](MtcCall* /* pCall */)
            {
                return IMS_TRUE;
            });
}

PUBLIC VIRTUAL IMSList<IMtcCall*> MtcCallManager::GetCallsByType(IN CallType eCallType)
{
    return GetCallsByFilter(
            [eCallType](MtcCall* pCall)
            {
                return pCall->GetSession()->GetCallType() == eCallType;
            });
}

PUBLIC VIRTUAL IMSList<IMtcCall*> MtcCallManager::GetCallsByServiceType(IN ServiceType eServiceType)
{
    return GetCallsByFilter(
            [eServiceType](MtcCall* pCall)
            {
                return pCall->GetService().GetServiceType() == eServiceType;
            });
}

PUBLIC VIRTUAL IMSList<IMtcCall*> MtcCallManager::GetCallsInConference()
{
    return GetCallsByFilter(
            [](MtcCall* pCall)
            {
                return pCall->GetCallInfo().bConference;
            });
}

PUBLIC VIRTUAL void MtcCallManager::OnCallStateChanged(IN CallKey nCallKey, IN State eState,
        IN Type /* eType */, IN IMS_BOOL /* bEmergency */, IN IMS_SINT32 /* nReason */)
{
    IMS_TRACE_D(
            "OnCallStateChanged : key[%d] state[%d]", nCallKey, static_cast<IMS_SINT32>(eState), 0);

    if (eState == State::TERMINATING)
    {
        RemoveCall(nCallKey);
    }
}

PUBLIC VIRTUAL void MtcCallManager::OnTotalCallStateChanged(IN State /* eState */) {}

PRIVATE
IMS_SINT32 MtcCallManager::GetFirstIndexByFilter(IN std::function<IMS_BOOL(MtcCall*)> objFilter)
{
    // Call index mustn't be outside of IMS_SINT32 range.
    for (IMS_UINT32 nIndex = 0; nIndex < m_lstCalls.GetSize(); nIndex++)
    {
        MtcCall* pCall = m_lstCalls.GetAt(nIndex);

        if (pCall != IMS_NULL && objFilter(pCall))
        {
            return nIndex;
        }
    }

    return -1;
}

PRIVATE
IMSList<IMtcCall*> MtcCallManager::GetCallsByFilter(IN std::function<IMS_BOOL(MtcCall*)> objFilter)
{
    IMSList<IMtcCall*> lstResult;

    for (IMS_UINT32 nIndex = 0; nIndex < m_lstCalls.GetSize(); nIndex++)
    {
        MtcCall* pCall = m_lstCalls.GetAt(nIndex);

        if (pCall != IMS_NULL && objFilter(pCall))
        {
            lstResult.Append(pCall);
        }
    }

    return lstResult;
}
