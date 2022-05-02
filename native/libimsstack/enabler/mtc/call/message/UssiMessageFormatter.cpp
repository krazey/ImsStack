#include "ISipHeader.h"
#include "call/message/UssiMessageFormatter.h"
#include "ussi/UssiConstants.h"
#include "utility/MessageUtil.h"
#include "SipHeaderName.h"

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
UssiMessageFormatter::UssiMessageFormatter(IN IMtcSessionContext& objContext) :
        MessageFormatter(objContext)
{
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL
UssiMessageFormatter::~UssiMessageFormatter()
{
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL
IMS_RESULT UssiMessageFormatter::FormStartMessage()
{
    if (MessageFormatter::FormStartMessage() == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetRecvInfoHeader();
    SetAcceptHeader();
    SetContentTypeHeader();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL
IMS_RESULT UssiMessageFormatter::FormAcceptMessage()
{
    if (MessageFormatter::FormAcceptMessage() == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetRecvInfoHeader();
    SetAcceptHeader();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void UssiMessageFormatter::SetRecvInfoHeader()
{
    MessageUtil::AddValueIfNotExists(m_piNextMessage, USSDConstants::HEADER_USSD_PACKAGE,
            ISIPHeader::UNKNOWN, SIPHeaderName::RECV_INFO);

    return;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void UssiMessageFormatter::SetAcceptHeader()
{
    MessageUtil::AddValueIfNotExists(m_piNextMessage, USSDConstants::HEADER_APPLICATION_SDP,
            ISIPHeader::ACCEPT);
    MessageUtil::AddValueIfNotExists(m_piNextMessage, USSDConstants::HEADER_APPLICATION_IMSXML,
            ISIPHeader::ACCEPT);
    MessageUtil::AddValueIfNotExists(m_piNextMessage, USSDConstants::HEADER_APPLICATION_USSDXML,
            ISIPHeader::ACCEPT);
    MessageUtil::AddValueIfNotExists(m_piNextMessage, USSDConstants::HEADER_MULTIPART_MIXED,
            ISIPHeader::ACCEPT);

    return;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void UssiMessageFormatter::SetContentTypeHeader()
{
    MessageUtil::AddValueIfNotExists(m_piNextMessage, USSDConstants::HEADER_MULTIPART_MIXED,
            ISIPHeader::CONTENT_TYPE);

    return;
}
