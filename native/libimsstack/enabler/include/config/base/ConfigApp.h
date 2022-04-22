/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170717  hwangoo.park@             Changed from AppConfigurableParameterManager
    </table>

    Description

*/

#ifndef _CONFIG_APP_H_
#define _CONFIG_APP_H_

#include "ImsMessageDef.h"
#include "IMSApp.h"
#include "IEventListener.h"

class ConfigApp
    : public IMSApp
    , public IEventListener
{
public:
    ConfigApp(IN const AString &strAppName);
    virtual ~ConfigApp();

public:
    void Start();

protected:
    // IMSApp class
    virtual IMS_BOOL OnPreprocess(IN IMSMSG &objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG &objMSG);
    virtual IMS_BOOL OnPostprocess(IN IMSMSG &objMSG);

    // IEventListener class
    virtual void Event_NotifyEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam,
            IN IMS_UINT32 nLParam);

    virtual void UpdateAllForHidden(IN IMS_SINT32 nItem, IN IMS_SINT32 nParam);
    virtual void UpdateAllForDM(IN IMS_SINT32 nItem, IN IMS_SINT32 nParam);
    virtual void UpdateItemForPST(IN IMS_UINT32 nItem, IN IMS_UINT32 nValue);
    virtual void UpdateItemForSDM(IN IMS_UINT32 nSDMI, IN IMS_UINT32 nValue);

    IMS_BOOL UpdateSipConifgV(IN IMS_SINT32 nCPI,
            IN const AString &strServiceId = AString::ConstNull());
    IMS_BOOL UpdateSubscriberConfig(IN IMS_SINT32 nCPI);

protected:
    // Internal messages
    enum
    {
        AMSG_START = (IMS_MSG_USER + 1),
    };
};

#endif // _CONFIG_APP_H_
