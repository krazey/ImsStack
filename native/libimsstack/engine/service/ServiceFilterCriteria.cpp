/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110719  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceFilterCriteria.h"

__IMS_TRACE_TAG_IMS__;

PUBLIC
ServiceFilterCriteria::ServiceFilterCriteria() :
        objCalleePreferences(IMSMap<IMS_SINT32, IMS_BOOL>()),
        nNextTriggerPointId(1),
        objTPs(IMSMap<IMS_UINT32, TriggerPoint*>())
{
    objCalleePreferences.Add(SipMethod::INVITE, IMS_FALSE);
    objCalleePreferences.Add(SipMethod::OPTIONS, IMS_FALSE);
    objCalleePreferences.Add(SipMethod::MESSAGE, IMS_FALSE);
    objCalleePreferences.Add(SipMethod::REFER, IMS_FALSE);
}

PUBLIC VIRTUAL ServiceFilterCriteria::~ServiceFilterCriteria()
{
    RemoveAllTriggerPoints();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_UINT32 ServiceFilterCriteria::AddTriggerPoint(IN CONST TriggerPoint& objTP)
{
    TriggerPoint* pTP = new TriggerPoint(objTP);

    //---------------------------------------------------------------------------------------------

    if (pTP == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a TriggerPoint failed", 0, 0, 0);
        return 0;
    }

    if (!objTPs.Add(nNextTriggerPointId, pTP))
    {
        IMS_TRACE_E(0, "Adding a TriggerPoint failed", 0, 0, 0);
        return 0;
    }

    ++nNextTriggerPointId;

    return (nNextTriggerPointId - 1);
}

/*

Remarks

*/
PUBLIC VIRTUAL void ServiceFilterCriteria::RemoveTriggerPoint(IN IMS_SINT32 nTriggerPointId)
{
    IMS_SLONG nIndex = objTPs.GetIndexOfKey(nTriggerPointId);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        IMS_TRACE_D("ServiceFilterCriteria :: No trigger point (%d)", nTriggerPointId, 0, 0);
        return;
    }

    TriggerPoint* pTP = objTPs.GetValueAt(nIndex);

    if (pTP != IMS_NULL)
    {
        delete pTP;
    }

    objTPs.RemoveAt(nIndex);

    if (objTPs.IsEmpty())
    {
        nNextTriggerPointId = 1;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL void ServiceFilterCriteria::RemoveAllTriggerPoints()
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objTPs.GetSize(); ++i)
    {
        TriggerPoint* pTP = objTPs.GetValueAt(i);

        if (pTP == IMS_NULL)
            continue;

        delete pTP;
    }

    objTPs.Clear();

    nNextTriggerPointId = 1;
}

/*

Remarks

*/
PUBLIC VIRTUAL void ServiceFilterCriteria::SetCalleePreference(
        IN CONST SipMethod& objMethod, IN IMS_BOOL bCalleePreference /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::OPTIONS) &&
            !objMethod.Equals(SipMethod::MESSAGE) && !objMethod.Equals(SipMethod::REFER))
    {
        return;
    }

    objCalleePreferences.SetValue(objMethod.ToInt(), bCalleePreference);
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 ServiceFilterCriteria::Evaluate(IN CONST ISipMessage* piSIPMsg) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objTPs.GetSize(); ++i)
    {
        TriggerPoint* pTP = objTPs.GetValueAt(i);

        if (pTP == IMS_NULL)
            continue;

        if (pTP->Evaluate(piSIPMsg))
        {
            return pTP->GetCount();
        }
    }

    return 0;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL ServiceFilterCriteria::IsCalleePreferenceSupported(IN CONST SipMethod& objMethod) const
{
    //---------------------------------------------------------------------------------------------

    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::OPTIONS) &&
            !objMethod.Equals(SipMethod::MESSAGE) && !objMethod.Equals(SipMethod::REFER))
    {
        return IMS_FALSE;
    }

    return objCalleePreferences.GetValue(objMethod.ToInt());
}

/*

Remarks

*/
PUBLIC
IMS_BOOL ServiceFilterCriteria::IsEmpty() const
{
    //---------------------------------------------------------------------------------------------

    return objTPs.IsEmpty();
}
