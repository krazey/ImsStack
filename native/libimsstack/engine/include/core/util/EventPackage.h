/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100328  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _EVENT_PACKAGE_H_
#define _EVENT_PACKAGE_H_

#include "AStringArray.h"

class ISipHeader;

class EventPackage
{
public:
    EventPackage();
    virtual ~EventPackage();

public:
    IMS_UINT32 GetDefaultDuration() const;

    const AString& GetEvent() const;
    const ISipHeader* GetEventHeader() const;
    IMS_SINT32 GetDuration() const;
    const AStringArray& GetMIMETypes() const;

    void SetDuration(IN IMS_SINT32 nDuration);
    void SetEvent(IN CONST AString& strEvent);
    void SetEventHeader(IN ISipHeader* piHeader);
    void SetMIMETypes(IN CONST AStringArray& objMIMETypes);

private:
    enum
    {
        DEFAULT_DURATION = 3600
    };

    // Event header
    AString strEvent;
    ISipHeader* piEventHeader;

    // In any case, MIN & MAX expiration value needs to be defined ...

    // Expires header
    //    - "Expires" header in the initial SUBSCRIBE request
    //    - We will take it as the default subscription duration for this event package
    IMS_SINT32 nInitialDuration;

    // Accept header
    AStringArray objMIMETypes;

    // Allow-Events header : It will be set from the AppConfig.
};

#endif  // _EVENT_PACKAGE_H_
