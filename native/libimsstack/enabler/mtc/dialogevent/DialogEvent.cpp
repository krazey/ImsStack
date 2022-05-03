/*
 * author : hyunbin.shin@
 * version : 2.0
 * date : 201503
 * brief :
 */

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"

#include "ICoreService.h"
#include "ISession.h"
#include "ISubscription.h"
#include "ISipMessage.h"
#include "ISipHeader.h"
#include "IMessageBodyPart.h"
#include "IMessage.h"
#include "TextParser.h"

#include "DomDocumentBuilderFactory.h"
#include "DocumentBuilder.h"
#include "IElement.h"
#include "INodeList.h"
#include "INamedNodeMap.h"
#include "IText.h"

#include "configuration/ConfigDef.h"
#include "dialogevent/DialogEvent.h"
#include "dialogevent/DialogEventManager.h"

__IMS_TRACE_TAG_COM_UC__;

/* ------------------------------------------------------------------------------------------------
    Constructor, Destructor
------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
UCDialog::UCDialog(IN IMtcApp* pApp) :
        m_nInfoVersion(0),
        m_eInfoState(DEMngr::DIALOGINFO_STATE_IDLE),
        m_aStrInfoEntity(AString::ConstNull()),
        m_aStrID(AString::ConstNull()),
        m_aStrCallID(AString::ConstNull()),
        m_aStrLocalTag(AString::ConstNull()),
        m_aStrRemoteTag(AString::ConstNull()),
        m_aStrDirection(AString::ConstNull()),
        m_eState(0),
        m_aStrState_Event(AString::ConstNull()),
        m_aStrState_Code(AString::ConstNull()),
        m_nDuration(0),
        m_aStrReplaces_CallID(AString::ConstNull()),
        m_aStrReplaces_LocalTag(AString::ConstNull()),
        m_aStrReplaces_RemoteTag(AString::ConstNull()),
        m_aStrReferredBy(AString::ConstNull()),
        m_aStrReferredBy_Display(AString::ConstNull()),
        m_bEnablePulled(IMS_FALSE),
        m_pApp(pApp),
        m_nSlotID(0)
{
    // TODO, MTC BUILD
    // IMS_TRACE_MEM("uc", "uc_M[%d] : UCDialog[%" PFLS_u "][%" PFLS_x "]"
    //                 , m_pApp->GetSlotID(), sizeof(UCDialog), this);

    // m_nSlotID = m_pApp->GetSlotID();
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL UCDialog::~UCDialog()
{
    IMS_TRACE_MEM("uc", "uc_F[%d] : UCDialog[%" PFLS_u "][%" PFLS_x "]", m_nSlotID,
            sizeof(UCDialog), this);

    if (m_pApp != IMS_NULL)
    {
        m_pApp = IMS_NULL;
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::Init(IN IElement* pDialogElement)
{
    AString aStrID = AString::ConstNull();

    IMS_TRACE_I("Init", 0, 0, 0);

    LoadConfig();
    AddEventListn();

    if (UpdateDialog(pDialogElement))
    {
        aStrID = GetID();
    }

    return aStrID;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCDialog::DeInit()
{
    IMS_TRACE_I("DeInit", 0, 0, 0);

    DeleteEventListn();
}

/* ------------------------------------------------------------------------------------------------
    PUBLIC METHODS
------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_BOOL UCDialog::UpdateDialogInfo(
        IN IMS_UINT32 nVersion, IN IMS_UINT32 eState, IN AString aStrEntity)
{
    IMS_TRACE_D("UpdateDialogInfo : [%d][%d][%s]", nVersion, eState, aStrEntity.GetStr());

    m_nInfoVersion = nVersion;
    m_eInfoState = eState;
    m_aStrInfoEntity = aStrEntity;

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_BOOL UCDialog::Update(IN IElement* pDialogElement)
{
    IMS_BOOL bUpdated = IMS_FALSE;

    if (pDialogElement == IMS_NULL)
    {
        IMS_TRACE_E(0, "Update : Root Dialog element is null", 0, 0, 0);
        return IMS_FALSE;
    }

    bUpdated |= UpdateDialog(pDialogElement);
    bUpdated |= UpdateDialogState(GetSubElement(pDialogElement, "state"));
    bUpdated |= UpdateDialogDuration(GetSubElement(pDialogElement, "duration"));
    bUpdated |= UpdateDialogReplaces(GetSubElement(pDialogElement, "replaces"));
    bUpdated |= UpdateDialogReferredBy(GetSubElement(pDialogElement, "referred-by"));
    bUpdated |= UpdateDialogLR(GetSubElement(pDialogElement, "local"), &m_stLocal);
    bUpdated |= UpdateDialogLR(GetSubElement(pDialogElement, "remote"), &m_stRemote);
    bUpdated |= UpdateDialogExtraInfo(pDialogElement);
    bUpdated |= UpdateOnHold(pDialogElement);
    bUpdated |= UpdateEnablePulled(pDialogElement);

    IMS_TRACE_I("Update : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_BOOL UCDialog::IsDialog(IN IElement* pDialogElement)
{
    IMS_BOOL bIs = IMS_FALSE;
    AString aStrID;

    if (pDialogElement == IMS_NULL)
    {
        IMS_TRACE_E(0, "IsDialog : Root Dialog element is null", 0, 0, 0);
        return IMS_FALSE;
    }

    aStrID = pDialogElement->GetAttribute("id");

    if (m_aStrID.Equals(aStrID))
    {
        bIs = IMS_TRUE;
    }

    IMS_TRACE_D("IsDialog : [%s][%s] [%s]", m_aStrID.GetStr(), aStrID.GetStr(), PS_BOOL(bIs));
    return bIs;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetID()
{
    IMS_TRACE_D("GetID : [%s]", m_aStrID.GetStr(), 0, 0);
    return m_aStrID;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetCallID()
{
    IMS_TRACE_D("GetCallID : [%s]", m_aStrCallID.GetStr(), 0, 0);
    return m_aStrCallID;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetLocalTag()
{
    IMS_TRACE_I("GetLocalTag : [%s]", m_aStrLocalTag.GetStr(), 0, 0);
    return m_aStrLocalTag;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetRemoteTag()
{
    IMS_TRACE_I("GetRemoteTag : [%s]", m_aStrRemoteTag.GetStr(), 0, 0);
    return m_aStrRemoteTag;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetDirection()
{
    IMS_TRACE_I("GetDirection : [%s]", m_aStrDirection.GetStr(), 0, 0);
    return m_aStrDirection;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_UINT32 UCDialog::GetState()
{
    IMS_TRACE_I("GetState : [%d]", m_eState, 0, 0);
    return m_eState;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetStateEvent()
{
    IMS_TRACE_I("GetStateEvent : [%s]", m_aStrState_Event.GetStr(), 0, 0);
    return m_aStrState_Event;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetStateCode()
{
    IMS_TRACE_I("GetStateCode : [%s]", m_aStrState_Code.GetStr(), 0, 0);
    return m_aStrState_Code;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_UINT32 UCDialog::GetDuration()
{
    IMS_TRACE_I("GetDuration : [%d]", m_nDuration, 0, 0);
    return m_nDuration;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetReferredBy()
{
    IMS_TRACE_D("GetReferredBy : [%s]", m_aStrReferredBy.GetStr(), 0, 0);
    return m_aStrReferredBy;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetReferredByDisplay()
{
    IMS_TRACE_D("GetReferredByDisplay : [%s]", m_aStrReferredBy_Display.GetStr(), 0, 0);
    return m_aStrReferredBy_Display;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCDialog::GetLocal(OUT DialogLR* pstDialogLR)
{
    pstDialogLR->aStrIdentity = m_stLocal.aStrIdentity;
    pstDialogLR->aStrIdentity_Display = m_stLocal.aStrIdentity_Display;
    pstDialogLR->aStrTarget_Uri = m_stLocal.aStrTarget_Uri;

    IMS_TRACE_D("GetLocal : Identity[%s][%s]", pstDialogLR->aStrIdentity.GetStr(),
            pstDialogLR->aStrTarget_Uri.GetStr(), 0);
    IMS_TRACE_D(
            "GetLocal : Identity_Display[%s]", pstDialogLR->aStrIdentity_Display.GetStr(), 0, 0);

    for (IMS_UINT32 index = 0; index < m_stLocal.objTargetParam.GetSize(); index++)
    {
        pstDialogLR->objTargetParam.Add(m_stLocal.objTargetParam.GetKeyAt(index),
                m_stLocal.objTargetParam.GetValueAt(index));
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetLocalIdentity()
{
    IMS_TRACE_D("GetLocalIdentity : [%s]", m_stLocal.aStrIdentity.GetStr(), 0, 0);
    return m_stLocal.aStrIdentity;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetLocalIdentityDisplay()
{
    IMS_TRACE_D("GetLocalIdentityDisplay : [%s]", m_stLocal.aStrIdentity_Display.GetStr(), 0, 0);
    return m_stLocal.aStrIdentity_Display;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetLocalpVal(IN AString aStrpName)
{
    AString aStrpVal = AString::ConstNull();
    IMS_SLONG nIndexKey = m_stLocal.objTargetParam.GetIndexOfKey(aStrpName);

    if (nIndexKey < 0)
    {
        return aStrpVal;
    }

    aStrpVal = m_stLocal.objTargetParam.GetValueAt(nIndexKey);

    IMS_TRACE_D("GetLocalpVal : [%s][%d][%s]", aStrpName.GetStr(), nIndexKey, aStrpVal.GetStr());
    return aStrpVal;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCDialog::GetRemote(OUT DialogLR* pstDialogLR)
{
    pstDialogLR->aStrIdentity = m_stRemote.aStrIdentity;
    pstDialogLR->aStrIdentity_Display = m_stRemote.aStrIdentity_Display;
    pstDialogLR->aStrTarget_Uri = m_stRemote.aStrTarget_Uri;

    IMS_TRACE_D("GetRemote : Identity[%s][%s]", pstDialogLR->aStrIdentity.GetStr(),
            pstDialogLR->aStrTarget_Uri.GetStr(), 0);
    IMS_TRACE_D(
            "GetRemote : Identity_Display[%s]", pstDialogLR->aStrIdentity_Display.GetStr(), 0, 0);

    for (IMS_UINT32 index = 0; index < m_stRemote.objTargetParam.GetSize(); index++)
    {
        pstDialogLR->objTargetParam.Add(m_stRemote.objTargetParam.GetKeyAt(index),
                m_stRemote.objTargetParam.GetValueAt(index));
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetRemoteIdentity()
{
    IMS_TRACE_D("GetRemoteIdentity : [%s]", m_stRemote.aStrIdentity.GetStr(), 0, 0);
    return m_stRemote.aStrIdentity;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetRemoteIdentityDisplay()
{
    IMS_TRACE_D("GetRemoteIdentityDisplay : [%s]", m_stRemote.aStrIdentity_Display.GetStr(), 0, 0);
    return m_stRemote.aStrIdentity_Display;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL AString UCDialog::GetRemotepVal(IN AString aStrpName)
{
    AString aStrpVal;
    IMS_SLONG nIndexKey = m_stRemote.objTargetParam.GetIndexOfKey(aStrpName);

    if (nIndexKey < 0)
    {
        return aStrpVal;
    }

    aStrpVal = m_stLocal.objTargetParam.GetValueAt(nIndexKey);

    IMS_TRACE_D("GetRemotepVal : [%s][%d][%s]", aStrpName.GetStr(), nIndexKey, aStrpVal.GetStr());
    return aStrpVal;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_BOOL UCDialog::EnablePulled()
{
    IMS_TRACE_I("EnablePulled : [%s]", PS_BOOL(m_bEnablePulled), 0, 0);
    return m_bEnablePulled;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialog(IN IElement* pDialogElement)
{
    IMS_BOOL bUpdated = IMS_FALSE;
    AString aStrDialog_ID = AString::ConstNull();
    AString aStrDialog_CallID = AString::ConstNull();
    AString aStrDialog_LocalTag = AString::ConstNull();
    AString aStrDialog_RemoteTag = AString::ConstNull();
    AString aStrDialog_Direction = AString::ConstNull();

    if (pDialogElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogState : pDialogElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    aStrDialog_ID = pDialogElement->GetAttribute("id");
    aStrDialog_CallID = pDialogElement->GetAttribute("call-id");
    aStrDialog_LocalTag = pDialogElement->GetAttribute("local-tag");
    aStrDialog_RemoteTag = pDialogElement->GetAttribute("remote-tag");
    aStrDialog_Direction = pDialogElement->GetAttribute("direction");

    IMS_TRACE_D("UpdateDialog : ID[%s][%s]", m_aStrID.GetStr(), aStrDialog_ID.GetStr(), 0);
    if (!m_aStrID.Equals(aStrDialog_ID))
    {
        m_aStrID = aStrDialog_ID;
        bUpdated = IMS_TRUE;
    }

    IMS_TRACE_D(
            "UpdateDialog : CallID[%s][%s]", m_aStrCallID.GetStr(), aStrDialog_CallID.GetStr(), 0);
    if (!m_aStrCallID.Equals(aStrDialog_CallID))
    {
        m_aStrCallID = aStrDialog_CallID;
        bUpdated = IMS_TRUE;
    }

    IMS_TRACE_D("UpdateDialog : LocalTag[%s][%s]", m_aStrLocalTag.GetStr(),
            aStrDialog_LocalTag.GetStr(), 0);
    if (!m_aStrLocalTag.Equals(aStrDialog_LocalTag))
    {
        m_aStrLocalTag = aStrDialog_LocalTag;
        bUpdated = IMS_TRUE;
    }

    IMS_TRACE_D("UpdateDialog : RemoteTag[%s][%s]", m_aStrRemoteTag.GetStr(),
            aStrDialog_RemoteTag.GetStr(), 0);
    if (!m_aStrRemoteTag.Equals(aStrDialog_RemoteTag))
    {
        m_aStrRemoteTag = aStrDialog_RemoteTag;
        bUpdated = IMS_TRUE;
    }

    IMS_TRACE_I("UpdateDialog : Direction[%s][%s]", m_aStrDirection.GetStr(),
            aStrDialog_Direction.GetStr(), 0);
    if (!m_aStrDirection.Equals(aStrDialog_Direction))
    {
        m_aStrDirection = aStrDialog_Direction;
        bUpdated = IMS_TRUE;
    }

    IMS_TRACE_I("UpdateDialog : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogState(IN IElement* pStateElement)
{
    if (pStateElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogState : pStateElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bUpdated = IMS_FALSE;
    AString aStrState = GetValueElement(pStateElement);
    AString aStrState_Event = pStateElement->GetAttribute("event");
    AString aStrState_Code = pStateElement->GetAttribute("code");

    if (!aStrState.IsEmpty() && !aStrState.IsNULL())
    {
        IMS_UINT32 eDialogState = ConvertDialogState(aStrState);
        IMS_TRACE_I("UpdateDialogState : [%d][%d]", m_eState, eDialogState, 0);
        if (m_eState != eDialogState)
        {
            m_eState = eDialogState;
            bUpdated = IMS_TRUE;
        }
    }

    if (!aStrState_Event.IsEmpty() && !aStrState_Event.IsNULL())
    {
        IMS_TRACE_I("UpdateDialogState : Event[%s][%s]", m_aStrState_Event.GetStr(),
                aStrState_Event.GetStr(), 0);
        if (!m_aStrState_Event.Equals(aStrState_Event))
        {
            m_aStrState_Event = aStrState_Event;
            bUpdated = IMS_TRUE;
        }
    }

    if (!aStrState_Code.IsEmpty() && !aStrState_Code.IsNULL())
    {
        IMS_TRACE_I("UpdateDialogState : Code[%s][%s]", m_aStrState_Code.GetStr(),
                aStrState_Code.GetStr(), 0);
        if (!m_aStrState_Code.Equals(aStrState_Code))
        {
            m_aStrState_Code = aStrState_Code;
            bUpdated = IMS_TRUE;
        }
    }

    IMS_TRACE_I("UpdateDialogState : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogDuration(IN IElement* pDurationElement)
{
    IMS_BOOL bUpdated = IMS_FALSE;
    AString aStrDuration;

    if (pDurationElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogDuration : pDurationElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    aStrDuration = GetValueElement(pDurationElement);

    if (!aStrDuration.IsEmpty() && !aStrDuration.IsNULL())
    {
        IMS_TRACE_I("UpdateDialogDuration : [%d][%d]", m_nDuration, aStrDuration.ToUInt32(), 0);
        if (m_nDuration != aStrDuration.ToUInt32())
        {
            m_nDuration = aStrDuration.ToUInt32();
            bUpdated = IMS_TRUE;
        }
    }

    IMS_TRACE_I("UpdateDialogDuration : [%s]", PS_BOOL(bUpdated), 0, 0);
    return IMS_FALSE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogReplaces(IN IElement* pReplacesElement)
{
    if (pReplacesElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogReplaces : pReplacesElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bUpdated = IMS_FALSE;
    AString aStrReplaces_CallID = pReplacesElement->GetAttribute("call-id");
    AString aStrReplaces_LocalTag = pReplacesElement->GetAttribute("local-tag");
    AString aStrReplaces_RemoteTag = pReplacesElement->GetAttribute("remote-tag");

    if (!aStrReplaces_CallID.IsEmpty() && !aStrReplaces_CallID.IsNULL())
    {
        IMS_TRACE_D("UpdateDialogReplaces : CallID[%s][%s]", m_aStrReplaces_CallID.GetStr(),
                aStrReplaces_CallID.GetStr(), 0);
        if (!m_aStrReplaces_CallID.Equals(aStrReplaces_CallID))
        {
            m_aStrReplaces_CallID = aStrReplaces_CallID;
            bUpdated = IMS_TRUE;
        }
    }

    if (!aStrReplaces_LocalTag.IsEmpty() && !aStrReplaces_LocalTag.IsNULL())
    {
        IMS_TRACE_D("UpdateDialogReplaces : LocalTag[%s][%s]", m_aStrReplaces_LocalTag.GetStr(),
                aStrReplaces_LocalTag.GetStr(), 0);
        if (!m_aStrReplaces_LocalTag.Equals(aStrReplaces_LocalTag))
        {
            m_aStrReplaces_LocalTag = aStrReplaces_LocalTag;
            bUpdated = IMS_TRUE;
        }
    }

    if (!aStrReplaces_RemoteTag.IsEmpty() && !aStrReplaces_RemoteTag.IsNULL())
    {
        IMS_TRACE_D("UpdateDialogReplaces : RemoteTag[%s][%s]", m_aStrReplaces_RemoteTag.GetStr(),
                aStrReplaces_RemoteTag.GetStr(), 0);
        if (!m_aStrReplaces_RemoteTag.Equals(aStrReplaces_RemoteTag))
        {
            m_aStrReplaces_RemoteTag = aStrReplaces_RemoteTag;
            bUpdated = IMS_TRUE;
        }
    }

    IMS_TRACE_I("UpdateDialogReplaces : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogReferredBy(IN IElement* pReferredByElement)
{
    if (pReferredByElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogReferredBy : pReferredByElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bUpdated = IMS_FALSE;
    AString aStrReferredBy = GetValueElement(pReferredByElement);
    AString aStrReferredBy_Display = pReferredByElement->GetAttribute("display");

    if (!aStrReferredBy.IsEmpty() && !aStrReferredBy.IsNULL())
    {
        IMS_TRACE_D("UpdateDialogReferredBy : value[%s][%s]", m_aStrReferredBy.GetStr(),
                aStrReferredBy.GetStr(), 0);
        if (!m_aStrReferredBy.Equals(aStrReferredBy))
        {
            m_aStrReferredBy = aStrReferredBy;
            bUpdated = IMS_TRUE;
        }
    }

    if (!aStrReferredBy_Display.IsEmpty() && !aStrReferredBy_Display.IsNULL())
    {
        IMS_TRACE_D("UpdateDialogReferredBy : Display[%s][%s]", m_aStrReferredBy_Display.GetStr(),
                aStrReferredBy_Display.GetStr(), 0);
        if (!m_aStrReferredBy_Display.Equals(aStrReferredBy_Display))
        {
            m_aStrReferredBy_Display = aStrReferredBy_Display;
            bUpdated = IMS_TRUE;
        }
    }

    IMS_TRACE_I("UpdateDialogReferredBy : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogLR(
        IN IElement* pLRElement, IN DialogLR* pstDialogLR)
{
    IMS_BOOL bUpdated = IMS_FALSE;

    if (pLRElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogLR : pLRElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IElement* pIdentityElement = GetSubElement(pLRElement, "identity");
    if (pIdentityElement != IMS_NULL)
    {
        bUpdated |= UpdateDialogLRIdentity(pIdentityElement, pstDialogLR);
    }

    IElement* pTergetElement = GetSubElement(pLRElement, "target");
    if (pTergetElement != IMS_NULL)
    {
        bUpdated |= UpdateDialogLRTarget(pTergetElement, pstDialogLR);
    }

    IMS_TRACE_I("UpdateDialogLR : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogLRIdentity(
        IN IElement* pIdentityElement, IN DialogLR* pstDialogLR)
{
    if (pIdentityElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogLRIdentity : pIdentityElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bUpdated = IMS_FALSE;
    AString aStrIdentity = GetValueElement(pIdentityElement);
    AString aStrIdentity_Display = pIdentityElement->GetAttribute("display");

    if (aStrIdentity_Display.IsEmpty() || aStrIdentity_Display.IsNULL())
    {
        aStrIdentity_Display = pIdentityElement->GetAttribute("display-name");
    }

    if (!aStrIdentity.IsEmpty() && !aStrIdentity.IsNULL())
    {
        IMS_TRACE_D("UpdateDialogLRIdentity : value[%s][%s]", pstDialogLR->aStrIdentity.GetStr(),
                aStrIdentity.GetStr(), 0);
        if (!pstDialogLR->aStrIdentity.Equals(aStrIdentity))
        {
            pstDialogLR->aStrIdentity = aStrIdentity;
            bUpdated = IMS_TRUE;
        }
    }

    if (!aStrIdentity_Display.IsEmpty() && !aStrIdentity_Display.IsNULL())
    {
        IMS_TRACE_D("UpdateDialogLRIdentity : Display[%s][%s]",
                pstDialogLR->aStrIdentity_Display.GetStr(), aStrIdentity_Display.GetStr(), 0);
        if (!pstDialogLR->aStrIdentity_Display.Equals(aStrIdentity_Display))
        {
            pstDialogLR->aStrIdentity_Display = aStrIdentity_Display;
            bUpdated = IMS_TRUE;
        }
    }

    IMS_TRACE_I("UpdateDialogLRIdentity : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogLRTarget(
        IN IElement* pTargetElement, IN DialogLR* pstDialogLR)
{
    IMS_BOOL bUpdated = IMS_FALSE;
    AString aStrTarget_Uri = AString::ConstNull();

    IMS_TRACE_I("UpdateDialogLRTarget", 0, 0, 0);

    if (pTargetElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogLRTarget : pTargetElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    aStrTarget_Uri = pTargetElement->GetAttribute("uri");

    if (!aStrTarget_Uri.IsEmpty() && !aStrTarget_Uri.IsNULL())
    {
        IMS_TRACE_D("UpdateDialogLRTarget : URI[%s][%s]", pstDialogLR->aStrTarget_Uri.GetStr(),
                aStrTarget_Uri.GetStr(), 0);
        if (!pstDialogLR->aStrTarget_Uri.Equals(aStrTarget_Uri))
        {
            pstDialogLR->aStrTarget_Uri = aStrTarget_Uri;
            bUpdated = IMS_TRUE;
        }
    }

    bUpdated |= UpdateDialogLRTarget_Param(pTargetElement, pstDialogLR);

    IMS_TRACE_I("UpdateDialogLRTarget : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogLRTarget_Param(
        IN IElement* pTargetElement, IN DialogLR* pstDialogLR)
{
    IMS_BOOL bUpdated = IMS_FALSE;

    if (pTargetElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogLRTarget_Param : pTargetElement is null", 0, 0, 0);
        return IMS_FALSE;
    }

    INodeList* pINodeList = pTargetElement->GetElementsByTagName("param");
    if (pINodeList == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogLRTarget_Param : piNodeList is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 nNode = pINodeList->GetLength();

    for (IMS_UINT32 index = 0; index < nNode; index++)
    {
        INode* pINode = pINodeList->Item(index);
        if (pINode == IMS_NULL)
        {
            continue;
        }

        IElement* pParamElement = DYNAMIC_CAST(IElement*, pINode);
        AString aStrParam_name = pParamElement->GetAttribute("pname");
        AString aStrParam_value = pParamElement->GetAttribute("pval");

        AString aStrpVal;
        IMS_SLONG nIndexKey = m_stRemote.objTargetParam.GetIndexOfKey(aStrParam_name);

        if (nIndexKey >= 0)
        {
            aStrpVal = pstDialogLR->objTargetParam.GetValueAt(nIndexKey);
        }

        if (!aStrpVal.IsNULL())
        {
            if (!aStrpVal.Equals(aStrParam_value))
            {
                pstDialogLR->objTargetParam.Remove(aStrParam_name);
                pstDialogLR->objTargetParam.Add(aStrParam_name, aStrParam_value);
                bUpdated = IMS_TRUE;
                IMS_TRACE_D("UpdateDialogLRTarget_Param : UPDATED [%d][%s][%s]", index,
                        aStrParam_name.GetStr(), aStrParam_value.GetStr());
            }
        }
        else
        {
            pstDialogLR->objTargetParam.Add(aStrParam_name, aStrParam_value);
            bUpdated = IMS_TRUE;
            IMS_TRACE_D("UpdateDialogLRTarget_Param : ADDED [%d][%s][%s]", index,
                    aStrParam_name.GetStr(), aStrParam_value.GetStr());
        }
    }

    pTargetElement->DestroyNodeList(pINodeList);

    IMS_TRACE_I("UpdateDialogLRTarget_Param : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateEnablePulled(IN IElement* pDialogElement)
{
    IMS_BOOL bUpdated = IMS_FALSE;

    if (pDialogElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateEnablePulled : DialogElement is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("UpdateEnablePulled : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateOnHold(IN IElement* pDialogElement)
{
    IMS_BOOL bIs = IMS_FALSE;

    if (pDialogElement == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_eState != DIALOG_STATE_CONFIRMED)
    {
        IMS_TRACE_I("UpdateOnHold : State[%d] ", m_eState, 0, 0);
        return IMS_FALSE;
    }

    if (bIs)
    {
        m_eState = DIALOG_STATE_ONHOLD;
    }

    IMS_TRACE_I("UpdateOnHold : State[%d] [%s]", m_eState, PS_BOOL(bIs), 0);
    return bIs;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCDialog::UpdateDialogExtraInfo(IN IElement* pDialogElement)
{
    IMS_BOOL bUpdated = IMS_FALSE;

    if (pDialogElement == IMS_NULL)
    {
        IMS_TRACE_I("UpdateDialogExtraInfo : DialogElement is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("UpdateDialogExtraInfo : [%s]", PS_BOOL(bUpdated), 0, 0);
    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
IMS_UINT32 UCDialog::ConvertDialogState(IN AString aStrState)
{
    IMS_UINT32 nState = DIALOG_STATE_IDLE;

    if (aStrState.EqualsIgnoreCase("trying"))
    {
        nState = DIALOG_STATE_TRYING;
    }
    else if (aStrState.EqualsIgnoreCase("proceeding"))
    {
        nState = DIALOG_STATE_PROCEEDING;
    }
    else if (aStrState.EqualsIgnoreCase("early"))
    {
        nState = DIALOG_STATE_EARLY;
    }
    else if (aStrState.EqualsIgnoreCase("confirmed"))
    {
        nState = DIALOG_STATE_CONFIRMED;
    }
    else if (aStrState.EqualsIgnoreCase("terminated"))
    {
        nState = DIALOG_STATE_TERMINATED;
    }

    IMS_TRACE_I("ConvertDialogState : [%s] [%d]", aStrState.GetStr(), nState, 0);
    return nState;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
AString UCDialog::GetValueElement(IN IElement* pIElement)
{
    INode* piNode = DYNAMIC_CAST(INode*, pIElement);
    if (piNode == IMS_NULL)
    {
        IMS_TRACE_I("GetValueElement : piNode is NULL", 0, 0, 0);
        return AString::ConstNull();
    }

    INode* piNodeChild = piNode->GetFirstChild();
    if (piNodeChild == IMS_NULL)
    {
        IMS_TRACE_I("GetValueElement : piNodeChild is NULL", 0, 0, 0);
        return AString::ConstNull();
    }

    if (piNodeChild->GetNodeType() != INode::TEXT_NODE)
    {
        IMS_TRACE_I("GetValueElement : GetNodeType is not TEXT_NODE", 0, 0, 0);
        return AString::ConstNull();
    }

    IText* piText = DYNAMIC_CAST(IText*, piNodeChild);
    AString aStrValue = piText->GetData();

    IMS_TRACE_D("GetValueElement : [%s]", aStrValue.GetStr(), 0, 0);
    return aStrValue;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
AString UCDialog::GetValueSubElement(IN IElement* pIElement, IN const IMS_CHAR* strSubElement)
{
    INodeList* pINodeList = pIElement->GetElementsByTagName(strSubElement);
    if (pINodeList == IMS_NULL)
    {
        IMS_TRACE_I("GetValueSubElement : piNodeList is NULL", 0, 0, 0);
        return AString::ConstNull();
    }

    INode* pINode = pINodeList->Item(0);
    if (pINode == IMS_NULL)
    {
        pIElement->DestroyNodeList(pINodeList);
        IMS_TRACE_I("GetValueSubElement : piNode is NULL", 0, 0, 0);
        return AString::ConstNull();
    }

    INode* piNodeChild = pINode->GetFirstChild();
    if (piNodeChild == IMS_NULL)
    {
        pIElement->DestroyNodeList(pINodeList);
        IMS_TRACE_I("GetValueSubElement : piNodeChild is NULL", 0, 0, 0);
        return AString::ConstNull();
    }

    if (piNodeChild->GetNodeType() != INode::TEXT_NODE)
    {
        pIElement->DestroyNodeList(pINodeList);
        IMS_TRACE_I("GetValueSubElement : GetNodeType is not TEXT_NODE", 0, 0, 0);
        return AString::ConstNull();
    }

    IText* pIText = DYNAMIC_CAST(IText*, piNodeChild);
    AString aStrValue = pIText->GetData();

    pIElement->DestroyNodeList(pINodeList);

    IMS_TRACE_D("GetValueSubElement : [%s][%s]", strSubElement, aStrValue.GetStr(), 0);
    return aStrValue;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
IElement* UCDialog::GetSubElement(
        IN IElement* pIElement, IN const IMS_CHAR* strSubElement, IN IMS_UINT32 nIndex /* = 0 */)
{
    INodeList* pINodeList = pIElement->GetElementsByTagName(strSubElement);
    if (pINodeList == IMS_NULL)
    {
        IMS_TRACE_I("GetSubElement : piNodeList is NULL", 0, 0, 0);
        return IMS_NULL;
    }

    INode* pINode = pINodeList->Item(nIndex);
    if (pINode == IMS_NULL)
    {
        IMS_TRACE_I("GetSubElement : piNode is NULL", 0, 0, 0);
        pIElement->DestroyNodeList(pINodeList);
        return IMS_NULL;
    }

    IElement* piElementUsers = DYNAMIC_CAST(IElement*, pINode);
    pIElement->DestroyNodeList(pINodeList);

    IMS_TRACE_I("GetSubElement : [%d][%s]", nIndex, strSubElement, 0);

    return piElementUsers;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
IMSList<IElement*> UCDialog::GetSubElementList(
        IN IElement* pIElement, IN const IMS_CHAR* strSubElement)
{
    IMSList<IElement*> lstElements;

    INodeList* pINodeList = pIElement->GetElementsByTagName(strSubElement);
    if (pINodeList == IMS_NULL)
    {
        IMS_TRACE_I("GetSubElementList : piNodeList is NULL", 0, 0, 0);
        return IMSList<IElement*>();
    }

    for (IMS_SINT32 index = 0; index < pINodeList->GetLength(); index++)
    {
        INode* pINode = pINodeList->Item(index);
        if (pINode == IMS_NULL)
        {
            continue;
        }
        IElement* pParamElement = DYNAMIC_CAST(IElement*, pINode);
        lstElements.Append(pParamElement);
    }

    pIElement->DestroyNodeList(pINodeList);

    IMS_TRACE_I("GetSubElementList : [%s] [%d]", strSubElement, lstElements.GetSize(), 0);
    return lstElements;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
AString UCDialog::GetAttributeFromElement(IN IElement* pIElement, IN const IMS_CHAR* strAttribute)
{
    AString aStrAttribute;

    if (pIElement == IMS_NULL)
    {
        IMS_TRACE_I("GetAttributeFromElement : pIElement is null", 0, 0, 0);
        return aStrAttribute;
    }

    aStrAttribute = pIElement->GetAttribute(strAttribute);

    IMS_TRACE_D("GetAttributeFromElement : [%s]", aStrAttribute.GetStr(), 0, 0);
    return aStrAttribute;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCDialog::LoadConfig() {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCDialog::AddEventListn() {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCDialog::DeleteEventListn() {}
