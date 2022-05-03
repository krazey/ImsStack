#ifndef MTC_DIALING_PLAN_H_
#define MTC_DIALING_PLAN_H_

#include "dialingplan/EmergencyNumberList.h"
#include "dialingplan/NormalDialingPlan.h"
#include "ImsIdentity.h"
#include "AString.h"

class IMtcContext;
struct CallInfo;

using NumberFormat = NormalDialingPlan::NumberFormat;
using LocalNumberPolicy = NormalDialingPlan::LocalNumberPolicy;
using Scheme = NormalDialingPlan::Scheme;

class MtcDialingPlan final
{
public:
    explicit MtcDialingPlan(IN IMtcContext& objContext);
    ~MtcDialingPlan();
    MtcDialingPlan(IN const MtcDialingPlan&) = delete;
    MtcDialingPlan& operator=(IN const MtcDialingPlan&) = delete;

public:
    /**
     * @brief returns a translated sip/tel To-Uri
     *
     * @param strNumber the number a user dialed.
     * @param objCallInfo the CallInfo of the call which contains the information of
     *         emergency/ussi characteristic.
     * @param eScheme the uri scheme for the translation.
     * @return the translated sip/tel Uri.
     */
    AString GetToUri(IN const AString& strNumber, IN const CallInfo& objCallInfo,
            IN Scheme eScheme = Scheme::UNKNOWN);

    void OnCountrySpecificServiceUrnReceived(
            IN const AString& strNumber, IN const AString& strServiceUrn);

private:
    IMS_BOOL IsUriForm(IN const AString& strNumber);

private:
    struct TemporaryServiceUrn final
    {
    public:
        TemporaryServiceUrn(IN AString strNumber_, IN AString strUrn_)
        {
            strNumber = strNumber_;
            strUrn = strUrn_;
        }
        TemporaryServiceUrn(IN const TemporaryServiceUrn&) = delete;
        TemporaryServiceUrn& operator=(IN const TemporaryServiceUrn&) = delete;

        inline const AString& GetNumber() const { return strNumber; }
        inline const AString& GetUrn() const { return strUrn; }

        AString strNumber;
        AString strUrn;
    };

    IMtcContext& m_objContext;

    // TODO: no requirement found... try to find the standard again and update the logic.
    std::unique_ptr<TemporaryServiceUrn> m_pTemporaryServiceUrn;
};

#endif
