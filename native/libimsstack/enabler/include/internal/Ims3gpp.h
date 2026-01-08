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
#ifndef IMS_3GPP_H_
#define IMS_3GPP_H_

#include "AString.h"

class INode;

/**
 * @brief This class defines the methods to get the information of 3GPP IM CN subsystem XML body.
 */
class Ims3gpp
{
public:
    class AlternativeService
    {
    public:
        inline AlternativeService() :
                m_nType(TYPE_UNKNOWN),
                m_strType(AString::ConstNull()),
                m_nAction(ACTION_UNKNOWN),
                m_strAction(AString::ConstNull()),
                m_strReason(AString::ConstNull())
        {
        }
        inline ~AlternativeService() {}

        AlternativeService(IN const AlternativeService&) = delete;
        AlternativeService& operator=(IN const AlternativeService&) = delete;

    public:
        inline IMS_SINT32 GetAction() const { return m_nAction; }
        inline const AString& GetReason() const { return m_strReason; }
        inline IMS_SINT32 GetType() const { return m_nType; }
        inline const AString& GetUnknownAction() const { return m_strAction; }
        inline const AString& GetUnknownType() const { return m_strType; }

    public:
        /// type in alternative-service
        enum
        {
            TYPE_UNKNOWN = 0,
            TYPE_EMERGENCY = 1,
            TYPE_RESTORATION = 2
        };

        /// action in alternative-service
        enum
        {
            ACTION_UNKNOWN = 0,
            ACTION_EMERGENCY_REGISTRATION = 1,
            ACTION_INITIAL_REGISTRATION = 2,
            ACTION_ANONYMOUS_EMERGENCYCALL = 3
        };

    private:
        friend class Ims3gpp;

        IMS_SINT32 m_nType;
        AString m_strType;
        IMS_SINT32 m_nAction;
        AString m_strAction;
        AString m_strReason;
    };

    class ServiceInfo
    {
    public:
        inline ServiceInfo() :
                m_strServiceInfo(AString::ConstNull())
        {
        }
        inline ~ServiceInfo() {}

        ServiceInfo(IN const ServiceInfo&) = delete;
        ServiceInfo& operator=(IN const ServiceInfo&) = delete;

    public:
        inline const AString& GetServiceInfo() const { return m_strServiceInfo; }

    private:
        friend class Ims3gpp;

        AString m_strServiceInfo;
    };

public:
    Ims3gpp();
    explicit Ims3gpp(IN const AString& str3gppIms);
    inline ~Ims3gpp() {}

    Ims3gpp(IN const Ims3gpp&) = delete;
    Ims3gpp& operator=(IN const Ims3gpp&) = delete;

public:
    inline const AlternativeService& GetAlternativeService() const
    {
        return m_objAlternativeService;
    }
    inline const ServiceInfo& GetServiceInfo() const { return m_objServiceInfo; }
    inline IMS_SINT32 GetType() const { return m_nType; }

    IMS_BOOL Parse(IN const AString& str3gppIms);

private:
    void CreateAlternativeService(IN const INode* piNode);
    void CreateServiceInfo(IN const INode* piNode);

public:
    /// "alternative-service" or "service-info"
    enum
    {
        TYPE_UNKNOWN = 0,
        TYPE_ALTERNATIVE_SERVICE = 1,
        TYPE_SERVICE_INFO = 2
    };

    static const IMS_CHAR ELEMENT_ACTION[];
    static const IMS_CHAR ELEMENT_ALTERNATIVE_SERVICE[];
    static const IMS_CHAR ELEMENT_IMS_3GPP[];
    static const IMS_CHAR ELEMENT_REASON[];
    static const IMS_CHAR ELEMENT_SERVICE_INFO[];
    static const IMS_CHAR ELEMENT_TYPE[];
    static const IMS_CHAR ATTR_VERSION[];

private:
    IMS_SINT32 m_nType;
    AlternativeService m_objAlternativeService;
    ServiceInfo m_objServiceInfo;
};

#endif
