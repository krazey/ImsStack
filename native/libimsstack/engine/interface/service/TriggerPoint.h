#ifndef _TRIGGER_POINT_H_
#define _TRIGGER_POINT_H_

#include "AStringArray.h"
#include "SipMethod.h"

class ISIPMessage;
class ISIPHeader;

/**
 * @brief This class defines a trigger point which can be used for an initial filter criteria.
 *
 * @see IServiceFilterCriteria
 */
class TriggerPoint
{
public:
    explicit TriggerPoint(IN CONST SIPMethod &objMethod_,
            IN IMS_BOOL bMethodNegated_ = IMS_FALSE);
    TriggerPoint(IN CONST TriggerPoint &objRHS);
    ~TriggerPoint();

public:
    TriggerPoint& operator=(IN CONST TriggerPoint &objRHS);

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
    IMS_BOOL AddHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
            IN CONST AString &strName = AString::ConstNull(),
            IN IMS_BOOL bHeaderNegated = IMS_FALSE);

    // Add an additional parameter for SDP later ...
    /**
     * @brief Adds SDP information for this trigger point.
     *
     * @param cName an SDP line type
     * @param strValue the value of SDP line
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL AddSDPInfo(IN CONST IMS_CHAR cName, IN CONST AString &strValue);

    /**
     * @brief Removes all the SIP headers from this trigger point.
     */
    void RemoveAllHeaders();
    /**
     * @brief Removes all the SDP information from this trigger point.
     */
    void RemoveAllSDPInfo();
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
    void SetEvaluationRule(IN IMS_SINT32 nRule);

protected:
    IMS_BOOL Evaluate(IN CONST ISIPMessage *piSIPMsg) const;
    IMS_UINT32 GetCount() const;

    static IMS_SINT32 CompareHeader(IN CONST ISIPHeader *piHeader,
            IN CONST ISIPHeader *piOtherHeader, IN IMS_SINT32 nEvaluationRule,
            IN IMS_BOOL bConditionNegated = IMS_FALSE);
    static IMS_SINT32 CompareHeaderInMessage(IN CONST ISIPHeader *piHeader,
            IN CONST ISIPMessage *piSIPMsg, IN IMS_SINT32 nEvaluationRule,
            IN IMS_BOOL bConditionNegated = IMS_FALSE);
    static IMS_SINT32 CompareSDPInfo(IN CONST IMSList<AString>& objMLines,
            IN CONST IMSList<AString>& objALines, IN CONST ISIPMessage *piSIPMsg,
            IN IMS_SINT32 nEvaluationRule, IN IMS_BOOL bConditionNegated = IMS_FALSE);
    static IMS_SINT32 GetIndexOf(IN CONST AStringArray &objSDPLines, IN CONST AString &strToken,
            IN IMS_BOOL bContain = IMS_TRUE);
    static IMS_BOOL IsParameterComparisonRequired(IN CONST ISIPHeader *piHeader);
    static void SplitLines(IN CONST AString &strSDP, OUT AStringArray &objSDPLines);

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
    IMS_SINT32 nEvaluationRule;

    // SPT : SIP method
    IMS_BOOL bMethodNegated;
    SIPMethod objMethod;

    // SPT : Header field
    IMSList<ISIPHeader*> objHeaders;
    IMSList<ISIPHeader*> objNegatedHeaders;

    // Additional field : SDP, ...
    IMSList<AString> objSDPMLines;
    IMSList<AString> objSDPALines;
};

#endif // _TRIGGER_POINT_H_
