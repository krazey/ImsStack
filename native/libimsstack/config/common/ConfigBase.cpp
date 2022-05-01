/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091026  toastops@                 Created
    </table>

    Description

*/
#include "ServiceConfig.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "ConfigLoader.h"
#include "IConfigUpdateListener.h"
#include "private/ConfigBase.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC GLOBAL
const IMS_CHAR ConfigBase::SECTION_UNIQUENESS[] = "Uniqueness";

PUBLIC
ConfigBase::ConfigBase(IN IMS_SINT32 nSlotId_)
    : ImsSlot(nSlotId_)
    , objConfigUpdateListeners(IMSMap< IMS_SINT32, IMSList<IConfigUpdateListener*> >())
{
}

PUBLIC VIRTUAL
ConfigBase::~ConfigBase()
{
}

PROTECTED VIRTUAL
IMS_BOOL ConfigBase::Init()
{
    return Load();
}

PUBLIC VIRTUAL
void ConfigBase::Refresh()
{
    // no-op
}

PUBLIC
IMS_BOOL ConfigBase::Load(IN const AString &strConfName /* = AString::ConstNull() */)
{
    // Read the configuration from the default medium
    if (strConfName.GetLength() == 0)
    {
        return ReadFrom();
    }

    return ReadFrom(strConfName);
}

PUBLIC
IMS_BOOL ConfigBase::Store(IN const AString &strConfName /* = AString::ConstNull() */)
{
    // Write the configuration from the default medium
    if (strConfName.GetLength() == 0)
    {
        return WriteTo();
    }

    return WriteTo(strConfName);
}

PROTECTED VIRTUAL
IMS_BOOL ConfigBase::ReadFrom()
{
    // The subclass MUST implement if it has a basic configuration information.
    return IMS_FALSE;
}

PROTECTED VIRTUAL
IMS_BOOL ConfigBase::WriteTo()
{
    // The subclass MUST implement if it has a basic configuration information.
    return IMS_FALSE;
}

PROTECTED VIRTUAL
IMS_BOOL ConfigBase::ReadFrom(IN const AString & /* strConfName */)
{
    // The subclass MUST implement if it has an application/service-specific
    // configuration information.
    return IMS_FALSE;
}

PROTECTED VIRTUAL
IMS_BOOL ConfigBase::WriteTo(IN const AString & /* strConfName */)
{
    // The subclass MUST implement if it has an application/service-specific
    // configuration information.
    return IMS_FALSE;
}

PROTECTED VIRTUAL
IMS_BOOL ConfigBase::Update(IN IMS_SINT32 /* nCPI */,
        IN const AString & /* strValue = AString::ConstNull() */)
{
    // The subclass MUST implement if it has the configurable items.
    return IMS_FALSE;
}

PROTECTED VIRTUAL
void ConfigBase::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 /*nSlotId*/)
{
    // No-op
}

PROTECTED
IMS_BOOL ConfigBase::AddListener(IN IMS_SINT32 nCPI, IN IConfigUpdateListener *piListener)
{
    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SLONG nIndex = objConfigUpdateListeners.GetIndexOfKey(nCPI);

    if (nIndex < 0)
    {
        IMSList<IConfigUpdateListener*> objListeners;

        objListeners.Append(piListener);

        if (!objConfigUpdateListeners.Add(nCPI, objListeners))
        {
            return IMS_FALSE;
        }

        IMS_TRACE_D("ConfigUpdateListener :: add - %d / %p",
                objConfigUpdateListeners.GetSize(), piListener, 0);

        return IMS_TRUE;
    }

    IMSList<IConfigUpdateListener*> &objListeners = objConfigUpdateListeners.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IConfigUpdateListener *piTmpListener = objListeners.GetAt(i);

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

    IMS_TRACE_D("ConfigUpdateListener :: add - %d / %p / %d",
            objConfigUpdateListeners.GetSize(), piListener, objListeners.GetSize());

    return objListeners.Append(piListener);
}

PROTECTED
void ConfigBase::RemoveListener(IN IMS_SINT32 nCPI, IN IConfigUpdateListener *piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    IMS_SLONG nIndex = objConfigUpdateListeners.GetIndexOfKey(nCPI);

    if (nIndex < 0)
    {
        return;
    }

    IMSList<IConfigUpdateListener*> &objListeners = objConfigUpdateListeners.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IConfigUpdateListener *piTmpListener = objListeners.GetAt(i);

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
        objConfigUpdateListeners.RemoveAt(nIndex);
    }

    IMS_TRACE_D("ConfigUpdateListener :: remove - %d / %p / %d",
            objConfigUpdateListeners.GetSize(), piListener, objListeners.GetSize());
}

PROTECTED
IMS_BOOL ConfigBase::NotifyUpdate(IN IMS_SINT32 nCPI,
        IN const AString &strConfName /* = AString::ConstNull() */,
        IN const AString &strExtraParam /* = AString::ConstNull() */)
{
    IMS_SLONG nIndex = objConfigUpdateListeners.GetIndexOfKey(nCPI);

    if (nIndex < 0)
    {
        return IMS_FALSE;
    }

    const IMSList<IConfigUpdateListener*> &objListeners
            = objConfigUpdateListeners.GetValueAt(nIndex);

    if (objListeners.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IConfigUpdateListener *piListener = objListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        piListener->ConfigUpdate_NotifyUpdate(nCPI, strConfName, strExtraParam);
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
