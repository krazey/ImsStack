#include "call/message/EmergencyMessageFormatter.h"
#include "call/IMtcSessionContext.h"
#include "FailReason.h"
#include "ISession.h"
#include "call/message/MessageSender.h"
#include "call/message/UssiMessageFormatter.h"

__IMS_TRACE_TAG_COM_MTC__;

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
MessageSender::MessageSender(IN IMtcSessionContext& objContext) :
        m_objContext(objContext),
        m_objSession(objContext.GetISession()),
        m_pFormatter(IMS_NULL)
{
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
MessageSender::~MessageSender() {}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::Start()
{
    CreateFormatter();

    if (m_pFormatter->FormStartMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.Start();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::SendProvisionalResponse(IN IMS_SINT32 eStatusCode, IN IMS_BOOL bReliable,
        IN IMS_BOOL bIncludeSdp, IN IMS_BOOL bIncludeAlertInfo)
{
    CreateFormatter();

    if (m_pFormatter->FormProvisionalResponseMessage(bIncludeAlertInfo) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    if (bReliable)
    {
        return m_objSession.SendRPR(eStatusCode, AString::ConstNull(), bIncludeSdp);
    }

    return m_objSession.SendProvisionalResponse(eStatusCode);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::SendPrack()
{
    if (m_pFormatter == IMS_NULL)
    {
        CreateFormatter();
    }

    if (m_pFormatter->FormPrackMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.SendPRAck();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::RespondToPrack(IN IMS_SINT32 eStatusCode)
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormPrackResponseMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.RespondToPRAck(eStatusCode);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::SendEarlyUpdate(IN UpdateType eUpdateType)
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormEarlyUpdateMessage(eUpdateType) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.UpdateEarlyMedia();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode)
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormEarlyUpdateResponseMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.RespondToEarlyUpdate(eStatusCode);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::Accept()
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormAcceptMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.Accept();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::Reject(IN const FailReason& objReason)
{
    CreateFormatter();

    IMS_SINT32 eStatusCode;
    AString strPhrase;

    if (m_pFormatter->FormRejectMessage(objReason, eStatusCode, strPhrase) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.RejectEx(eStatusCode, strPhrase);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::SendAck()
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormAckMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.SendAck();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::Update(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo,
        IN IMS_SINT32 eMethod /*= SipMethod::INVITE*/, IN IMS_BOOL bSessionRefresh /*= IMS_FALSE*/)
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormUpdateMessage(eUpdateType, bIncludeAlertInfo) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.UpdateEx(eMethod, bSessionRefresh);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::AcceptUpdate()
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormAcceptUpdateMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.Accept();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::CancelUpdate(IN const FailReason& objReason)
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormCancelUpdateMessage(objReason) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.TerminateEx(IMS_FALSE);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::Terminate(IN IMS_BOOL bUseBye, IN const FailReason& objReason)
{
    if (m_pFormatter == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pFormatter->FormTerminateMessage(objReason) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    m_objSession.SetReasonForCallTermination(objReason.nReason);
    return m_objSession.TerminateEx(bUseBye);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageSender::CreateFormatter()
{
    if (m_pFormatter != IMS_NULL)
    {
        return;
    }

    IMS_BOOL bEmergency = IMS_FALSE;  // TODO
    if (bEmergency)
    {
        m_pFormatter = new EmergencyMessageFormatter(m_objContext);
        return;
    }

    IMS_BOOL bUssi = IMS_FALSE;  // TODO
    if (bUssi)
    {
        m_pFormatter = new UssiMessageFormatter(m_objContext);
        return;
    }

    m_pFormatter = new MessageFormatter(m_objContext);
}
