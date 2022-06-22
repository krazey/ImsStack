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
#include "ServiceConfig.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "ConfigLoader.h"
#include "IConfigUpdateListener.h"
#include "private/ConfigBase.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC GLOBAL const IMS_CHAR ConfigBase::SECTION_UNIQUENESS[] = "Uniqueness";

PUBLIC
ConfigBase::ConfigBase(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_objConfigUpdateListeners(IMSMap<IMS_SINT32, IMSList<IConfigUpdateListener*>>())
{
}

PUBLIC VIRTUAL ConfigBase::~ConfigBase() {}

PUBLIC
IMS_BOOL ConfigBase::Load(IN const AString& strConfName /*= AString::ConstNull()*/)
{
    // Read the configuration from the default medium
    if (strConfName.GetLength() == 0)
    {
        return ReadFrom();
    }

    return ReadFrom(strConfName);
}

PUBLIC
IMS_BOOL ConfigBase::Store(IN const AString& strConfName /*= AString::ConstNull()*/)
{
    // Write the configuration from the default medium
    if (strConfName.GetLength() == 0)
    {
        return WriteTo();
    }

    return WriteTo(strConfName);
}

PROTECTED VIRTUAL IMS_BOOL ConfigBase::ReadFrom()
{
    // The subclass MUST implement if it has a basic configuration information.
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL ConfigBase::WriteTo()
{
    // The subclass MUST implement if it has a basic configuration information.
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL ConfigBase::ReadFrom(IN const AString& /*strConfName*/)
{
    // The subclass MUST implement if it has an application/service-specific
    // configuration information.
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL ConfigBase::WriteTo(IN const AString& /*strConfName*/)
{
    // The subclass MUST implement if it has an application/service-specific
    // configuration information.
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL ConfigBase::Update(
        IN IMS_SINT32 /*nCpi*/, IN const AString& /*strValue = AString::ConstNull()*/)
{
    // The subclass MUST implement if it has the configurable items.
    return IMS_FALSE;
}

PROTECTED
IMS_BOOL ConfigBase::AddListener(IN IMS_SINT32 nCpi, IN IConfigUpdateListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SLONG nIndex = m_objConfigUpdateListeners.GetIndexOfKey(nCpi);

    if (nIndex < 0)
    {
        IMSList<IConfigUpdateListener*> objListeners;

        objListeners.Append(piListener);

        if (!m_objConfigUpdateListeners.Add(nCpi, objListeners))
        {
            return IMS_FALSE;
        }

        IMS_TRACE_D("ConfigUpdateListener :: add - %d / %p", m_objConfigUpdateListeners.GetSize(),
                piListener, 0);

        return IMS_TRUE;
    }

    IMSList<IConfigUpdateListener*>& objListeners = m_objConfigUpdateListeners.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IConfigUpdateListener* piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == IMS_NULL)
        {
            continue;
        }

        if (piListener == piTmpListener)
        {
            // The listener is already registered
            return IMS_TRUE;
        }
    }

    IMS_TRACE_D("ConfigUpdateListener :: add - %d / %p / %d", m_objConfigUpdateListeners.GetSize(),
            piListener, objListeners.GetSize());

    return objListeners.Append(piListener);
}

PROTECTED
void ConfigBase::RemoveListener(IN IMS_SINT32 nCpi, IN IConfigUpdateListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    IMS_SLONG nIndex = m_objConfigUpdateListeners.GetIndexOfKey(nCpi);

    if (nIndex < 0)
    {
        return;
    }

    IMSList<IConfigUpdateListener*>& objListeners = m_objConfigUpdateListeners.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IConfigUpdateListener* piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == IMS_NULL)
        {
            continue;
        }

        if (piListener == piTmpListener)
        {
            objListeners.RemoveAt(i);
            break;
        }
    }

    if (objListeners.IsEmpty())
    {
        m_objConfigUpdateListeners.RemoveAt(nIndex);
    }

    IMS_TRACE_D("ConfigUpdateListener :: remove - %d / %p / %d",
            m_objConfigUpdateListeners.GetSize(), piListener, objListeners.GetSize());
}

PROTECTED
IMS_BOOL ConfigBase::NotifyUpdate(IN IMS_SINT32 nCpi,
        IN const AString& strConfName /*= AString::ConstNull()*/,
        IN const AString& strExtraParam /*= AString::ConstNull()*/)
{
    IMS_SLONG nIndex = m_objConfigUpdateListeners.GetIndexOfKey(nCpi);

    if (nIndex < 0)
    {
        return IMS_FALSE;
    }

    const IMSList<IConfigUpdateListener*>& objListeners =
            m_objConfigUpdateListeners.GetValueAt(nIndex);

    if (objListeners.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IConfigUpdateListener* piListener = objListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        piListener->ConfigUpdate_NotifyUpdate(nCpi, strConfName, strExtraParam);
    }

    return IMS_TRUE;
}

PROTECTED
IConfigBuffer* ConfigBase::GetConfigBufferFromContent(IN const AString& strContent) const
{
    return ConfigLoader::GetConfig(strContent);
}

PROTECTED
ICarrierConfig* ConfigBase::GetCarrierConfig()
{
    return ConfigService::GetConfigService()->GetCarrierConfig(GetSlotId());
}

PROTECTED
IImsPrivateProperty* ConfigBase::GetPrivateProperty()
{
    return UtilService::GetUtilService()->GetPrivateProperty();
}
