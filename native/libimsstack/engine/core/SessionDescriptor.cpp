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

#include "offeranswer/SdpSessionParameter.h"

#include "ISessionState.h"
#include "SessionDescriptor.h"
#include "SipDebug.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_IMS_CORE__;

// clang-format off
PUBLIC GLOBAL const IMS_CHAR*
SessionDescriptor::RESERVED_ATTRIBUTE[SessionDescriptor::MAX_RESERVED_ATTRIBUTE] =
{
    "charset",
    "charset:ISO8895-1",  // ? It needs confirmation
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
// clang-format on

PUBLIC
SessionDescriptor::SessionDescriptor(IN ISessionState* piSessionState) :
        m_piSessionState(piSessionState)
{
}

PUBLIC VIRTUAL SessionDescriptor::~SessionDescriptor() {}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::AddAttribute(IN const AString& strAttribute)
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.Decode(strAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check if it is a reserved or not
    for (IMS_SINT32 i = 0; i < MAX_RESERVED_ATTRIBUTE; ++i)
    {
        if (objAttribute.GetAttributeName().Equals(RESERVED_ATTRIBUTE[i]))
        {
            Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Reserved attribute (%s) can't be added", strAttribute.GetStr(), 0, 0);
            return IMS_FAILURE;
        }
    }

    // Check if it already exists in the session
    if (pSessionParam->Contains(objAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!pSessionParam->AddAttribute(objAttribute))
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL ImsList<AString> SessionDescriptor::GetAttributes() const
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return ImsList<AString>();
    }

    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return ImsList<AString>();
    }

    const ImsList<SdpAttribute>& objSdpAttributes = pSessionParam->GetAttributes();
    ImsList<AString> objAttributes;

    for (IMS_UINT32 i = 0; i < objSdpAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = objSdpAttributes.GetAt(i);

        objAttributes.Append(objAttribute.GetValue());
    }

    return objAttributes;
}

PRIVATE VIRTUAL AString SessionDescriptor::GetProtocolVersion() const
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetVersion().GetValue();
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetSessionId() const
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetOrigin().GetSessionId();
}

PRIVATE VIRTUAL AString SessionDescriptor::GetSessionInfo() const
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

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
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetSessionName().GetValue();
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::RemoveAttribute(IN const AString& strAttribute)
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.Decode(strAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check if it is a reserved or not
    for (IMS_SINT32 i = 0; i < MAX_RESERVED_ATTRIBUTE; ++i)
    {
        if (objAttribute.GetAttributeName().EqualsIgnoreCase(RESERVED_ATTRIBUTE[i]))
        {
            Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Reserved attribute (%s) can't be removed", strAttribute.GetStr(), 0, 0);
            return IMS_FAILURE;
        }
    }

    // Check if it already exists in the session
    if (!pSessionParam->Contains(objAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    pSessionParam->RemoveAttribute(objAttribute);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetSessionInfo(IN const AString& strInfo)
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpInformation objSessionInfo;

    if (!objSessionInfo.Decode(strInfo))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    pSessionParam->SetInformation(objSessionInfo);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetSessionName(IN const AString& strName)
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if (!pSessionParam->GetSessionName().Decode(strName))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::AddAttribute(IN IMS_SINT32 nType,
        IN const AString& strAttrValue, IN const AString& strType /*= AString::ConstNull()*/)
{
    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

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
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d, %s)", nType, strAttrValue.GetStr(), 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.SetValue(nType, strAttrValue, strType))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Setting an attribute (%s) failed", strAttrValue.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    // Check if it already exists in the session
    if (pSessionParam->Contains(objAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Attribute (%s) already exists", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    if (!pSessionParam->AddAttribute(objAttribute))
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Adding an attribute (%s) failed", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::AddAttributeInt(IN IMS_SINT32 nType,
        IN IMS_SINT32 nAttrValue, IN const AString& strType /*= AString::ConstNull()*/)
{
    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %d) in the state (%d)", nType, nAttrValue,
                nState);
        return IMS_FAILURE;
    }

    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d, %d)", nType, nAttrValue, 0);
        return IMS_FAILURE;
    }
    else if ((nType == SdpAttribute::SETUP) || (nType == SdpAttribute::CONNECTION))
    {
        SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

        if (pSessionParam == IMS_NULL)
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);

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
        IN IMS_SINT32 nBandwidth, IN const AString& strType /*= AString::ConstNull()*/)
{
    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set a bandwidth (%d, %d) in the state (%d)", nType, nBandwidth,
                nState);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpBandwidth objBandwidth;

    // Check a syntax of the attribute
    if (!objBandwidth.SetValue(nType, nBandwidth, strType))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(
                0, "Decoding a bandwidth (%d, %d, %s) failed", nType, nBandwidth, strType.GetStr());
        return IMS_FAILURE;
    }

    if (!pSessionParam->AddBandwidth(objBandwidth))
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Adding a bandwidth (%s) failed", objBandwidth.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetAttribute(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer session parameter", 0, 0, 0);
        return AString::ConstNull();
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY) || (nType == SdpAttribute::SETUP) ||
            (nType == SdpAttribute::CONNECTION))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return AString::ConstNull();
    }

    const SdpAttribute* pAttribute = IMS_NULL;

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
    {
        pAttribute = pSessionParam->GetAttribute(nType);
    }
    else
    {
        pAttribute = pSessionParam->GetAttribute(strType);
    }

    if (pAttribute == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NOT_FOUND);
        return AString::ConstNull();
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pAttribute->GetAttributeValue();
}

PRIVATE VIRTUAL IMS_SINT32 SessionDescriptor::GetAttributeInt(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer session parameter", 0, 0, 0);
        return INVALID_VALUE;
    }

    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

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
    {
        pAttribute = pSessionParam->GetAttribute(nType);
    }
    else
    {
        pAttribute = pSessionParam->GetAttribute(strType);
    }

    if (pAttribute == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NOT_FOUND);
        return INVALID_VALUE;
    }

    const AString& strAttrValue = pAttribute->GetAttributeValue();

    if (strAttrValue.IsNULL())
    {
        return INVALID_VALUE;
    }

    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nAttrValue = strAttrValue.ToInt32(&bOk);

    if (!bOk)
    {
        IMS_TRACE_E(0, "Converting the attribute (integer format: %d, %s) failed", nType,
                strType.GetStr(), 0);

        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return INVALID_VALUE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return nAttrValue;
}

PRIVATE VIRTUAL IMS_SINT32 SessionDescriptor::GetBandwidth(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer session parameter", 0, 0, 0);
        return INVALID_VALUE;
    }

    const SdpBandwidth* pBandwidth = IMS_NULL;

    if (nType != SdpBandwidth::TYPE_OTHER)
    {
        pBandwidth = pSessionParam->GetBandwidth(nType);
    }
    else
    {
        pBandwidth = pSessionParam->GetBandwidth(strType);
    }

    if (pBandwidth == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NOT_FOUND);
        return INVALID_VALUE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pBandwidth->GetBandwidth();
}

PRIVATE VIRTUAL IMS_SINT32 SessionDescriptor::GetDirection() const
{
    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer session parameter", 0, 0, 0);
        return Sdp::DIRECTION_NONE;
    }

    return pSessionParam->GetDirection();
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetSessionVersion() const
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetOrigin().GetSessionVersion();
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetUsername() const
{
    if (m_piSessionState == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    const SdpSessionParameter* pSessionParam = m_piSessionState->GetPeerSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSessionParam->GetOrigin().GetUsername();
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::RemoveAttribute(IN const SdpAttribute& objAttribute)
{
    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %s) in the state (%d)",
                objAttribute.GetAttribute(), objAttribute.GetAttributeValue().GetStr(), nState);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return Sdp::DIRECTION_NONE;
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    IMS_SINT32 nType = objAttribute.GetAttribute();

    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

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
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Attribute (%s) does not exist", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    pSessionParam->RemoveAttribute(objAttribute);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::RemoveAttribute(IN IMS_SINT32 nType,
        IN const AString& strAttrValue /*= AString::ConstNull()*/,
        IN const AString& strType /*= AString::ConstNull()*/)
{
    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %s) in the state (%d)", nType,
                strAttrValue.GetStr(), nState);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return Sdp::DIRECTION_NONE;
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

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
            Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

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
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to remove all the bandwidths in the state (%d)", nState, 0, 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    ImsList<SdpBandwidth> objBandwidths;

    pSessionParam->SetBandwidths(objBandwidths);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetConnectionAddress(IN const AString& strAddress)
{
    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set the address (%s) in the state (%d)", strAddress.GetStr(),
                nState, 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pSessionParam->SetConnectionAddress(strAddress))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Setting a connection address (%s) in c-line of session-level",
                SipDebug::GetIp(strAddress), 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetDirection(IN IMS_SINT32 nDirection)
{
    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set the direction (%d) in the state (%d)", nDirection, nState, 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    pSessionParam->SetDirection(nDirection);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SessionDescriptor::SetOriginAddress(IN const AString& strAddress)
{
    // Check a session state
    IMS_SINT32 nState = m_piSessionState->GetSessionState();

    if ((nState != ISessionState::SESSION_STATE_INITIATED) &&
            (nState != ISessionState::SESSION_STATE_NEGOTIATING) &&
            (nState != ISessionState::SESSION_STATE_ESTABLISHED) &&
            (nState != ISessionState::SESSION_STATE_RENEGOTIATING))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set the address (%s) in the state (%d)",
                SipDebug::GetIp(strAddress), nState, 0);
        return IMS_FAILURE;
    }

    SdpSessionParameter* pSessionParam = m_piSessionState->GetProposalSessionParameter();

    if (pSessionParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal session parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpOrigin& objOrigin = const_cast<SdpOrigin&>(pSessionParam->GetOrigin());

    if (!objOrigin.SetAddress(strAddress))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Setting an unicast address (%s) in o-line of session-level",
                SipDebug::GetIp(strAddress), 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IpAddress SessionDescriptor::GetLocalAddress() const
{
    return IpAddress(m_piSessionState->GetConnectionAddress());
}

PRIVATE VIRTUAL IpAddress SessionDescriptor::GetRemoteAddress() const
{
    IpAddress objAddress;

    if (!objAddress.Parse(GetRemoteAddressAsString()))
    {
        IMS_TRACE_D("Remote connection address may be FQDN or null", 0, 0, 0);
        return IpAddress::NONE;
    }

    return objAddress;
}

PRIVATE VIRTUAL const AString& SessionDescriptor::GetRemoteAddressAsString() const
{
    return m_piSessionState->GetPeerConnectionAddress();
}
