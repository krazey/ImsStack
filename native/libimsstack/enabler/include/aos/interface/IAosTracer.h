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
#ifndef INTERFACE_AOS_TRACER_
#define INTERFACE_AOS_TRACER_

#include "ImsTypeDef.h"
#include "interface/IAosAppContext.h"

/**
 * @brief Severity levels for diagnostic traces.
 *        Shortened to single letters to match Android Logcat style and save buffer space.
 */
enum class AosSeverity
{
    I = 0,  // INFO: Normal operational milestones (e.g., state changes, success).
    W = 1,  // WARN: Potential issues, transient failures, or retry actions.
    E = 2   // ERROR: Definite failures, critical errors, or permanent blocks.
};

/**
 * @brief Logical domains for diagnostic traces.
 * Maps to source code directory structures for easy identification of the originating module.
 */
enum class AosDomain
{
    APP = 0,  // app/ (AosApplication state)
    COND,     // condition/ (Service prerequisites and blocking conditions)
    CONN,     // connection/ (PDN/Network connection management)
    HDL,      // handle/ (Specific service handles like MTC, MTS)
    REG,      // registration/ (SIP Registration procedures)
    RSUB,     // registration Subscription/ (SIP Subscription procedures)
    SUBS,     // Subscriber/ (Subscriber identity and SIM state specific events)
    MISC      // Miscellaneous or uncategorized events
};

/**
 * @brief Data structure encapsulating the vital status of the system.
 *        Designed as a DTO (Data Transfer Object) for TraceStatus method.
 *        This structure holds a single, consistent snapshot of the application's core state
 *        and network conditions at the moment of tracing.
 */
struct AosStatusInfo
{
    IMS_UINT32 m_nAppState = 0;
    IMS_UINT32 m_nRegResult = 0;
    IMS_UINT32 m_nRegReason = 0;
    IMS_UINT32 m_nConnState = 0;
    IMS_UINT32 m_nConnReason = 0;
    IMS_UINT32 m_nOffReason = 0;
    IMS_SINT32 m_nDataFailureReason = 0;

    /**
     * @brief Default Constructor.
     *        Initializes all members to their specified default values (0, IMS_FALSE, etc.).
     */
    AosStatusInfo() = default;

    /**
     * @brief Parameterized Constructor (Full Initialization).
     *        Used internally by the static factory method to set all known vital fields.
     *
     * @param nAppState Current main application state.
     * @param nRegResult Last registration result code.
     * @param nRegReason Last registration reason code.
     * @param nConnState Current connection state.
     * @param nConnReason Current connection reason.
     * @param nOffReason Reason why service is off.
     * @param nDataFailCause Last known data failure reason.
     * @return A fully initialized AosStatusInfo object.
     */
    AosStatusInfo(IN IMS_UINT32 nAppState, IN IMS_UINT32 nRegResult, IN IMS_UINT32 nRegReason,
            IN IMS_UINT32 nConnState, IN IMS_UINT32 nConnReason, IN IMS_UINT32 nOffReason,
            IN IMS_SINT32 nDataFailCause) :
            m_nAppState(nAppState),
            m_nRegResult(nRegResult),
            m_nRegReason(nRegReason),
            m_nConnState(nConnState),
            m_nConnReason(nConnReason),
            m_nOffReason(nOffReason),
            m_nDataFailureReason(nDataFailCause)
    {
    }

    /**
     * @brief Static Factory Method: CollectStatus
     *        Creates a new status snapshot with all vital parameters required for tracing.
     */
    static AosStatusInfo CollectStatus(IN IMS_UINT32 nAppState, IN IMS_UINT32 nRegResult,
            IN IMS_UINT32 nRegReason, IN IMS_UINT32 nConnState, IN IMS_UINT32 nConnReason,
            IN IMS_UINT32 nOffReason, IN IMS_SINT32 nDataFailCause)
    {
        return AosStatusInfo(nAppState, nRegResult, nRegReason, nConnState, nConnReason, nOffReason,
                nDataFailCause);
    }
};

/**
 * @brief Interface for the centralized system tracing facility.
 * Allows native modules to report structured diagnostic data to the upper layers (Java)
 * via a standardized and simplified API.
 */
class IAosTracer
{
public:
    virtual ~IAosTracer() {}

    /**
     * @brief Captures and traces a comprehensive snapshot of the system's vital status.
     *        Uses the AosStatusInfo DTO structure.
     *
     * @param piContext Pointer to the application context. Used to retrieve Block Reasons and
     *                  Slot ID for tracing.
     * @param objStatus A structure containing the current App State, OffReason, and RAT...
     *
     */
    virtual void TraceStatus(IN IAosAppContext* piContext, IN const AosStatusInfo& objStatus) = 0;

    /**
     * @brief Traces a specific transactional diagnostic event. (Standard Single Name)
     *
     * @param piContext Pointer to the application context.
     * @param eDomain The originating module domain.
     * @param eSeverity The importance level.
     * @param pszEventName A short, human-readable string identifying the event type.
     * @param pszEventValue string value detailing the event result.
     *
     * @note Log Format: "[Domain][Severity] EventName : EventValue"
     */
    virtual void TraceEvent(IN IAosAppContext* piContext, IN AosDomain eDomain,
            IN AosSeverity eSeverity, IN const IMS_CHAR* pszEventName,
            IN const IMS_CHAR* pszEventValue) = 0;

    /**
     * @brief Traces a specific transactional diagnostic event (Composite Name Overload).
     *
     * @param piContext Pointer to the application context.
     * @param eDomain The originating module domain.
     * @param eSeverity The importance level.
     * @param pszPrefix String prefix (e.g., "Cmd_").
     * @param pszEventName Event name (e.g., "REG_START").
     * @param pszEventValue string value detailing the event result.
     *
     * @note Log Format: "[Domain][Severity] PrefixEventName : EventValue"
     */
    virtual void TraceEvent(IN IAosAppContext* piContext, IN AosDomain eDomain,
            IN AosSeverity eSeverity, IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszEventName,
            IN const IMS_CHAR* pszEventValue) = 0;
};

// ===================================================================================
//  AosTracer Helper Macros
// ===================================================================================
/**
 * @note IMPORTANT: The source file using these macros MUST include "provider/AosProvider.h".
 */

#define _GET_AOS_TRACER(CTX) \
    ((CTX != IMS_NULL) ? AosProvider::GetInstance()->GetTracer((CTX)->GetSlotId()) : IMS_NULL)

/**
 * @brief Macro for TraceStatus.
 *
 * @param CTX          Pointer to IAosAppContext (Must be valid to retrieve SlotId).
 * @param STATUS_INFO  An instance of AosStatusInfo (Pre-constructed by caller).
 */
#define TRACE_AOS_STATUS(CTX, STATUS_INFO)                    \
    do                                                        \
    {                                                         \
        if ((CTX) != IMS_NULL)                                \
        {                                                     \
            IAosTracer* _piTracer = _GET_AOS_TRACER(CTX);     \
            if (_piTracer != IMS_NULL)                        \
            {                                                 \
                _piTracer->TraceStatus((CTX), (STATUS_INFO)); \
            }                                                 \
        }                                                     \
    } while (0)

// -----------------------------------------------------------------------------------
// Macros for TraceEvent (Standard & Composite Name)
// -----------------------------------------------------------------------------------

/**
 * @brief Macros for TraceEvent (Standard: Single Event Name)
 *        Use TRACE_AOS_I (Info), TRACE_AOS_W (Warn), TRACE_AOS_E (Error).
 *
 * @param CTX Pointer to IAosAppContext.
 * @param DOM AosDomain.
 * @param EVENT Event Name String.
 * @param VAL Numeric Value.
 */
#define TRACE_AOS_I(CTX, DOM, EVENT, VAL)                                        \
    do                                                                           \
    {                                                                            \
        IAosTracer* _piTracer = _GET_AOS_TRACER(CTX);                            \
        if (_piTracer != IMS_NULL)                                               \
        {                                                                        \
            _piTracer->TraceEvent((CTX), (DOM), AosSeverity::I, (EVENT), (VAL)); \
        }                                                                        \
    } while (0)

#define TRACE_AOS_W(CTX, DOM, EVENT, VAL)                                        \
    do                                                                           \
    {                                                                            \
        IAosTracer* _piTracer = _GET_AOS_TRACER(CTX);                            \
        if (_piTracer != IMS_NULL)                                               \
        {                                                                        \
            _piTracer->TraceEvent((CTX), (DOM), AosSeverity::W, (EVENT), (VAL)); \
        }                                                                        \
    } while (0)

#define TRACE_AOS_E(CTX, DOM, EVENT, VAL)                                        \
    do                                                                           \
    {                                                                            \
        IAosTracer* _piTracer = _GET_AOS_TRACER(CTX);                            \
        if (_piTracer != IMS_NULL)                                               \
        {                                                                        \
            _piTracer->TraceEvent((CTX), (DOM), AosSeverity::E, (EVENT), (VAL)); \
        }                                                                        \
    } while (0)

/**
 * @brief Macros for TraceEvent with Composite Name (C = Composite)
 *        These macros combine the PREFIX and NAME strings for a dynamic event name.
 *
 * @param CTX Pointer to IAosAppContext.
 * @param DOM AosDomain.
 * @param PREFIX String prefix (e.g., "Cmd_").
 * @param NAME Event name (e.g., "REGISTER_START").
 * @param VAL Numeric Value.
 */
#define TRACE_AOS_C_I(CTX, DOM, PREFIX, NAME, VAL)                                        \
    do                                                                                    \
    {                                                                                     \
        IAosTracer* _piTracer = _GET_AOS_TRACER(CTX);                                     \
        if (_piTracer != IMS_NULL)                                                        \
        {                                                                                 \
            _piTracer->TraceEvent((CTX), (DOM), AosSeverity::I, (PREFIX), (NAME), (VAL)); \
        }                                                                                 \
    } while (0)

#define TRACE_AOS_C_W(CTX, DOM, PREFIX, NAME, VAL)                                        \
    do                                                                                    \
    {                                                                                     \
        IAosTracer* _piTracer = _GET_AOS_TRACER(CTX);                                     \
        if (_piTracer != IMS_NULL)                                                        \
        {                                                                                 \
            _piTracer->TraceEvent((CTX), (DOM), AosSeverity::W, (PREFIX), (NAME), (VAL)); \
        }                                                                                 \
    } while (0)

#define TRACE_AOS_C_E(CTX, DOM, PREFIX, NAME, VAL)                                        \
    do                                                                                    \
    {                                                                                     \
        IAosTracer* _piTracer = _GET_AOS_TRACER(CTX);                                     \
        if (_piTracer != IMS_NULL)                                                        \
        {                                                                                 \
            _piTracer->TraceEvent((CTX), (DOM), AosSeverity::E, (PREFIX), (NAME), (VAL)); \
        }                                                                                 \
    } while (0)

#endif  // INTERFACE_AOS_TRACER_
