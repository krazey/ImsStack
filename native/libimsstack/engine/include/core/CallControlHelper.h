/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100415  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _CALL_CONTROL_HELPER_H_
#define _CALL_CONTROL_HELPER_H_

#include "IMSMap.h"

class Replaces;

/*
This class defines a 3rd party call control helper utilities.

Example

See Also

*/
class CallControlHelper
{
private:
    CallControlHelper();
    ~CallControlHelper();

public:
    IMS_BOOL AddSession(IN CONST AString& strSessionId, IN Replaces* pReplaces);
    IMS_UINT32 GetSessionCount() const;
    void RemoveSession(IN CONST AString& strSessionId);

    Replaces* GetReplacesFromSessionId(IN CONST AString& strSessionId);
    const AString& GetSessionIdFromReplaces(IN CONST Replaces* pReplaces);

    static Replaces* CreateReplaces(IN IMS_BOOL bMO, IN ISipDialog* piDialog);
    static const AString CreateSessionId();
    static CallControlHelper* GetInstance();

private:
    IMS_UINT32 nGlobalSessionId;
    // <session-id> , <replaces> pair
    IMSMap<AString, Replaces*> objSessions;
};

#endif  // _CALL_CONTROL_HELPER_H_
