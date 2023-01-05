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
#ifndef TRIGGER_POINT_H_
#define TRIGGER_POINT_H_

#include "AStringArray.h"

#include "SipMethod.h"

class ISipHeader;
class ISipMessage;

/**
 * @brief This class defines a trigger point which can be used for an initial filter criteria.
 *
 * @see IServiceFilterCriteria
 */
class TriggerPoint
{
public:
    explicit TriggerPoint(IN const SipMethod& objMethod, IN IMS_BOOL bMethodNegated = IMS_FALSE);
    TriggerPoint(IN const TriggerPoint& other);
    ~TriggerPoint();

public:
    TriggerPoint& operator=(IN const TriggerPoint& other);

public:
    /**
     * @brief Adds SIP header for this trigger point.
     *
     * @param nType an SIP header type for known header type
     * @param strValue the header field value
     * @param strName the header field name if header type is unknown
     * @param bHeaderNegated flag to indicate whether the header is negated or not
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL AddHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull(),
            IN IMS_BOOL bHeaderNegated = IMS_FALSE);

    // Add an additional parameter for SDP later ...
    /**
     * @brief Adds SDP information for this trigger point.
     *
     * @param cName an SDP line type
     * @param strValue the value of SDP line
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL AddSdpInfo(IN const IMS_CHAR cName, IN const AString& strValue);

    /**
     * @brief Removes all the SIP headers from this trigger point.
     */
    void RemoveAllHeaders();
    /**
     * @brief Removes all the SDP information from this trigger point.
     */
    void RemoveAllSdpInfo();
    /**
     * @brief Sets the evaluation rules for this trigger point.
     *
     * @param nRule the rules to evaluate the trigger points\n
     *              Followings are bitmasked:\n
     *                 #SPT_SIP_RULE_MATCH\n
     *                 #SPT_SIP_RULE_CONTAIN\n
     *                 #SPT_SDP_RULE_MATCH\n
     *                 #SPT_SDP_RULE_CONTAIN
     */
    inline void SetEvaluationRule(IN IMS_SINT32 nRule) { m_nEvaluationRule = nRule; }

protected:
    IMS_BOOL Evaluate(IN const ISipMessage* piSipMsg) const;
    IMS_UINT32 GetCount() const;

    static IMS_SINT32 CompareHeader(IN const ISipHeader* piHeader,
            IN const ISipHeader* piOtherHeader, IN IMS_SINT32 nEvaluationRule,
            IN IMS_BOOL bConditionNegated = IMS_FALSE);
    static IMS_SINT32 CompareHeaderInMessage(IN const ISipHeader* piHeader,
            IN const ISipMessage* piSipMsg, IN IMS_SINT32 nEvaluationRule,
            IN IMS_BOOL bConditionNegated = IMS_FALSE);
    static IMS_SINT32 CompareSdpInfo(IN const IMSList<AString>& objMLines,
            IN const IMSList<AString>& objALines, IN const ISipMessage* piSipMsg,
            IN IMS_SINT32 nEvaluationRule, IN IMS_BOOL bConditionNegated = IMS_FALSE);
    static IMS_SINT32 GetIndexOf(IN const AStringArray& objSdpLines, IN const AString& strToken,
            IN IMS_BOOL bContain = IMS_TRUE);
    static IMS_BOOL IsParameterComparisonRequired(IN const ISipHeader* piHeader);
    static void SplitLines(IN const AString& strSdp, OUT AStringArray& objSdpLines);

public:
    /// Matching rules
    enum
    {
        // SDP info. SHOULD always be applied to "contain" rule

        /// Checks if the value is matched the one in the message
        /// Exceptional
        ///   SIP header (Accept-Contact, Contact)
        SPT_SIP_RULE_MATCH = 0x01,
        /// Checks if the value is contained the one in the message
        SPT_SIP_RULE_CONTAIN = 0x02,
        /// Checks if SDP information is matched or not
        SPT_SDP_RULE_MATCH = 0x04,
        /// Checks if SDP information is contained or not
        SPT_SDP_RULE_CONTAIN = 0x08
    };

private:
    friend class ServiceFilterCriteria;

    enum
    {
        SPT_MATCH_NONE = 0x00,
        SPT_MATCH_NOK = 0x01,
        SPT_MATCH_OK = 0x02
    };

    // Rule for SIP header or SDP information
    // Default : SPT_SIP_RULE_MATCH | SPT_SDP_RULE_CONTAIN
    IMS_SINT32 m_nEvaluationRule;

    // SPT : SIP method
    IMS_BOOL m_bMethodNegated;
    SipMethod m_objMethod;

    // SPT : Header field
    IMSList<ISipHeader*> m_objHeaders;
    IMSList<ISipHeader*> m_objNegatedHeaders;

    // Additional field : SDP, ...
    IMSList<AString> m_objSdpMLines;
    IMSList<AString> m_objSdpALines;
};

#endif
