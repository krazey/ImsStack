/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef QOS_PROPERTY_H_
#define QOS_PROPERTY_H_

#include "private/ImsProperty.h"

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
    explicit QosProperty(IN const AString& strContentType);
    QosProperty(IN const QosProperty& other);
    inline virtual ~QosProperty() {}

public:
    QosProperty& operator=(IN const QosProperty& other);

public:
    inline IMS_BOOL Equals(IN const AString& strValue) const override
    {
        return m_strContentType.Equals(strValue);
    }
    inline const AString& GetContentType() const { return m_strContentType; }
    inline QualityOfService GetQos() const { return m_objQos; }
    AString GetQosString() const;
    IMS_BOOL SetQos(IN const AString& strValue);

private:
    AString m_strContentType;
    QualityOfService m_objQos;
};

#endif
