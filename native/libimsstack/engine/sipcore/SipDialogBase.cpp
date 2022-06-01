/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  hwangoo.park@             Created
    </table>

    Description
     This class defines a base class for SIP dialog class.
*/

#include "SipDialogBase.h"

PUBLIC
SIPDialogBase::SIPDialogBase(IN SIPDialogState* pDState_) :
        RCObject(),
        pDState(pDState_)
{
}

PUBLIC
SIPDialogBase::SIPDialogBase(IN CONST SIPDialogBase& objRHS) :
        RCObject(objRHS),
        pDState(objRHS.pDState)
{
}

PUBLIC VIRTUAL SIPDialogBase::~SIPDialogBase() {}

/*

Remarks

*/
PUBLIC
SIPDialogBase& SIPDialogBase::operator=(IN CONST SIPDialogBase& objRHS)
{
    //---------------------------------------------------------------------------------------------

    RCObject::operator=(objRHS);

    return (*this);
}

/*

Remarks

*/
PUBLIC
SIPDialogState* SIPDialogBase::GetDialogState() const
{
    //---------------------------------------------------------------------------------------------

    return pDState.Get();
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPDialogBase::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return pDState->GetState();
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_BOOL SIPDialogBase::IsDialogCreatable(IN CONST SipMethod& objMethod)
{
    //---------------------------------------------------------------------------------------------

    if (objMethod.Equals(SipMethod::INVALID))
        return IMS_FALSE;

    // Check if the current method can create a dialog or not
    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::SUBSCRIBE) &&
            !objMethod.Equals(SipMethod::REFER)
            /* && !objMethod.Equals(SipMethod::NOTIFY) */)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
