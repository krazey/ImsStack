#ifndef INTERFACE_MTC_CALL_MANAGER_H_
#define INTERFACE_MTC_CALL_MANAGER_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "call/IMtcCall.h"
#include "IMtcService.h"
#include "MtcDef.h"

// Holds `IMtcCall` objects and provides methods to create, delete and find them.
class IMtcCallManager
{
public:
    virtual ~IMtcCallManager(){};

    // Creates a new call and starts to manage it. Returns the created call.
    virtual IMtcCall* CreateCall(IN ServiceType eServiceType, IN CallInfo& pCallInfo) = 0;

    // Deletes the call matching the given call key. Does nothing if the call doesn't exist.
    virtual void RemoveCall(IN CallKey nCallKey) = 0;

    // Returns a call matching the given call key.
    // Returns new `UnknownCall` instance if the call doesn't exist.
    virtual IMtcCall* GetCallByCallKey(IN CallKey nCallKey) = 0;

    // Returns a list of all calls.
    // The list is sorted in the order in which the calls were created.
    virtual IMSList<IMtcCall*> GetCalls() = 0;

    // Returns a list of calls matching the given session type.
    // The list is sorted in the order in which the calls were created.
    virtual IMSList<IMtcCall*> GetCallsByType(IN CallType eCallType) = 0;

    // Returns a list of calls matching the given service type.
    // The list is sorted in the order in which the calls were created.
    virtual IMSList<IMtcCall*> GetCallsByServiceType(IN ServiceType eServiceType) = 0;

    // Returns a list of conference calls.
    // The list is sorted in the order in which the calls were created.
    virtual IMSList<IMtcCall*> GetCallsInConference() = 0;
};

#endif
