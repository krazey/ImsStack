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
#ifndef SIP_PARAMETER_H_
#define SIP_PARAMETER_H_

#include "AStringArray.h"

/**
 * @brief This class provides an interface for SIP parameters (URI / header parameters).
 */
class SipParameter
{
public:
    SipParameter();
    explicit SipParameter(IN const AString& strName);
    SipParameter(IN const AString& strName, IN const AString& strValue);
    SipParameter(IN const AString& strName, IN const AStringArray& objValues);
    SipParameter(IN const SipParameter& other);
    ~SipParameter();

public:
    SipParameter& operator=(IN const SipParameter& other);

public:
    /**
     * @brief Adds the value of SIP parameter.
     *
     * @param strValue The parameter value to be added
     */
    void AddValue(IN const AString& strValue);

    /**
     * @brief Adds the values (comma-separated string format) of SIP parameter.
     *
     * @param strValues The parameter values to be added
     */
    void AddValues(IN const AString& strValues);

    /**
     * @brief Parses the SIP parameter.
     *
     * @param strParameter The SIP parameter value format\n
     *                     If it represents multiple values, it's a comma-separated string format.
     */
    IMS_BOOL Create(IN const AString& strParameter);

    /**
     * @brief Checks if both SIP parameter is the same or not.
     *
     * @param pParameter The SIP parameter to be compared
     * @return If both SIP parameter is matched, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Equals(IN const SipParameter* pParameter) const;

    /**
     * @brief Gets the name of SIP parameter.
     *
     * @return The SIP parameter name.
     */
    inline const AString& GetName() const { return m_strName; }

    /**
     * @brief Gets the topmost value of SIP parameter.
     *
     * @return The topmost SIP parameter value.
     */
    const AString& GetValue() const;

    /**
     * @brief Gets all the values of SIP parameter.
     *
     * @return The SIP parameter values.
     */
    inline const AStringArray& GetValues() const { return m_objValues; }

    /**
     * @brief Checks if this SIP parameter is name-only parameter.
     *
     * @return If it's a name-only parameter (boolean parameter), returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsNameOnly() const { return (m_objValues.GetCount() == 0); }

    /**
     * @brief Removes the specified value from SIP parameter.
     *
     * @param strValue The parameter value to be removed
     */
    void RemoveValue(IN const AString& strValue);

    /**
     * @brief Sets the value of SIP parameter.
     *
     * @param strValue The parameter value to be set
     */
    IMS_RESULT SetValue(IN const AString& strValue);

    /**
     * @brief Sets the values (comma-separated string format) of SIP parameter.
     *
     * @param strValues The parameter values to be set
     */
    IMS_RESULT SetValues(IN const AString& strValues);

    /**
     * @brief Returns SIP parameter value as a string format.
     *
     * @return The string format of SIP parameter.
     */
    AString ToString() const;

private:
    AString m_strName;
    AStringArray m_objValues;
};

#endif
