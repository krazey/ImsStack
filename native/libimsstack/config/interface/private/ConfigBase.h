/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091026  toastops@                 Created
    </table>

    Description

*/

#ifndef _CONFIG_BASE_H_
#define _CONFIG_BASE_H_

#include "IMSList.h"
#include "IMSMap.h"
#include "ImsSlot.h"

#include "ICarrierConfig.h"
#include "IConfigurable.h"

class IImsPrivateProperty;

class IConfigBuffer;

class ConfigBase
    : public ImsSlot
    , public ICarrierConfigListener
{
public:
    explicit ConfigBase(IN IMS_SINT32 nSlotId_);
    virtual ~ConfigBase() = 0;

public:
    virtual IMS_BOOL Init();
    virtual void Refresh();

    IMS_BOOL Load(IN const AString &strConfName = AString::ConstNull());
    IMS_BOOL Store(IN const AString &strConfName = AString::ConstNull());

protected:
    virtual IMS_BOOL ReadFrom();
    virtual IMS_BOOL WriteTo();
    virtual IMS_BOOL ReadFrom(IN const AString &strConfName);
    virtual IMS_BOOL WriteTo(IN const AString &strConfName);
    virtual IMS_BOOL Update(IN IMS_SINT32 nCPI,
            IN const AString &strValue = AString::ConstNull());
    virtual void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId);

    IMS_BOOL AddListener(IN IMS_SINT32 nCPI, IN IConfigUpdateListener *piListener);
    void RemoveListener(IN IMS_SINT32 nCPI, IN IConfigUpdateListener *piListener);
    IMS_BOOL NotifyUpdate(IN IMS_SINT32 nCPI,
            IN const AString &strConfName = AString::ConstNull(),
            IN const AString &strExtraParam = AString::ConstNull());

    IConfigBuffer* GetConfigBufferFromContent(IN const AString& strContent) const;

    ICarrierConfig* GetCarrierConfig();
    IImsPrivateProperty* GetPrivateProperty();

protected:
    class Configurable
        : public IConfigurable
    {
    public:
        inline Configurable(IN ConfigBase *pConfig_)
            : pConfig(pConfig_)
        {}
        inline virtual ~Configurable()
        {}

    public:
        // IConfigurable class
        inline virtual IMS_BOOL AddListener(IN IMS_SINT32 nCPI, IN IConfigUpdateListener *piListener)
        {
            if (pConfig == IMS_NULL)
            {
                return IMS_FALSE;
            }

            return pConfig->AddListener(nCPI, piListener);
        }

        inline virtual void RemoveListener(IN IMS_SINT32 nCPI, IN IConfigUpdateListener *piListener)
        {
            if (pConfig == IMS_NULL)
            {
                return;
            }

            pConfig->RemoveListener(nCPI, piListener);
        }

        inline virtual IMS_BOOL Update(IN IMS_SINT32 nCPI,
                IN const AString &strValue = AString::ConstNull())
        {
            if (pConfig == IMS_NULL)
            {
                return IMS_FALSE;
            }

            return pConfig->Update(nCPI, strValue);
        }

    private:
        ConfigBase *pConfig;
    };

public:
    static const IMS_CHAR SECTION_UNIQUENESS[];

    // Listener for configuration update notification
    IMSMap< IMS_SINT32, IMSList<IConfigUpdateListener*> > objConfigUpdateListeners;
};

#endif // _CONFIG_BASE_H_
