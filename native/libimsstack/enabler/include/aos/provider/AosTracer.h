/*
 * Copyright (C) 2025 The Android Open Source Project
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
#ifndef AOS_TRACER_H_
#define AOS_TRACER_H_

#include "IAosService.h"
#include "AosUtil.h"
#include "interface/IAosBlock.h"
#include "interface/IAosTracer.h"

/**
 * @class AosTracer
 * @brief Implementation of the centralized diagnostic tracer.
 *
 * Designed to be managed by AosProvider.
 * Manages formatting of trace messages and forwarding them to the Java layer via JNI.
 */
class AosTracer : public IAosTracer
{
public:
    /**
     * @brief Constructor.
     * Instantiated by AosProvider or system initialization logic.
     */
    explicit AosTracer(IN IMS_SINT32 nSlotId);

    /**
     * @brief Destructor.
     */
    ~AosTracer() override;

    // IAosTracer Implementation
    void TraceStatus(IN IAosAppContext* piContext, IN const AosStatusInfo& objStatus) override;

    void TraceEvent(IN IAosAppContext* piContext, IN AosDomain eDomain, IN AosSeverity eSeverity,
            IN const IMS_CHAR* pszEventName, IN const IMS_CHAR* pszEventValue) override;

    void TraceEvent(IN IAosAppContext* piContext, IN AosDomain eDomain, IN AosSeverity eSeverity,
            IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszEventName,
            IN const IMS_CHAR* pszEventValue) override;

protected:
    IMS_BOOL IsTraceTarget(IN IAosAppContext* piContext, OUT AosRegistrationType& eRegType) const;
    AString FormatCriticalKey(IN const AosStatusInfo& objStatus) const;

    static const IMS_CHAR* DomainToString(IN AosDomain eDomain);
    static const IMS_CHAR* SeverityToString(IN AosSeverity eSeverity);

private:
    IMS_SINT32 m_nSlotId;
    AString m_strTag;
    AString m_strLastCriticalLog;
};

#endif  // AOS_TRACER_H_
