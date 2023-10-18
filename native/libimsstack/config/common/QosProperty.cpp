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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "QosProperty.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC
QosProperty::QosProperty() :
        ImsProperty(ImsProperty::PKEY_QOS),
        m_strContentType(AString::ConstNull())
{
    m_objQos.nAverageRate = 0;
    m_objQos.nBufferSize = 0;
    m_objQos.nPeakRate = 0;
    m_objQos.nDelay = 0;
    m_objQos.nDelayVariance = 0;
    m_objQos.nMaxChunkSize = 0;
    m_objQos.nMinimalPolicedSize = 0;
}

PUBLIC
QosProperty::QosProperty(IN const AString& strContentType) :
        ImsProperty(ImsProperty::PKEY_QOS),
        m_strContentType(strContentType)
{
    m_objQos.nAverageRate = 0;
    m_objQos.nBufferSize = 0;
    m_objQos.nPeakRate = 0;
    m_objQos.nDelay = 0;
    m_objQos.nDelayVariance = 0;
    m_objQos.nMaxChunkSize = 0;
    m_objQos.nMinimalPolicedSize = 0;
}

PUBLIC
QosProperty::QosProperty(IN const QosProperty& other) :
        ImsProperty(ImsProperty::PKEY_QOS),
        m_strContentType(other.m_strContentType),
        m_objQos(other.m_objQos)
{
}

PUBLIC
QosProperty& QosProperty::operator=(IN const QosProperty& other)
{
    if (this != &other)
    {
        ImsProperty::operator=(other);

        m_strContentType = other.m_strContentType;
        m_objQos = other.m_objQos;
    }

    return (*this);
}

PUBLIC
AString QosProperty::GetQosString() const
{
    AString strQos;

    strQos.Sprintf("%u %u %u %u %u %u %u", m_objQos.nAverageRate, m_objQos.nBufferSize,
            m_objQos.nPeakRate, m_objQos.nDelay, m_objQos.nDelayVariance, m_objQos.nMaxChunkSize,
            m_objQos.nMinimalPolicedSize);

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

    m_objQos.nAverageRate = anValue[0];
    m_objQos.nBufferSize = anValue[1];
    m_objQos.nPeakRate = anValue[2];
    m_objQos.nDelay = anValue[3];
    m_objQos.nDelayVariance = anValue[4];
    m_objQos.nMaxChunkSize = anValue[5];
    m_objQos.nMinimalPolicedSize = anValue[6];

    return IMS_TRUE;
}
