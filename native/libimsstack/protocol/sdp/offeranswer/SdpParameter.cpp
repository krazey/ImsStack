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
#include "AStringBuffer.h"
#include "ServiceMemory.h"

#include "Sdp.h"
#include "offeranswer/SdpSessionParameter.h"

// 4 Check this attribute fields
#define __IMS_SETUP_CONNECTION__

PUBLIC
SdpParameter::SdpParameter() :
        m_bDirectionPresent(IMS_FALSE),
        m_nDirection(Sdp::DIRECTION_NONE),
        m_nPreviousDirection(Sdp::DIRECTION_NONE),
        m_nAttrSetup(Sdp::SETUP_NONE),
        m_nAttrConnection(Sdp::CONNECTION_NONE),
        m_pInformation(IMS_NULL),
        m_pKey(IMS_NULL)
{
    for (IMS_SINT32 i = 0; i < Sdp::TYPE_MAX; ++i)
    {
        m_abLineContains[i] = IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
    {
        m_abAttributeContains[i] = IMS_FALSE;
    }
}

PUBLIC
SdpParameter::SdpParameter(IN const SdpParameter& other) :
        m_bDirectionPresent(other.m_bDirectionPresent),
        m_nDirection(other.m_nDirection),
        m_nPreviousDirection(other.m_nPreviousDirection),
        m_nAttrSetup(other.m_nAttrSetup),
        m_nAttrConnection(other.m_nAttrConnection),
        m_pInformation(IMS_NULL),
        m_pKey(IMS_NULL)
{
    for (IMS_SINT32 i = 0; i < Sdp::TYPE_MAX; ++i)
    {
        m_abLineContains[i] = other.m_abLineContains[i];
    }

    for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
    {
        m_abAttributeContains[i] = other.m_abAttributeContains[i];
    }

    if (other.m_pInformation != IMS_NULL)
    {
        m_pInformation = new SdpInformation(*(other.m_pInformation));
    }

    if (other.m_pKey != IMS_NULL)
    {
        m_pKey = new SdpEncryptionKey(*(other.m_pKey));
    }

    m_objAttributes = other.m_objAttributes;
    m_objBandwidths = other.m_objBandwidths;
}

PUBLIC VIRTUAL SdpParameter::~SdpParameter()
{
    ClearAllParameters();
}

PUBLIC
SdpParameter& SdpParameter::operator=(IN const SdpParameter& other)
{
    if (this != &other)
    {
        for (IMS_SINT32 i = 0; i < Sdp::TYPE_MAX; ++i)
        {
            m_abLineContains[i] = other.m_abLineContains[i];
        }

        m_bDirectionPresent = other.m_bDirectionPresent;
        m_nDirection = other.m_nDirection;
        m_nPreviousDirection = other.m_nPreviousDirection;

        for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
        {
            m_abAttributeContains[i] = other.m_abAttributeContains[i];
        }

        m_nAttrSetup = other.m_nAttrSetup;
        m_nAttrConnection = other.m_nAttrConnection;

        if (m_pInformation != IMS_NULL)
        {
            delete m_pInformation;
            m_pInformation = IMS_NULL;
        }

        if (other.m_pInformation != IMS_NULL)
        {
            m_pInformation = new SdpInformation(*(other.m_pInformation));
        }

        if (m_pKey != IMS_NULL)
        {
            delete m_pKey;
            m_pKey = IMS_NULL;
        }

        if (other.m_pKey != IMS_NULL)
        {
            m_pKey = new SdpEncryptionKey(*(other.m_pKey));
        }

        m_objBandwidths.Clear();
        m_objBandwidths = other.m_objBandwidths;

        m_objAttributes.Clear();
        m_objAttributes = other.m_objAttributes;
    }

    return (*this);
}

PUBLIC VIRTUAL AString SdpParameter::ToSdp() const
{
    AStringBuffer objSdp(256);

    if (m_bDirectionPresent && IsDirectionAttributeRequired())
    {
        SdpAttribute objAttr;

        objAttr.SetValue(
                SdpAttribute::ConvertDirectionToAttribute(GetDirection()), AString::ConstNull());
        objSdp.Append(objAttr.Encode());
    }

#ifdef __IMS_SETUP_CONNECTION__
    if (m_abAttributeContains[ATTR_SETUP])
    {
        SdpAttribute objAttr;
        AString strAttrValue(Sdp::STR_A_SETUP[m_nAttrSetup]);

        objAttr.SetValue(SdpAttribute::SETUP, strAttrValue);
        objSdp.Append(objAttr.Encode());
    }

    if (m_abAttributeContains[ATTR_CONNECTION])
    {
        SdpAttribute objAttr;
        AString strAttrValue(Sdp::STR_A_CONNECTION[m_nAttrConnection]);

        objAttr.SetValue(SdpAttribute::CONNECTION, strAttrValue);
        objSdp.Append(objAttr.Encode());
    }
#endif  // __IMS_SETUP_CONNECTION__

    if (m_abLineContains[Sdp::TYPE_A])
    {
        for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
        {
            const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

            objSdp.Append(objAttribute.Encode());
        }
    }

    return static_cast<const AStringBuffer&>(objSdp).GetString();
}

PUBLIC
IMS_BOOL SdpParameter::AddAttribute(IN const SdpAttribute& objAttribute)
{
#ifdef __IMS_SETUP_CONNECTION__
    IMS_SINT32 nAttribute = objAttribute.GetAttribute();

    if (nAttribute == SdpAttribute::SETUP)
    {
        IMS_SINT32 nTypeOfSetup = Sdp::SETUP_NONE;

        Sdp::ParseAttributeSetup(objAttribute.GetAttributeValue(), nTypeOfSetup);

        if (nTypeOfSetup == Sdp::SETUP_NONE)
        {
            return IMS_FALSE;
        }

        m_nAttrSetup = nTypeOfSetup;
        m_abAttributeContains[ATTR_SETUP] = IMS_TRUE;

        return IMS_TRUE;
    }
    else if (nAttribute == SdpAttribute::CONNECTION)
    {
        IMS_SINT32 nTypeOfConnection = Sdp::CONNECTION_NONE;

        Sdp::ParseAttributeConnection(objAttribute.GetAttributeValue(), nTypeOfConnection);

        if (nTypeOfConnection == Sdp::CONNECTION_NONE)
        {
            return IMS_FALSE;
        }

        m_nAttrConnection = nTypeOfConnection;
        m_abAttributeContains[ATTR_CONNECTION] = IMS_TRUE;

        return IMS_TRUE;
    }
#endif

    if (!m_objAttributes.Append(objAttribute))
    {
        return IMS_FALSE;
    }

    if (!m_abLineContains[Sdp::TYPE_A])
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_TRUE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpParameter::AddBandwidth(IN const SdpBandwidth& objBandwidth)
{
    if (!m_objBandwidths.Append(objBandwidth))
    {
        return IMS_FALSE;
    }

    if (!m_abLineContains[Sdp::TYPE_B])
    {
        m_abLineContains[Sdp::TYPE_B] = IMS_TRUE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpParameter::Contains(IN const SdpAttribute& objAttribute)
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objExAttribute = m_objAttributes.GetAt(i);

        if (objExAttribute.Equals(&objAttribute))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL SdpParameter::Contains(IN const SdpBandwidth& objBandwidth)
{
    for (IMS_UINT32 i = 0; i < m_objBandwidths.GetSize(); ++i)
    {
        const SdpBandwidth& objExBandwidth = m_objBandwidths.GetAt(i);

        if (objExBandwidth.Equals(&objBandwidth))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL SdpParameter::Create(IN const SdpDescription& objDescription)
{
    if (objDescription.Contains(Sdp::TYPE_I))
    {
        const SdpInformation* pInfo = objDescription.GetInformation();

        m_pInformation = new SdpInformation(*pInfo);
    }

    if (objDescription.Contains(Sdp::TYPE_B))
    {
        m_objBandwidths = objDescription.GetBandwidths();
    }

    if (objDescription.Contains(Sdp::TYPE_K))
    {
        const SdpEncryptionKey* pNewKey = objDescription.GetEncryptionKey();

        m_pKey = new SdpEncryptionKey(*pNewKey);
    }

    if (objDescription.Contains(Sdp::TYPE_A))
    {
        m_objAttributes = objDescription.GetAttributes();
    }

    for (IMS_SINT32 i = 0; i < Sdp::TYPE_MAX; ++i)
    {
        m_abLineContains[i] = objDescription.Contains(i);
    }

    // Set a direction
    m_nPreviousDirection = m_nDirection;
    m_nDirection = objDescription.GetDirection();

    if (m_nDirection == Sdp::DIRECTION_NONE)
    {
        m_bDirectionPresent = IMS_FALSE;
        m_nDirection = Sdp::DIRECTION_NONE;
    }
    else
    {
        m_bDirectionPresent = IMS_TRUE;
        RemoveAttribute(SdpAttribute::ConvertDirectionToAttribute(m_nDirection));
    }

#ifdef __IMS_SETUP_CONNECTION__
    // Setup attribute
    const SdpAttribute* pSetup = objDescription.GetAttribute(SdpAttribute::SETUP);

    if (pSetup == IMS_NULL)
    {
        m_abAttributeContains[ATTR_SETUP] = IMS_FALSE;
        m_nAttrSetup = Sdp::SETUP_NONE;
    }
    else
    {
        m_abAttributeContains[ATTR_SETUP] = IMS_TRUE;
        Sdp::ParseAttributeSetup(pSetup->GetAttributeValue(), m_nAttrSetup);

        RemoveAttribute(SdpAttribute::SETUP);
    }

    // Connection attribute
    const SdpAttribute* pConnection = objDescription.GetAttribute(SdpAttribute::CONNECTION);

    if (pConnection == IMS_NULL)
    {
        m_abAttributeContains[ATTR_CONNECTION] = IMS_FALSE;
        m_nAttrConnection = Sdp::CONNECTION_NONE;
    }
    else
    {
        m_abAttributeContains[ATTR_CONNECTION] = IMS_TRUE;
        Sdp::ParseAttributeConnection(pConnection->GetAttributeValue(), m_nAttrConnection);

        RemoveAttribute(SdpAttribute::CONNECTION);
    }
#endif  // __IMS_SETUP_CONNECTION__

    if (m_objAttributes.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
const SdpAttribute* SdpParameter::GetAttribute(IN IMS_SINT32 nAttribute) const
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttribute() == nAttribute)
        {
            return &objAttribute;
        }
    }

    return IMS_NULL;
}

PUBLIC
const SdpAttribute* SdpParameter::GetAttribute(IN const AString& strAttribute) const
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttributeName().Equals(strAttribute))
        {
            return &objAttribute;
        }
    }

    return IMS_NULL;
}

PUBLIC
ImsList<SdpAttribute> SdpParameter::GetAttributes(IN IMS_SINT32 nAttribute) const
{
    ImsList<SdpAttribute> objCollectedAttributes;

    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttribute() == nAttribute)
        {
            objCollectedAttributes.Append(objAttribute);
        }
    }

    return objCollectedAttributes;
}

PUBLIC
ImsList<SdpAttribute> SdpParameter::GetAttributes(IN const AString& strAttribute) const
{
    ImsList<SdpAttribute> objCollectedAttributes;

    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttributeName().Equals(strAttribute))
        {
            objCollectedAttributes.Append(objAttribute);
        }
    }

    return objCollectedAttributes;
}

PUBLIC
const SdpBandwidth* SdpParameter::GetBandwidth(IN IMS_SINT32 nType) const
{
    if (nType == SdpBandwidth::TYPE_OTHER)
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objBandwidths.GetSize(); ++i)
    {
        const SdpBandwidth& objBandwidth = m_objBandwidths.GetAt(i);

        if (objBandwidth.GetType() == nType)
        {
            return &objBandwidth;
        }
    }

    return IMS_NULL;
}

PUBLIC
const SdpBandwidth* SdpParameter::GetBandwidth(IN const AString& strType) const
{
    for (IMS_UINT32 i = 0; i < m_objBandwidths.GetSize(); ++i)
    {
        const SdpBandwidth& objBandwidth = m_objBandwidths.GetAt(i);

        if (objBandwidth.GetType() != SdpBandwidth::TYPE_OTHER)
        {
            continue;
        }

        if (strType.EqualsIgnoreCase(objBandwidth.GetTypeName()))
        {
            return &objBandwidth;
        }
    }

    return IMS_NULL;
}

PUBLIC
void SdpParameter::NegotiateDirection(IN IMS_SINT32 nCurrentDirection)
{
    // Special Case
    // If the peer is provided with "sendrecv" and our direction is "sendonly" / "recvonly",
    // then change the direction accordingly i.e. "recvonly" / "sendonly".
    // So that while changing the direction for the negotiated session, the peer's view
    // and the negotiated will be in sync.
    // e.g.
    // Received: "sendrecv", Local: "recvonly"
    // Here it will be updated as "sendonly" and after the negotiation, the negotiated session
    // will be having " recvonly" and the peer's view will be having "sendonly".

    // In the case of "inactive", in either side (from the peer or local the resultant negotiated
    // as well as the peer's view shall be "inactive".
    if ((m_nDirection == Sdp::DIRECTION_SENDRECV) || (m_nDirection == Sdp::DIRECTION_NONE))
    {
        if (nCurrentDirection == Sdp::DIRECTION_SENDONLY)
        {
            m_nDirection = Sdp::DIRECTION_RECVONLY;
        }
        else if (nCurrentDirection == Sdp::DIRECTION_RECVONLY)
        {
            m_nDirection = Sdp::DIRECTION_SENDONLY;
        }
        else if (nCurrentDirection == Sdp::DIRECTION_SENDRECV)
        {
            m_nDirection = Sdp::DIRECTION_SENDRECV;
        }
    }
    else if (nCurrentDirection == Sdp::DIRECTION_INACTIVE)
    {
        m_nDirection = Sdp::DIRECTION_INACTIVE;
    }
}

PUBLIC
void SdpParameter::RemoveAttribute(IN IMS_SINT32 nAttribute)
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objExAttribute = m_objAttributes.GetAt(i);

        if (objExAttribute.GetAttribute() == nAttribute)
        {
            m_objAttributes.RemoveAt(i);
            break;
        }
    }

    if (m_objAttributes.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
    }
}

PUBLIC
void SdpParameter::RemoveAttribute(IN const SdpAttribute& objAttribute)
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objExAttribute = m_objAttributes.GetAt(i);

        if (objExAttribute.Equals(&objAttribute))
        {
            m_objAttributes.RemoveAt(i);
            break;
        }
    }

    if (m_objAttributes.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
    }
}

PUBLIC
void SdpParameter::RemoveAttributes(IN IMS_SINT32 nAttribute)
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize();)
    {
        const SdpAttribute& objExAttribute = m_objAttributes.GetAt(i);

        if (objExAttribute.GetAttribute() == nAttribute)
        {
            m_objAttributes.RemoveAt(i);
        }
        else
        {
            ++i;
        }
    }

    if (m_objAttributes.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
    }
}

PUBLIC
void SdpParameter::RemoveAttributes()
{
    m_objAttributes.Clear();

    m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
}

PUBLIC
void SdpParameter::RemoveEncryptionKey()
{
    if (m_pKey != IMS_NULL)
    {
        delete m_pKey;
        m_pKey = IMS_NULL;
    }

    m_abLineContains[Sdp::TYPE_K] = IMS_FALSE;
}

PUBLIC
void SdpParameter::RemoveInformation()
{
    if (m_pInformation != IMS_NULL)
    {
        delete m_pInformation;
        m_pInformation = IMS_NULL;
    }

    m_abLineContains[Sdp::TYPE_I] = IMS_FALSE;
}

PUBLIC
void SdpParameter::SetBandwidths(IN const ImsList<SdpBandwidth>& objBandwidths)
{
    m_objBandwidths.Clear();
    m_objBandwidths = objBandwidths;

    if (m_objBandwidths.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_B] = IMS_FALSE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_B] = IMS_TRUE;
    }
}

PUBLIC
void SdpParameter::SetDirection(IN IMS_SINT32 nDirection)
{
    if ((nDirection < Sdp::DIRECTION_NONE) || (nDirection > Sdp::DIRECTION_SENDRECV))
    {
        return;
    }

    m_nPreviousDirection = m_nDirection;
    m_nDirection = nDirection;

    if (m_nDirection == Sdp::DIRECTION_NONE)
    {
        m_bDirectionPresent = IMS_FALSE;
    }
    else
    {
        m_bDirectionPresent = IMS_TRUE;
    }
}

PUBLIC
void SdpParameter::SetEncryptionKey(IN const SdpEncryptionKey& objEncryptionKey)
{
    if (m_pKey != IMS_NULL)
    {
        delete m_pKey;
    }

    m_pKey = new SdpEncryptionKey(objEncryptionKey);

    if (m_pKey != IMS_NULL)
    {
        m_abLineContains[Sdp::TYPE_K] = IMS_TRUE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_K] = IMS_FALSE;
    }
}

PUBLIC
void SdpParameter::SetInformation(IN const SdpInformation& objInformation)
{
    if (m_pInformation != IMS_NULL)
    {
        delete m_pInformation;
    }

    m_pInformation = new SdpInformation(objInformation);

    if (m_pInformation != IMS_NULL)
    {
        m_abLineContains[Sdp::TYPE_I] = IMS_TRUE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_I] = IMS_FALSE;
    }
}

PUBLIC
void SdpParameter::SetAttributeConnection(IN IMS_SINT32 nAttrConnection)
{
    if ((nAttrConnection < Sdp::CONNECTION_NONE) || (nAttrConnection >= Sdp::CONNECTION_MAX))
    {
        return;
    }

    m_nAttrConnection = nAttrConnection;

    if (m_nAttrConnection == Sdp::CONNECTION_NONE)
    {
        m_abAttributeContains[ATTR_CONNECTION] = IMS_FALSE;
    }
    else
    {
        m_abAttributeContains[ATTR_CONNECTION] = IMS_TRUE;
    }
}

PUBLIC
void SdpParameter::SetAttributeSetup(IN IMS_SINT32 nAttrSetup)
{
    if ((nAttrSetup < Sdp::SETUP_NONE) || (nAttrSetup >= Sdp::SETUP_MAX))
    {
        return;
    }

    m_nAttrSetup = nAttrSetup;

    if (m_nAttrSetup == Sdp::SETUP_NONE)
    {
        m_abAttributeContains[ATTR_SETUP] = IMS_FALSE;
    }
    else
    {
        m_abAttributeContains[ATTR_SETUP] = IMS_TRUE;
    }
}

PUBLIC
void SdpParameter::UpdateDirection()
{
    if (m_nDirection == Sdp::DIRECTION_SENDONLY)
    {
        m_nDirection = Sdp::DIRECTION_RECVONLY;
    }
    else if (m_nDirection == Sdp::DIRECTION_RECVONLY)
    {
        m_nDirection = Sdp::DIRECTION_SENDONLY;
    }
}

PUBLIC
void SdpParameter::UpdateDirection(IN const SdpParameter& objParam)
{
    m_bDirectionPresent = objParam.m_bDirectionPresent;

    m_nPreviousDirection = m_nDirection;
    m_nDirection = objParam.m_nDirection;
}

PUBLIC
void SdpParameter::UpdateDirection(
        IN const SdpParameter& objPeer, OUT SdpParameter& objProposal) const
{
    // Special Case
    // If the peer is provided with "sendrecv" and our direction is "sendonly" / "recvonly",
    // then change the direction accordingly i.e. "recvonly" / "sendonly".
    // So that while changing the direction for the negotiated session, the peer's view
    // and the negotiated will be in sync.
    // e.g.
    // Received: "sendrecv", Local: "recvonly"
    // Here it will be updated as "sendonly" and after the negotiation, the negotiated session
    // will be having " recvonly" and the peer's view will be having "sendonly".

    // In the case of "inactive", in either side (from the peer or local the resultant negotiated
    // as well as the peer's view shall be "inactive".

    if ((objPeer.m_nDirection == Sdp::DIRECTION_SENDRECV) ||
            (objPeer.m_nDirection == Sdp::DIRECTION_NONE))
    {
        if (m_nDirection == Sdp::DIRECTION_SENDONLY)
        {
            objProposal.m_bDirectionPresent = IMS_TRUE;
            objProposal.m_nDirection = Sdp::DIRECTION_RECVONLY;
        }
        else if (m_nDirection == Sdp::DIRECTION_RECVONLY)
        {
            objProposal.m_bDirectionPresent = IMS_TRUE;
            objProposal.m_nDirection = Sdp::DIRECTION_SENDONLY;
        }
        else if (m_nDirection == Sdp::DIRECTION_SENDRECV)
        {
            objProposal.m_bDirectionPresent = IMS_TRUE;
            objProposal.m_nDirection = Sdp::DIRECTION_SENDRECV;
        }
    }

    if (m_nDirection == Sdp::DIRECTION_INACTIVE)
    {
        objProposal.m_bDirectionPresent = IMS_TRUE;
        objProposal.m_nDirection = Sdp::DIRECTION_INACTIVE;
    }

    objProposal.m_nPreviousDirection = m_nPreviousDirection;

    /*
    if (m_nDirection != Sdp::DIRECTION_NONE)
    {
        objProposed.m_bDirectionPresent = IMS_TRUE;
    }
    */
}

PUBLIC
IMS_BOOL SdpParameter::ValidateDirection(IN const SdpParameter* pPeer) const
{
    // sendonly - sendonly or recvonly - recvonly ---> failure
    // sendonly - recvonly or recvonly - sendonly ---> success
    // sendonly - sendrecv or recvonly - sendrecv ---> success
    // New direction is "inactive" or "sendrecv", then success

    if (pPeer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Check the validity for direction attribute
    if (pPeer->GetDirection() == Sdp::DIRECTION_RECVONLY)
    {
        if (m_nDirection == Sdp::DIRECTION_RECVONLY)
        {
            return IMS_FALSE;
        }
    }
    else if (pPeer->GetDirection() == Sdp::DIRECTION_SENDONLY)
    {
        if (m_nDirection == Sdp::DIRECTION_SENDONLY)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void SdpParameter::Clear()
{
    ClearAllParameters();
}

PROTECTED
void SdpParameter::ClearAllParameters()
{
    if (m_pInformation != IMS_NULL)
    {
        delete m_pInformation;
        m_pInformation = IMS_NULL;
    }

    m_objBandwidths.Clear();

    if (m_pKey != IMS_NULL)
    {
        delete m_pKey;
        m_pKey = IMS_NULL;
    }

    m_objAttributes.Clear();

    for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
    {
        m_abAttributeContains[i] = IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < Sdp::TYPE_MAX; ++i)
    {
        m_abLineContains[i] = IMS_FALSE;
    }
}

PROTECTED
IMS_BOOL SdpParameter::Contains(IN IMS_SINT32 nType) const
{
    if ((nType < Sdp::TYPE_V) || (nType >= Sdp::TYPE_MAX))
    {
        return IMS_FALSE;
    }

    return m_abLineContains[nType];
}

PROTECTED
void SdpParameter::UpdateProperties(IN const SdpParameter& objParam)
{
    m_nPreviousDirection = objParam.m_nPreviousDirection;

    if (objParam.Contains(Sdp::TYPE_I))
    {
        if (m_pInformation != IMS_NULL)
        {
            delete m_pInformation;
        }

        m_pInformation = new SdpInformation(*(objParam.m_pInformation));

        m_abLineContains[Sdp::TYPE_I] = IMS_TRUE;
    }

    if (objParam.Contains(Sdp::TYPE_B))
    {
        m_objBandwidths.Clear();

        m_objBandwidths = objParam.m_objBandwidths;

        m_abLineContains[Sdp::TYPE_B] = IMS_TRUE;
    }

    if (objParam.Contains(Sdp::TYPE_K))
    {
        if (m_pKey != IMS_NULL)
        {
            delete m_pKey;
        }

        m_pKey = new SdpEncryptionKey(*(objParam.m_pKey));

        m_abLineContains[Sdp::TYPE_K] = IMS_TRUE;
    }

    for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
    {
        m_abAttributeContains[i] = objParam.m_abAttributeContains[i];
    }

    // Setup attribute
    if (m_abAttributeContains[ATTR_SETUP])
    {
        m_nAttrSetup = objParam.m_nAttrSetup;
    }

    // Connection attribute
    if (m_abAttributeContains[ATTR_CONNECTION])
    {
        m_nAttrConnection = objParam.m_nAttrConnection;
    }

    // Replaces all the attributes by my SDP information
    if (objParam.Contains(Sdp::TYPE_A))
    {
        m_objAttributes.Clear();
        m_objAttributes = objParam.m_objAttributes;

        m_abLineContains[Sdp::TYPE_A] = IMS_TRUE;
    }
}

PROTECTED GLOBAL IMS_BOOL SdpParameter::ValidateDirection(
        IN IMS_SINT32 nCurrentDirection, IN IMS_SINT32 nOfferDirection)
{
    // sendonly - sendonly or recvonly - recvonly ---> failure
    // sendonly - recvonly or recvonly - sendonly ---> success
    // sendonly - sendrecv or recvonly - sendrecv ---> success
    // New direction is "inactive" or "sendrecv", then success

    // Check the validity for direction attribute
    if (nCurrentDirection == Sdp::DIRECTION_RECVONLY)
    {
        if (nOfferDirection == Sdp::DIRECTION_RECVONLY)
        {
            return IMS_FALSE;
        }
    }
    else if (nCurrentDirection == Sdp::DIRECTION_SENDONLY)
    {
        if (nOfferDirection == Sdp::DIRECTION_SENDONLY)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}
