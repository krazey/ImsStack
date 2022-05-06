/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100324  joonhun.shin@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"

#include "IMSActivityMngr.h"

#if 0  // public
#endif

PUBLIC
IMSActivityMngr::IMSActivityMngr() :
        nMajorId(0),
        nMinorId(0),
        objIMSActivities(IMSList<IMSActivity*>())
{
}

PUBLIC
IMSActivityMngr::~IMSActivityMngr() {}

PUBLIC
IMS_BOOL IMSActivityMngr::Attach(IN IMSActivity* pIMSActivity)
{
    if (pIMSActivity == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return objIMSActivities.Append(pIMSActivity);
}

PUBLIC
void IMSActivityMngr::Detach(IN IMSActivity* pIMSActivity)
{
    for (IMS_UINT32 i = 0; i < objIMSActivities.GetSize(); ++i)
    {
        IMSActivity* pActivity = objIMSActivities.GetAt(i);

        if (pActivity == pIMSActivity)
        {
            objIMSActivities.RemoveAt(i);

            if (objIMSActivities.IsEmpty())
            {
                nMajorId = 0;
                nMinorId = 0;
            }
            break;
        }
    }
}

PUBLIC
IMSActivity* IMSActivityMngr::Get(IN CONST AString& strActivityName)
{
    for (IMS_UINT32 i = 0; i < objIMSActivities.GetSize(); ++i)
    {
        IMSActivity* pActivity = objIMSActivities.GetAt(i);

        if (pActivity->GetName().Equals(strActivityName))
        {
            return pActivity;
        }
    }

    return IMS_NULL;
}

PUBLIC
AString IMSActivityMngr::GenerateName(IN CONST AString& strThreadName, IN CONST AString& strName)
{
    AString strNewName;

    if (strName.GetLength() > 0)
    {
        strNewName.Sprintf("%s.%s", strThreadName.GetStr(), strName.GetStr());
    }
    else
    {
        strNewName.Sprintf("%s.ATVT%X_%X", strThreadName.GetStr(), nMajorId, nMinorId++);

        if (nMinorId == 0xFFFFFFFF)
        {
            nMajorId++;
            nMinorId = 0;
        }
    }

    return strNewName;
}

PUBLIC
IMS_BOOL IMSActivityMngr::HandleMessage(IN IMSMSG& objMSG)
{
    AString strTartgetName = objMSG.GetTargetName();
    AString strTartgetActivityName = GetActivityNameFromMsg(strTartgetName);

    IMSActivity* pIMSActivity = Get(strTartgetActivityName);

    if (pIMSActivity != IMS_NULL)
    {
        return pIMSActivity->DispatchMessage(objMSG);
    }

    return IMS_FALSE;
}

PUBLIC
IIMSActivityControl* IMSActivityMngr::GetController(IN CONST AString& strControllerName)
{
    IMSActivity* pIMSActivity = Get(strControllerName);

    if (pIMSActivity != IMS_NULL)
    {
        return pIMSActivity->GetController();
    }

    return IMS_NULL;
}

#if 0  // protected
#endif

#if 0  // private
#endif

PRIVATE
AString IMSActivityMngr::GetActivityNameFromMsg(IN CONST AString& strTargetName)
{
    IMS_SINT32 nOffset = strTargetName.GetIndexOf('.');
    IMS_SINT32 nIndex = strTargetName.GetIndexOf('.', nOffset + 1);

    return strTargetName.GetSubStr(0, nIndex);
}
