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
#ifndef SIP_METHOD_H_
#define SIP_METHOD_H_

#include "AString.h"

/**
 * @brief This class provides an interface for SIP methods.
 */
class SipMethod
{
public:
    SipMethod(IN IMS_SINT32 nMethod = SipMethod::INVALID);
    explicit SipMethod(IN const IMS_CHAR* pszMethod);
    explicit SipMethod(IN const AString& strMethod);
    SipMethod(IN const SipMethod& other);
    ~SipMethod();

public:
    SipMethod& operator=(IN const SipMethod& other);
    SipMethod& operator=(IN IMS_SINT32 nMethod);
    SipMethod& operator=(IN const IMS_CHAR* pszMethod);
    SipMethod& operator=(IN const AString& strMethod);

public:
    /**
     * @brief Checks if SIP method is the same or not.
     *
     * @param nMethod SIP method enumeration type\n
     *                #ACK\n
     *                #BYE\n
     *                #CANCEL\n
     *                #INVITE\n
     *                #OPTIONS\n
     *                #REGISTER\n
     *                #PRACK\n
     *                #SUBSCRIBE\n
     *                #NOTIFY\n
     *                #UPDATE\n
     *                #MESSAGE\n
     *                #REFER\n
     *                #PUBLISH\n
     *                #INFO\n
     *                #UNKNOWN
     * @return If it matches, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN IMS_SINT32 nMethod) const { return m_nMethod == nMethod; }
    /**
     * @brief Checks if SIP method is the same or not.
     *
     * @param pszMethod SIP method as null-termianted string format
     * @return If it matches, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN const IMS_CHAR* pszMethod) const
    {
        return m_strMethod.Equals(pszMethod);
    }
    /**
     * @brief Checks if SIP method is the same or not.
     *
     * @param strMethod SIP method as string format
     * @return If it matches, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN const AString& strMethod) const
    {
        return m_strMethod.Equals(strMethod);
    }
    /**
     * @brief Checks if SIP method is the same or not.
     *
     * @param objMethod SIP method object
     * @return If it matches, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN const SipMethod& objMethod) const
    {
        return ((m_nMethod == objMethod.m_nMethod) && m_strMethod.Equals(objMethod.m_strMethod));
    }

    /**
     * @brief Gets SIP method as enumeration type.
     *
     * @return SIP method.\n
     *         #ACK\n
     *         #BYE\n
     *         #CANCEL\n
     *         #INVITE\n
     *         #OPTIONS\n
     *         #REGISTER\n
     *         #PRACK\n
     *         #SUBSCRIBE\n
     *         #NOTIFY\n
     *         #UPDATE\n
     *         #MESSAGE\n
     *         #REFER\n
     *         #PUBLISH\n
     *         #INFO\n
     *         #UNKNOWN
     */
    inline IMS_SINT32 ToInt() const { return m_nMethod; }

    /**
     * @brief Gets SIP method as string format.
     *
     * @return SIP method as string ("INVITE" / "BYE" / ...).
     */
    inline const AString& ToString() const { return m_strMethod; }

    /**
     * @brief Converts the enumeration type to string format of SIP method.
     *
     * @param nMethod SIP method enumeration type\n
     *                #ACK\n
     *                #BYE\n
     *                #CANCEL\n
     *                #INVITE\n
     *                #OPTIONS\n
     *                #REGISTER\n
     *                #PRACK\n
     *                #SUBSCRIBE\n
     *                #NOTIFY\n
     *                #UPDATE\n
     *                #MESSAGE\n
     *                #REFER\n
     *                #PUBLISH\n
     *                #INFO\n
     *                #UNKNOWN
     * @return SIP method as null-terminated string format ("INVITE" / "BYE" / ...).
     */
    static const IMS_CHAR* ToName(IN IMS_SINT32 nMethod);

private:
    static IMS_SINT32 ConvertStringToMethod(IN const AString& strMethod);
    static AString ConvertMethodToString(IN IMS_SINT32 nMethod);

public:
    enum
    {
        INVALID = (-1),

        /// Defined in RFC 3261
        ACK = 0,
        BYE,       ///< Defined in RFC 3261
        CANCEL,    ///< Defined in RFC 3261
        INVITE,    ///< Defined in RFC 3261
        OPTIONS,   ///< Defined in RFC 3261
        REGISTER,  ///< Defined in RFC 3261

        /// Defined in RFC 3262
        PRACK,

        /// Defined in RFC 3265
        SUBSCRIBE,
        NOTIFY,  ///< Defined in RFC 3265

        /// Defined in RFC 3311
        UPDATE,

        /// Defined in RFC 3428
        MESSAGE,

        /// Defined in RFC 3515
        REFER,

        /// Defined in RFC 3903
        PUBLISH,

        /// Defined in RFC 2976
        INFO,

        UNKNOWN,

        MAX
    };

    static const IMS_CHAR* NAME[];
    static const SipMethod INVALID_METHOD;

private:
    IMS_SINT32 m_nMethod;
    AString m_strMethod;
};

#endif
