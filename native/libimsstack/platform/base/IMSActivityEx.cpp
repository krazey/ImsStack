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

#include "IMSActivityEx.h"

#if 0  // public
#endif

PUBLIC
IMSActivityEx::IMSActivityEx(IN CONST AString& strName) :
        IMSActivity(strName)
{
}

PUBLIC VIRTUAL IMSActivityEx::~IMSActivityEx() {}

#if 0  // protected
#endif

PROTECTED VIRTUAL IMS_BOOL IMSActivityEx::OnMessage(IN IMSMSG& /* objMSG */)
{
    return IMS_FALSE;
}

#if 0  // private
#endif

PRIVATE VIRTUAL IMS_BOOL IMSActivityEx::DispatchMessage(IN IMSMSG& objMSG)
{
    return OnMessage(objMSG);
}
