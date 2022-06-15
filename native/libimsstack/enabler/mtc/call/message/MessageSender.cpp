#include "call/message/EmergencyMessageFormatter.h"
#include "call/IMtcSessionContext.h"
#include "FailReason.h"
#include "ISession.h"
#include "call/message/MessageSender.h"

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
    if (GetFormatter().FormStartMessage() != IMS_SUCCESS)
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
    if (GetFormatter().FormProvisionalResponseMessage(bIncludeAlertInfo) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    if (bReliable)
    {
        return m_objSession.SendRpr(eStatusCode, AString::ConstNull(), bIncludeSdp);
    }

    return m_objSession.SendProvisionalResponse(eStatusCode);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::SendPrack()
{
    if (GetFormatter().FormPrackMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.SendPrack();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::RespondToPrack(IN IMS_SINT32 eStatusCode)
{
    if (GetFormatter().FormPrackResponseMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.RespondToPrack(eStatusCode);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
IMS_RESULT MessageSender::SendEarlyUpdate(IN UpdateType eUpdateType)
{
    if (GetFormatter().FormEarlyUpdateMessage(eUpdateType) != IMS_SUCCESS)
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
    if (GetFormatter().FormEarlyUpdateResponseMessage() != IMS_SUCCESS)
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
    if (GetFormatter().FormAcceptMessage() != IMS_SUCCESS)
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
    IMS_SINT32 eStatusCode;
    AString strPhrase;

    if (GetFormatter().FormRejectMessage(objReason, eStatusCode, strPhrase) != IMS_SUCCESS)
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
    if (GetFormatter().FormAckMessage() != IMS_SUCCESS)
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
    if (GetFormatter().FormUpdateMessage(eUpdateType, bIncludeAlertInfo) != IMS_SUCCESS)
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
    if (GetFormatter().FormAcceptUpdateMessage() != IMS_SUCCESS)
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
    if (GetFormatter().FormCancelUpdateMessage(objReason) != IMS_SUCCESS)
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
    if (GetFormatter().FormTerminateMessage(objReason) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    m_objSession.SetReasonForCallTermination(objReason.nReason);
    return m_objSession.TerminateEx(bUseBye);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
MessageFormatter& MessageSender::GetFormatter()
{
    if (m_pFormatter != IMS_NULL)
    {
        return *m_pFormatter;
    }

    /* TODO:
    IMS_BOOL bEmergency = IMS_FALSE;
    if (bEmergency)
    {
        m_pFormatter = new EmergencyMessageFormatter(m_objContext);
        return;
    }
    */

    m_pFormatter = new MessageFormatter(m_objContext);
    return *m_pFormatter;
}
