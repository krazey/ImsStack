#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ImsAos.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "ImsAosParameter.h"
#include "IMtsClient.h"
#include "utility/MtsDynamicLoader.h"
#include "MtsServiceState.h"
#include "message/MtsMessageController.h"
#include "utility/MtsStrName.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsServiceState::MtsServiceState(IN IMS_SINT32 nSlotId) :
        m_nMtsServiceState(IMtsClient::STATE_INIT),
        m_bIsImsConnected(IMS_FALSE),
        m_bIsAosRegModAdmin(IMS_FALSE),
        m_bIsImsSuspend(IMS_FALSE),
        m_bIsSmsOverIpConf(IMS_FALSE),
        m_bIsTemporaryBlocked(IMS_FALSE),
        m_nConnectedServices(ImsAosService::NONE),
        m_nSlotId(nSlotId),
        m_pMtsMessageController(IMS_NULL)
{
    IMS_TRACE_I("+MtsServiceState", 0, 0, 0);
}

PUBLIC
MtsServiceState::~MtsServiceState()
{
    IMS_TRACE_I("~MtsServiceState", 0, 0, 0);
}

PUBLIC
MtsServiceState* MtsServiceState::GetInstance(IN IMS_SINT32 nSlotId)
{
    static MtsServiceState* s_pMtsServiceState = IMS_NULL;

    if (s_pMtsServiceState == IMS_NULL)
    {
        s_pMtsServiceState = new MtsServiceState(nSlotId);
    }

    return s_pMtsServiceState;
}

PUBLIC
void MtsServiceState::SetImsRegConnected(IN IMS_BOOL bConnected)
{
    IMS_UINT32 nType = IImsAosInfo::REG_MODE_UNKNOWN;
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController == IMS_NULL)
    {
        IMS_TRACE_E(0, "pMtsMessageController is null", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("MtsServiceState::SetImsRegConnected() m_bIsImsConnected(%s)/bConnected(%s)",
            _TRACE_B_(m_bIsImsConnected), _TRACE_B_(bConnected), 0);

    if (m_bIsImsConnected == bConnected)
    {
        return;
    }

    m_bIsImsConnected = bConnected;

    if (m_bIsImsConnected)
    {
        IImsAos* piImsAos =
                ImsAos::GetImsAos(AString("ims.app.mts"), AString("ims.service.mts"), m_nSlotId);

        if (piImsAos == IMS_NULL)
        {
            IMS_TRACE_E(0, "MtsServiceState:: Fail to get AoSApp", 0, 0, 0);
            return;
        }

        nType = piImsAos->GetAosInfo()->GetRegistrationMode();

        if (IImsAosInfo::REG_MODE_ADMIN == nType)
        {
            m_bIsAosRegModAdmin = IMS_TRUE;
        }
        else
        {
            m_bIsAosRegModAdmin = IMS_FALSE;

            if (IImsAosInfo::REG_MODE_UNKNOWN == nType)
            {
                IMS_TRACE_I("MtsServiceState:: IMS Reg Mod is UNKNOWN!!", 0, 0, 0);
            }
        }
    }

    IMS_TRACE_I("MtsServiceState:: IMS Reg State (%s), IMS Admin Reg (%s)",
            _TRACE_B_(m_bIsImsConnected), _TRACE_B_(m_bIsAosRegModAdmin), 0);

    UpdateServiceState();
}

// TODO: consider of utilizing this method for VZW E911 SMS case
PUBLIC
void MtsServiceState::SetImsRegConnected(IN IMS_BOOL /*bConnected*/, IMS_BOOL /*bIsEmergencyType*/)
{
    IMS_TRACE_I("MtsServiceState::SetImsRegConnected() For Only VZW", 0, 0, 0);
}

PUBLIC
void MtsServiceState::SetImsSuspendState(IN IMS_BOOL bState)
{
    if (m_bIsImsSuspend == bState)
    {
        return;
    }

    m_bIsImsSuspend = bState;  // if IMSAoSApp_OnImsSuspended. block mo service

    IMS_TRACE_I("MtsServiceState:: IMS Suspend State is (%s)", _TRACE_B_(m_bIsImsSuspend), 0, 0);

    UpdateServiceState();
}

PUBLIC
void MtsServiceState::SetSmsOverIpState(IN IMS_BOOL bState)
{
    if (m_bIsSmsOverIpConf == bState)
    {
        return;
    }

    m_bIsSmsOverIpConf = bState;

    IMS_TRACE_I("MtsServiceState:: Sms Over IP Network State is (%s)",
            _TRACE_B_(m_bIsSmsOverIpConf), 0, 0);

    UpdateServiceState();
}

PUBLIC
void MtsServiceState::SetTemporaryServiceBlocked(IN IMS_BOOL bBlocked)
{
    m_bIsTemporaryBlocked = bBlocked;

    IMS_TRACE_I("MtsServiceState:: Service Blocked State is (%s)", _TRACE_B_(m_bIsTemporaryBlocked),
            0, 0);
}

PUBLIC
void MtsServiceState::SetMtsMessageController(IN MtsMessageController* pMtsMessageController)
{
    m_pMtsMessageController = pMtsMessageController;
}

PUBLIC
IMS_SINT32 MtsServiceState::GetSlotId()
{
    IMS_TRACE_D("GetSlotId[%d]", m_nSlotId, 0, 0);
    return m_nSlotId;
}

PUBLIC
IMS_BOOL MtsServiceState::GetImsRegState()
{
    return m_bIsImsConnected;
}

PUBLIC
IMS_BOOL MtsServiceState::GetImsRegMod()
{
    return m_bIsAosRegModAdmin;
}

PUBLIC
IMS_BOOL MtsServiceState::GetImsSuspendState()
{
    return m_bIsImsSuspend;
}

PUBLIC
IMS_BOOL MtsServiceState::GetSmsOverIpState()
{
    return m_bIsSmsOverIpConf;
}

PUBLIC
IMS_SINT32 MtsServiceState::GetMtsServiceState()
{
    return m_nMtsServiceState;
}

PUBLIC
void MtsServiceState::SetMtsServiceState(IN IMS_SINT32 nServiceState)
{
    m_nMtsServiceState = nServiceState;
}

PUBLIC
IMS_UINT32 MtsServiceState::GetConnectedServices()
{
    return m_nConnectedServices;
}

PUBLIC
MtsMessageController* MtsServiceState::GetMtsMessageController()
{
    return m_pMtsMessageController;
}

PUBLIC
IMS_BOOL MtsServiceState::IsServiceConnected(IN IMS_UINT32 nService)
{
    return (m_nConnectedServices & nService) != 0;
}

PUBLIC
void MtsServiceState::SetConnectedServices(IN IMS_UINT32 nServices)
{
    m_nConnectedServices = nServices;
}

PUBLIC
void MtsServiceState::OnImsConnected()
{
    IMS_TRACE_I("MtsServiceState::OnImsConnected()", 0, 0, 0);

    SetImsRegConnected(IMS_TRUE);
}

// TODO: consider of utilizing this method for VZW E911 SMS case
PUBLIC
void MtsServiceState::OnImsConnected(IN IUSendSmsRequestParam* /*pToBeSentSms*/)
{
    IMS_TRACE_I("MtsServiceState::OnImsConnected() For Only VZW", 0, 0, 0);
}

PUBLIC
void MtsServiceState::OnImsDisconnected(IN IMS_UINT32 nReason)
{
    MtsMessageController* pMtsMessageController = IMS_NULL;

    IMS_TRACE_I("MtsServiceState::OnImsDisconnected() Reason is (%d)", nReason, 0, 0);

    SetImsSuspendState(IMS_FALSE);
    SetImsRegConnected(IMS_FALSE);

    // if ims data connection is disconnected, terminate all pending messages.
    pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController != IMS_NULL)
    {
        pMtsMessageController->TerminateAllPendingMessages(IMS_FALSE);
    }
}

PUBLIC
void MtsServiceState::OnImsDisconnecting(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("MtsServiceState::OnImsDisconnecting() Reason is (%d)", nReason, 0, 0);
}

PUBLIC
void MtsServiceState::OnImsSuspended(IN IMS_UINT32 nReason)
{
    MtsMessageController* pMtsMessageController = IMS_NULL;

    IMS_TRACE_I("MtsServiceState::OnImsSuspended() Reason is (%d)", nReason, 0, 0);
    SetImsSuspendState(IMS_TRUE);

    IMS_TRACE_I("Mts transaction permanent failure", 0, 0, 0);
    pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController != IMS_NULL)
    {
        pMtsMessageController->TerminateAllPendingMessages(IMS_TRUE);
    }
}

PUBLIC
void MtsServiceState::OnImsResumed()
{
    IMS_TRACE_I("MtsServiceState::OnImsResumed()", 0, 0, 0);

    SetImsSuspendState(IMS_FALSE);
}

PUBLIC
void MtsServiceState::NotifySpecificMessage(
        IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam)
{
    (void)nMsg;
    (void)nWparam;
    (void)nLparam;

    IMS_TRACE_I("MtsServiceState::NotifySpecificMessage()", 0, 0, 0);
}

PUBLIC
IMS_SINT32 MtsServiceState::GetServiceState()
{
    IMS_SINT32 nState = IMtsClient::STATE_NOTREADY;

    if (m_bIsImsConnected)
    {
        if (m_bIsImsSuspend || (!m_bIsSmsOverIpConf) || m_bIsAosRegModAdmin)
        {
            nState = IMtsClient::STATE_LIMITED;
        }
        else
        {
            nState = IMtsClient::STATE_READY;
        }
    }

    return nState;
}

PUBLIC
void MtsServiceState::UpdateServiceState()
{
    IMS_SINT32 nTempState = GetServiceState();

    IMS_TRACE_I("nTempState(%d) m_nMtsServiceState(%d)", nTempState, m_nMtsServiceState, 0);

    if (nTempState != m_nMtsServiceState)
    {
        m_nMtsServiceState = nTempState;
    }
}

PUBLIC
IMS_BOOL MtsServiceState::IsMoServiceBlocked()
{
    return (GetMtsServiceState() != IMtsClient::STATE_READY);
}

PUBLIC
IMS_BOOL MtsServiceState::IsMtServiceBlocked()
{
    return (GetMtsServiceState() == IMtsClient::STATE_NOTREADY);
}

PUBLIC
IMS_BOOL MtsServiceState::IsTemporaryServiceBlocked()
{
    return m_bIsTemporaryBlocked;
}
