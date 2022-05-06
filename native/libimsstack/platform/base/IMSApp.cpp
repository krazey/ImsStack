/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  joonhun.shin@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "IMSApp.h"

PUBLIC
IMSApp::IMSApp(IN CONST AString& strName) :
        IMSActivity(strName),
        objIMSServices(IMSList<IMSService*>())
{
}

PUBLIC VIRTUAL IMSApp::~IMSApp()
{
    while (!objIMSServices.IsEmpty())
    {
        IMSService* pService = objIMSServices.GetAt(0);
        if (pService != IMS_NULL)
        {
            delete pService;
        }
        objIMSServices.RemoveAt(0);
    }
}

PUBLIC
IMS_BOOL IMSApp::AttachService(IN IMSService* pIMSService)
{
    if (pIMSService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return objIMSServices.Append(pIMSService);
}

PUBLIC
void IMSApp::DetachService(IN IMSService* pIMSService)
{
    if (pIMSService == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objIMSServices.GetSize(); i++)
    {
        IMSService* pService = objIMSServices.GetAt(i);

        if (pService == pIMSService)
        {
            objIMSServices.RemoveAt(i);
            break;
        }
    }
}

PUBLIC
IMSService* IMSApp::GetService(IN CONST AString& strServiceName)
{
    if (strServiceName.IsNULL())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objIMSServices.GetSize(); i++)
    {
        IMSService* pService = objIMSServices.GetAt(i);

        if (pService->GetName().Equals(strServiceName))
        {
            return pService;
        }
    }

    return IMS_NULL;
}

PROTECTED VIRTUAL IMS_BOOL IMSApp::OnPreprocess(IN IMSMSG& /* objMSG */)
{
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL IMSApp::OnMessage(IN IMSMSG& /* objMSG */)
{
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL IMSApp::OnPostprocess(IN IMSMSG& /* objMSG */)
{
    return IMS_FALSE;
}

PROTECTED VIRTUAL IIMSActivityControl* IMSApp::GetController()
{
    return IMS_NULL;
}

PRIVATE VIRTUAL IMS_BOOL IMSApp::DispatchMessage(IN IMSMSG& objMSG)
{
    IMS_BOOL bRetValue = IMS_FALSE;

    (void)OnPreprocess(objMSG);
    bRetValue = OnMessage(objMSG);
    (void)OnPostprocess(objMSG);

    return bRetValue;
}
