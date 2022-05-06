/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _QOS_PROPERTY_H_
#define _QOS_PROPERTY_H_

#include "private/ImsProperty.h"

/*

Class QoSProperty

Example

See Also

*/
class QosProperty : public ImsProperty
{
public:
    struct QualityOfService
    {
        IMS_SINT32 nAverageRate;
        IMS_SINT32 nBufferSize;
        IMS_SINT32 nPeakRate;
        IMS_SINT32 nDelay;
        IMS_SINT32 nDelayVariance;
        IMS_SINT32 nMaxChunkSize;
        IMS_SINT32 nMinimalPolicedSize;
    };

public:
    QosProperty();
    explicit QosProperty(IN const AString& strContentType_);
    QosProperty(IN const QosProperty& objRHS);
    virtual ~QosProperty();

public:
    QosProperty& operator=(IN const QosProperty& objRHS);

public:
    virtual IMS_BOOL Equals(IN const AString& strValue) const;

    const AString& GetContentType() const;
    QualityOfService GetQos() const;
    AString GetQosString() const;
    IMS_BOOL SetQos(IN const AString& strValue);

private:
    AString strContentType;
    QualityOfService stQos;
};

#endif  // _QOS_PROPERTY_H_
