/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100427  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "EngineActivity.h"

PUBLIC
EngineActivity::EngineActivity(IN CONST AString& strName_ /* = AString::ConstNull() */) :
        IMSActivity(strName_)
{
}

PUBLIC VIRTUAL EngineActivity::~EngineActivity() {}

/*

Remarks

*/
PROTECTED VIRTUAL IIMSActivityControl* EngineActivity::GetController()
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_BOOL EngineActivity::DispatchMessage(IN IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
        case AMSG_DESTROY:
            OnDestroy();
            return IMS_TRUE;

        default:
            break;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL void EngineActivity::OnDestroy()
{
    //---------------------------------------------------------------------------------------------

    delete this;
}
