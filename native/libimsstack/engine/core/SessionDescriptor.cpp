/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090622  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipDebug.h"
#include "base/IMS.h"
#include "ISessionState.h"
#include "offeranswer/SdpSessionParameter.h"
#include "SessionDescriptor.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL const IMS_CHAR*
SessionDescriptor::RESERVED_ATTRIBUTE[SessionDescriptor::MAX_RESERVED_ATTRIBUTE] =
{
    "charset",
    "charset:ISO8895-1",  // ??? It needs confirmation
    "group",
    "maxprate",
    "ice-lite",
    "ice-mismatch",
    "ice-options",
    "ice-pwd",
    "ice-ufrag",
    "inactive",
    "sendonly",
    "recvonly",
    "sendrecv",
    "csup",
    "creq",
    "acap",
    "tcap",
};

PUBLIC
SessionDescriptor::SessionDescriptor(IN ISessionState* piSessionState_) :
        piSessionState(piSessionState_)
{
}

PUBLIC VIRTUAL SessionDescriptor::~SessionDescriptor() {}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::AddAttribute(IN CONST AString& strAttribute)
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.Decode(strAttribute))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check if it is a reserved or not
    for (IMS_SINT32 i = 0; i < MAX_RESERVED_ATTRIBUTE; ++i)
    {
        if (objAttribute.GetAttributeEx().Equals(RESERVED_ATTRIBUTE[i]))
        {
            IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Reserved attribute (%s) can't be added", strAttribute.GetStr(), 0, 0);
            return IMS_FAILURE;
        }
    }

    // Check if it already exists in the session
    if (pSessionParam->Contains(objAttribute))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!pSessionParam->AddAttribute(objAttribute))
    {
        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMSList<AString> SessionDescriptor::GetAttributes() const
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMSList<AString>();
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMSList<AString>();
    }

    const IMSList<SdpAttribute>& objSDPAttributes = pSessionParam->GetAttributes();
    IMSList<AString> objAttributes;

    for (IMS_UINT32 i = 0; i < objSDPAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = objSDPAttributes.GetAt(i);

        objAttributes.Append(objAttribute.GetValue());
    }

    return objAttributes;
}

PRIVATE VIRTUAL AString SessionDescriptor::GetProtocolVersion() const
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetVersion().GetValue();
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetSessionId() const
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetOrigin().GetSessionId();
}

PRIVATE VIRTUAL AString SessionDescriptor::GetSessionInfo() const
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    const SdpInformation* pInformation = pSessionParam->GetInformation();

    if (pInformation == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pInformation->GetValue();
}

PRIVATE VIRTUAL AString SessionDescriptor::GetSessionName() const
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetSessionName().GetValue();
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::RemoveAttribute(IN CONST AString& strAttribute)
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.Decode(strAttribute))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check if it is a reserved or not
    for (IMS_SINT32 i = 0; i < MAX_RESERVED_ATTRIBUTE; ++i)
    {
        if (objAttribute.GetAttributeEx().EqualsIgnoreCase(RESERVED_ATTRIBUTE[i]))
        {
            IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Reserved attribute (%s) can't be removed", strAttribute.GetStr(), 0, 0);
            return IMS_FAILURE;
        }
    }

    // Check if it already exists in the session
    if (!pSessionParam->Contains(objAttribute))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    pSessionParam->RemoveAttribute(objAttribute);  // throw exception : OPERATION_FAILED ???

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetSessionInfo(IN CONST AString& strInfo)
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpInformation objSessionInfo;

    if (!objSessionInfo.Decode(strInfo))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    pSessionParam->SetInformation(objSessionInfo);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetSessionName(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if (!pSessionParam->GetSessionName().Decode(strName))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::AddAttribute(IN IMS_SINT32 nType,
        IN CONST AString& strAttrValue, IN CONST AString& strType /* = AString::ConstNull() */)
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %s) in the state (%d)", nType,
                strAttrValue.GetStr(), nState);
        return IMS_FAILURE;
    }

    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY) || (nType == SdpAttribute::SETUP) ||
            (nType == SdpAttribute::CONNECTION))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d, %s)", nType, strAttrValue.GetStr(), 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.SetValue(nType, strAttrValue, strType))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Setting an attribute (%s) failed", strAttrValue.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    // Check if it already exists in the session
    if (pSessionParam->Contains(objAttribute))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Attribute (%s) already exists", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    if (!pSessionParam->AddAttribute(objAttribute))
    {
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Adding an attribute (%s) failed", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::AddAttributeInt(IN IMS_SINT32 nType,
        IN IMS_SINT32 nAttrValue, IN CONST AString& strType /* = AString::ConstNull() */)
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %d) in the state (%d)", nType, nAttrValue,
                nState);
        return IMS_FAILURE;
    }

    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d, %d)", nType, nAttrValue, 0);
        return IMS_FAILURE;
    }
    else if ((nType == SdpAttribute::SETUP) || (nType == SdpAttribute::CONNECTION))
    {
        SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

        if (pSessionParam == IMS_NULL)
        {
            IMS::SetLastError(IMSError::ILLEGAL_STATE);

            IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
            return IMS_FAILURE;
        }

        if (nType == SdpAttribute::SETUP)
        {
            pSessionParam->SetAttributeSetup(nAttrValue);
        }
        else if (nType == SdpAttribute::CONNECTION)
        {
            pSessionParam->SetAttributeConnection(nAttrValue);
        }

        return IMS_SUCCESS;
    }

    AString strAttrValue;

    strAttrValue.SetNumber(nAttrValue);

    return AddAttribute(nType, strAttrValue, strType);
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::AddBandwidth(IN IMS_SINT32 nType,
        IN IMS_SINT32 nBandwidth, IN CONST AString& strType /* = AString::ConstNull() */)
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set a bandwidth (%d, %d) in the state (%d)", nType, nBandwidth,
                nState);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpBandwidth objBandwidth;

    // Check a syntax of the attribute
    if (!objBandwidth.SetValue(nType, nBandwidth, strType))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Decoding an bandwidth (%d, %d, %s) failed", nType, nBandwidth,
                strType.GetStr());
        return IMS_FAILURE;
    }

    if (!pSessionParam->AddBandwidth(objBandwidth))
    {
        IMS::SetLastError(IMSError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Adding a bandwidth (%s) failed", objBandwidth.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetAttribute(
        IN IMS_SINT32 nType, IN CONST AString& strType /* = AString::ConstNull() */) const
{
    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    //---------------------------------------------------------------------------------------------

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer session parameter", 0, 0, 0);
        return AString::ConstNull();
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY) || (nType == SdpAttribute::SETUP) ||
            (nType == SdpAttribute::CONNECTION))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return AString::ConstNull();
    }

    const SdpAttribute* pAttribute = IMS_NULL;

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
        pAttribute = pSessionParam->GetAttribute(nType);
    else
        pAttribute = pSessionParam->GetAttribute(strType);

    if (pAttribute == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NOT_FOUND);
        return AString::ConstNull();
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pAttribute->GetAttributeValue();
}

PRIVATE VIRTUAL IMS_SINT32 SessionDescriptor::GetAttributeInt(
        IN IMS_SINT32 nType, IN CONST AString& strType /* = AString::ConstNull() */) const
{
    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    //---------------------------------------------------------------------------------------------

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer session parameter", 0, 0, 0);
        return INVALID_VALUE;
    }

    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return INVALID_VALUE;
    }
    else if (nType == SdpAttribute::SETUP)
    {
        return pSessionParam->GetAttributeSetup();
    }
    else if (nType == SdpAttribute::CONNECTION)
    {
        return pSessionParam->GetAttributeConnection();
    }

    const SdpAttribute* pAttribute = IMS_NULL;

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
        pAttribute = pSessionParam->GetAttribute(nType);
    else
        pAttribute = pSessionParam->GetAttribute(strType);

    if (pAttribute == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NOT_FOUND);
        return INVALID_VALUE;
    }

    const AString& strAttrValue = pAttribute->GetAttributeValue();

    if (strAttrValue.IsNULL())
    {
        return INVALID_VALUE;
    }

    IMS_BOOL bOK = IMS_FALSE;
    IMS_SINT32 nAttrValue = strAttrValue.ToInt32(&bOK);

    if (!bOK)
    {
        IMS_TRACE_E(0, "Converting the attribute (integer format: %d, %s) failed", nType,
                strType.GetStr(), 0);

        IMS::SetLastError(IMSError::INVALID_OPERATION);
        return INVALID_VALUE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return nAttrValue;
}

PRIVATE VIRTUAL IMS_SINT32 SessionDescriptor::GetBandwidth(
        IN IMS_SINT32 nType, IN CONST AString& strType /* = AString::ConstNull() */) const
{
    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    //---------------------------------------------------------------------------------------------

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer session parameter", 0, 0, 0);
        return INVALID_VALUE;
    }

    const SdpBandwidth* pBandwidth = IMS_NULL;

    if (nType != SdpBandwidth::TYPE_OTHER)
        pBandwidth = pSessionParam->GetBandwidth(nType);
    else
        pBandwidth = pSessionParam->GetBandwidth(strType);

    if (pBandwidth == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NOT_FOUND);
        return INVALID_VALUE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pBandwidth->GetBandwidth();
}

PRIVATE VIRTUAL IMS_SINT32 SessionDescriptor::GetDirection() const
{
    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    //---------------------------------------------------------------------------------------------

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer session parameter", 0, 0, 0);
        return Sdp::DIRECTION_NONE;
    }

    return pSessionParam->GetDirection();
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetSessionVersion() const
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetOrigin().GetSessionVersion();
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetUsername() const
{
    //---------------------------------------------------------------------------------------------

    if (piSessionState == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetOrigin().GetUsername();
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::RemoveAttribute(IN CONST SdpAttribute& objAttribute)
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %s) in the state (%d)",
                objAttribute.GetAttribute(), objAttribute.GetAttributeValue().GetStr(), nState);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return Sdp::DIRECTION_NONE;
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    IMS_SINT32 nType = objAttribute.GetAttribute();

    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    if (nType == SdpAttribute::SETUP)
    {
        pSessionParam->SetAttributeSetup(Sdp::SETUP_NONE);
        return IMS_SUCCESS;
    }
    else if (nType == SdpAttribute::CONNECTION)
    {
        pSessionParam->SetAttributeConnection(Sdp::CONNECTION_NONE);
        return IMS_SUCCESS;
    }

    // Check if it already exists in the session
    if (!pSessionParam->Contains(objAttribute))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Attribute (%s) does not exist", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    pSessionParam->RemoveAttribute(objAttribute);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::RemoveAttribute(IN IMS_SINT32 nType,
        IN CONST AString& strAttrValue /* = AString::ConstNull() */,
        IN CONST AString& strType /* = AString::ConstNull() */)
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %s) in the state (%d)", nType,
                strAttrValue.GetStr(), nState);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return Sdp::DIRECTION_NONE;
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    if (nType == SdpAttribute::SETUP)
    {
        pSessionParam->SetAttributeSetup(Sdp::SETUP_NONE);
        return IMS_SUCCESS;
    }
    else if (nType == SdpAttribute::CONNECTION)
    {
        pSessionParam->SetAttributeConnection(Sdp::CONNECTION_NONE);
        return IMS_SUCCESS;
    }

    if (!strAttrValue.IsNULL())
    {
        SdpAttribute objAttribute;

        objAttribute.SetValue(nType, strAttrValue, strType);

        if (!pSessionParam->Contains(objAttribute))
        {
            IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Attribute (%s) does not exist", strAttrValue.GetStr(), 0, 0);
            return IMS_FAILURE;
        }

        pSessionParam->RemoveAttribute(objAttribute);

        return IMS_SUCCESS;
    }

    // Check if it already exists in the session
    pSessionParam->RemoveAttribute(nType);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::RemoveAllBandwidths()
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to remove all the bandwidths in the state (%d)", nState, 0, 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMSList<SdpBandwidth> objBandwidths;

    pSessionParam->SetBandwidths(objBandwidths);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetConnectionAddress(IN CONST AString& strAddress)
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set the address (%s) in the state (%d)", strAddress.GetStr(),
                nState, 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pSessionParam->SetConnectionAddress(strAddress))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Setting a connection address (%s) in c-line of session-level",
                SipDebug::GetIp(strAddress), 0, 0);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetDirection(IN IMS_SINT32 nDirection)
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set the direction (%d) in the state (%d)", nDirection, nState, 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    pSessionParam->SetDirection(nDirection);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetOriginAddress(IN CONST AString& strAddress)
{
    // Check a session state
    IMS_SINT32 nState = piSessionState->GetSessionState();

    //---------------------------------------------------------------------------------------------

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set the address (%s) in the state (%d)",
                SipDebug::GetIp(strAddress), nState, 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpOrigin& objOrigin = const_cast<SdpOrigin&>(pSessionParam->GetOrigin());

    if (!objOrigin.SetAddress(strAddress))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Setting an unicast address (%s) in o-line of session-level",
                SipDebug::GetIp(strAddress), 0, 0);
        return IMS_FAILURE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IPAddress SessionDescriptor::GetLocalAddress() const
{
    //---------------------------------------------------------------------------------------------

    return IPAddress(piSessionState->GetConnectionAddress());
}

PRIVATE VIRTUAL IPAddress SessionDescriptor::GetRemoteAddress() const
{
    IPAddress objAddress;

    //---------------------------------------------------------------------------------------------

    if (!objAddress.Parse(GetRemoteAddressAsString()))
    {
        IMS_TRACE_D("Remote connection address may be FQDN or null", 0, 0, 0);
        return IPAddress::NONE;
    }

    return objAddress;
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetRemoteAddressAsString() const
{
    //---------------------------------------------------------------------------------------------

    return piSessionState->GetPeerConnectionAddress();
}
