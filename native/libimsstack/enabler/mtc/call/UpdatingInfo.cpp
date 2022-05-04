#include "call/UpdatingInfo.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UpdatingInfo::UpdatingInfo() :
        m_objNegotiatedInfo(MediaInfo()),
        m_objModifyingInfo(MediaInfo()),
        m_objAlertingInfo(MediaInfo()),
        m_objModifiedInfo(MediaInfo()),
        m_bModifier(IMS_FALSE),
        m_bAlerted(IMS_FALSE)
{
    IMS_TRACE_D("+UpdatingInfo", 0, 0, 0);
}

PUBLIC VIRTUAL UpdatingInfo::~UpdatingInfo()
{
    IMS_TRACE_D("~UpdatingInfo", 0, 0, 0);
}

PUBLIC
IMS_BOOL UpdatingInfo::IsHeld()
{
    if (m_objNegotiatedInfo.eADir == DIRECTION_SEND_RECEIVE)
    {
        if (m_objModifyingInfo.eADir == DIRECTION_SEND ||
                m_objModifyingInfo.eADir == DIRECTION_INACTIVE)
        {
            return IMS_TRUE;
        }
    }
    else if (m_objNegotiatedInfo.eADir == DIRECTION_RECEIVE &&
            m_objModifyingInfo.eADir == DIRECTION_INACTIVE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsHeldBy()
{
    IMS_SINT32 eNewADir = m_objModifiedInfo.eADir;
    if (eNewADir == DIRECTION_INVALID)
    {
        eNewADir = m_objAlertingInfo.eADir;
    }

    if (m_objNegotiatedInfo.eADir == DIRECTION_SEND_RECEIVE)
    {
        if (eNewADir == DIRECTION_RECEIVE)
        {
            return IMS_TRUE;
        }
        else if (eNewADir == DIRECTION_INACTIVE && m_objModifyingInfo.eADir != DIRECTION_INACTIVE)
        {
            return IMS_TRUE;
        }
    }
    else if (m_objNegotiatedInfo.eADir == DIRECTION_SEND && eNewADir == DIRECTION_INACTIVE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsResumed()
{
    if (m_objNegotiatedInfo.eADir == DIRECTION_SEND &&
            m_objModifyingInfo.eADir == DIRECTION_SEND_RECEIVE)
    {
        return IMS_TRUE;
    }
    else if (m_objNegotiatedInfo.eADir == DIRECTION_INACTIVE)
    {
        if (m_objModifyingInfo.eADir == DIRECTION_RECEIVE ||
                m_objModifyingInfo.eADir == DIRECTION_SEND_RECEIVE)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsResumedBy()
{
    IMS_SINT32 eNewADir = m_objModifiedInfo.eADir;
    if (eNewADir == DIRECTION_INVALID)
    {
        eNewADir = m_objAlertingInfo.eADir;
    }

    if (m_objNegotiatedInfo.eADir == DIRECTION_RECEIVE && eNewADir == DIRECTION_SEND_RECEIVE)
    {
        return IMS_TRUE;
    }
    else if (m_objNegotiatedInfo.eADir == DIRECTION_INACTIVE)
    {
        if (eNewADir == DIRECTION_SEND)
        {
            return IMS_TRUE;
        }
        else if (eNewADir == DIRECTION_SEND_RECEIVE &&
                m_objModifyingInfo.eADir != DIRECTION_SEND_RECEIVE)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsNeedToAlert()
{
    if (IsHeldBy() || IsResumedBy())
    {
        return IMS_FALSE;
    }

    if (m_objNegotiatedInfo.eVDir != m_objAlertingInfo.eVDir ||
            m_objNegotiatedInfo.eTDir != m_objAlertingInfo.eTDir)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsRequestedHoldResume()
{
    if (m_objModifyingInfo.eADir == DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    if (m_objNegotiatedInfo.eADir == m_objModifyingInfo.eADir)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsRequestedModifying()
{
    if (m_objModifyingInfo.eADir == DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    if (m_objNegotiatedInfo.eADir != m_objModifyingInfo.eADir)
    {
        return IMS_FALSE;
    }

    if (m_objNegotiatedInfo.eVDir != m_objModifyingInfo.eVDir ||
            m_objNegotiatedInfo.eTDir != m_objModifyingInfo.eTDir)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsModified()
{
    if (m_objNegotiatedInfo.eVDir != m_objModifiedInfo.eVDir &&
            (m_objNegotiatedInfo.eVDir == DIRECTION_INVALID ||
                    m_objModifiedInfo.eVDir == DIRECTION_INVALID))
    {
        return IMS_TRUE;
    }

    if (m_objNegotiatedInfo.eTDir != m_objModifiedInfo.eTDir &&
            (m_objNegotiatedInfo.eTDir == DIRECTION_INVALID ||
                    m_objModifiedInfo.eTDir == DIRECTION_INVALID))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
