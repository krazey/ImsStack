/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "QosProperty.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC
QosProperty::QosProperty() :
        ImsProperty(ImsProperty::PKEY_QOS),
        strContentType(AString::ConstNull())
{
    stQos.nAverageRate = 0;
    stQos.nBufferSize = 0;
    stQos.nPeakRate = 0;
    stQos.nDelay = 0;
    stQos.nDelayVariance = 0;
    stQos.nMaxChunkSize = 0;
    stQos.nMinimalPolicedSize = 0;
}

PUBLIC
QosProperty::QosProperty(IN const AString& strContentType_) :
        ImsProperty(ImsProperty::PKEY_QOS),
        strContentType(strContentType_)
{
    stQos.nAverageRate = 0;
    stQos.nBufferSize = 0;
    stQos.nPeakRate = 0;
    stQos.nDelay = 0;
    stQos.nDelayVariance = 0;
    stQos.nMaxChunkSize = 0;
    stQos.nMinimalPolicedSize = 0;
}

PUBLIC
QosProperty::QosProperty(IN const QosProperty& objRHS) :
        ImsProperty(ImsProperty::PKEY_QOS),
        strContentType(objRHS.strContentType),
        stQos(objRHS.stQos)
{
}

PUBLIC VIRTUAL QosProperty::~QosProperty() {}

PUBLIC
QosProperty& QosProperty::operator=(IN const QosProperty& objRHS)
{
    if (this != &objRHS)
    {
        ImsProperty::operator=(objRHS);

        strContentType = objRHS.strContentType;
        stQos = objRHS.stQos;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL QosProperty::Equals(IN const AString& strValue) const
{
    return strContentType.Equals(strValue);
}

PUBLIC
const AString& QosProperty::GetContentType() const
{
    return strContentType;
}

PUBLIC
QosProperty::QualityOfService QosProperty::GetQos() const
{
    return stQos;
}

PUBLIC
AString QosProperty::GetQosString() const
{
    AString strQos;

    strQos.Sprintf("%u %u %u %u %u %u %u", stQos.nAverageRate, stQos.nBufferSize, stQos.nPeakRate,
            stQos.nDelay, stQos.nDelayVariance, stQos.nMaxChunkSize, stQos.nMinimalPolicedSize);

    return strQos;
}

PUBLIC
IMS_BOOL QosProperty::SetQos(IN const AString& strValue)
{
    AStringArray objValues = ImsProperty::Decode(strValue);

    if (objValues.GetCount() != 7)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bOK;
    IMS_SINT32 anValue[7] = {
            0,
    };

    // <average rate>SP<buffer size>SP<peak rate>SP<delay>
    // SP<delayVariance>SP<max chunk size>SP<minimal policed size>
    for (IMS_SINT32 i = 0; i < objValues.GetCount(); ++i)
    {
        bOK = IMS_FALSE;

        anValue[i] = objValues.GetElementAt(i).ToUInt32(&bOK, 10);

        if (!bOK)
        {
            IMS_TRACE_D("Unable to parse QoS string (%s) element: %s", strValue.GetStr(),
                    objValues.GetElementAt(i).GetStr(), 0);
            return IMS_FALSE;
        }
    }

    stQos.nAverageRate = anValue[0];
    stQos.nBufferSize = anValue[1];
    stQos.nPeakRate = anValue[2];
    stQos.nDelay = anValue[3];
    stQos.nDelayVariance = anValue[4];
    stQos.nMaxChunkSize = anValue[5];
    stQos.nMinimalPolicedSize = anValue[6];

    return IMS_TRUE;
}
