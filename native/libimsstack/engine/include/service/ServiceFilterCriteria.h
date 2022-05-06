/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110719  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SERVICE_FILTER_CRITERIA_H_
#define _SERVICE_FILTER_CRITERIA_H_

#include "IMSMap.h"
#include "IServiceFilterCriteria.h"

class ServiceFilterCriteria : public IServiceFilterCriteria
{
public:
    ServiceFilterCriteria();
    virtual ~ServiceFilterCriteria();

private:
    ServiceFilterCriteria(IN CONST ServiceFilterCriteria& objRHS);
    ServiceFilterCriteria& operator=(IN CONST ServiceFilterCriteria& objRHS);

public:
    // IServiceFilterCriteria class
    virtual IMS_UINT32 AddTriggerPoint(IN CONST TriggerPoint& objTP);
    virtual void RemoveTriggerPoint(IN IMS_SINT32 nTriggerPointId);
    virtual void RemoveAllTriggerPoints();
    virtual void SetCalleePreference(
            IN CONST SipMethod& objMethod, IN IMS_BOOL bCalleePreference = IMS_TRUE);

    IMS_UINT32 Evaluate(IN CONST ISipMessage* piSIPMsg) const;
    IMS_BOOL IsCalleePreferenceSupported(IN CONST SipMethod& objMethod) const;
    IMS_BOOL IsEmpty() const;

private:
    // < SIP method, Flag of callee preference >
    IMSMap<IMS_SINT32, IMS_BOOL> objCalleePreferences;

    IMS_UINT32 nNextTriggerPointId;
    IMSMap<IMS_UINT32, TriggerPoint*> objTPs;
};

#endif  // _SERVICE_FILTER_CRITERIA_H_
