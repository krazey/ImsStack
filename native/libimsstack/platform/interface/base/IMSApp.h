/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IMS_APP_H_
#define _IMS_APP_H_

#include "IMSActivity.h"
#include "IMSService.h"

class IMSApp : public IMSActivity
{
public:
    IMSApp(IN CONST AString& strAppName);
    virtual ~IMSApp();

    IMS_BOOL AttachService(IN IMSService* pIMSService);
    void DetachService(IN IMSService* pIMSService);
    IMSService* GetService(IN CONST AString& strServiceName);
    inline const IMSList<IMSService*> GetServices() { return objIMSServices; }

protected:
    virtual IMS_BOOL OnPreprocess(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnPostprocess(IN IMSMSG& objMSG);
    virtual IIMSActivityControl* GetController();

private:
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

private:
    IMSList<IMSService*> objIMSServices;
};

// Definition of function pointer to create the IMSApp derived classes
typedef IMSApp* (*IMSApp_Creator)(IN const AString& strName);

#endif  // _IMS_APP_H_
