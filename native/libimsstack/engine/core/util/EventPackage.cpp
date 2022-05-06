/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100328  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ISipHeader.h"
#include "util/EventPackage.h"

PUBLIC
EventPackage::EventPackage() :
        strEvent(AString::ConstNull()),
        piEventHeader(IMS_NULL),
        nInitialDuration(-1)
{
}

PUBLIC VIRTUAL EventPackage::~EventPackage()
{
    if (piEventHeader != IMS_NULL)
        piEventHeader->Destroy();
}

PUBLIC
IMS_UINT32 EventPackage::GetDefaultDuration() const
{
    //---------------------------------------------------------------------------------------------

    // Default duration for all event packages...
    return DEFAULT_DURATION;
}

PUBLIC
IMS_SINT32 EventPackage::GetDuration() const
{
    //---------------------------------------------------------------------------------------------

    return nInitialDuration;
}

PUBLIC
const AString& EventPackage::GetEvent() const
{
    //---------------------------------------------------------------------------------------------

    return strEvent;
}

PUBLIC
const ISipHeader* EventPackage::GetEventHeader() const
{
    //---------------------------------------------------------------------------------------------

    return piEventHeader;
}

PUBLIC
const AStringArray& EventPackage::GetMIMETypes() const
{
    //---------------------------------------------------------------------------------------------

    return objMIMETypes;
}

PUBLIC
void EventPackage::SetDuration(IN IMS_SINT32 nDuration)
{
    //---------------------------------------------------------------------------------------------

    this->nInitialDuration = nDuration;
}

PUBLIC
void EventPackage::SetEvent(IN CONST AString& strEvent)
{
    //---------------------------------------------------------------------------------------------

    this->strEvent = strEvent;
}

PUBLIC
void EventPackage::SetEventHeader(IN ISipHeader* piHeader)
{
    //---------------------------------------------------------------------------------------------

    if (this->piEventHeader != IMS_NULL)
    {
        this->piEventHeader->Destroy();
    }

    this->piEventHeader = piHeader;
}

PUBLIC
void EventPackage::SetMIMETypes(IN CONST AStringArray& objMIMETypes)
{
    //---------------------------------------------------------------------------------------------

    this->objMIMETypes = objMIMETypes;
}
