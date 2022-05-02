#ifndef _SIP_TIMER_VALUES_H_
#define _SIP_TIMER_VALUES_H_

#include "IMSTypeDef.h"

/**
 * @brief This class defines the SIP transaction timer values which are set by the application.
 */
class SIPTimerValues
{
public:
    SIPTimerValues();
    SIPTimerValues(IN CONST SIPTimerValues &objRHS);
    ~SIPTimerValues();

public:
    SIPTimerValues& operator=(IN CONST SIPTimerValues &objRHS);

public:
    /**
     * @brief Gets the timer value for the give type.
     *
     * @param nType Type of SIP timer\n
     *              #TIMER_T1\n
     *              #TIMER_T2\n
     *              #TV_TIMER_B\n
     *              #TV_TIMER_D\n
     *              #TV_TIMER_F\n
     *              #TV_TIMER_H\n
     *              #TV_TIMER_I\n
     *              #TV_TIMER_J\n
     *              #TV_TIMER_K
     * @return The timer value.
     */
    IMS_SINT32 GetValue(IN IMS_SINT32 nType) const;
    /**
     * @brief Checks if the given timer is provisioned or not.
     *
     * @param nType Type of SIP timer\n
     *              #TIMER_T1\n
     *              #TIMER_T2\n
     *              #TV_TIMER_B\n
     *              #TV_TIMER_D\n
     *              #TV_TIMER_F\n
     *              #TV_TIMER_H\n
     *              #TV_TIMER_I\n
     *              #TV_TIMER_J\n
     *              #TV_TIMER_K
     * @return If the timer is provisioned, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSet(IN IMS_SINT32 nType) const;
    /**
     * @brief Sets the timer value.
     *
     * @param nType Type of SIP timer\n
     *              #TIMER_T1\n
     *              #TIMER_T2\n
     *              #TV_TIMER_B\n
     *              #TV_TIMER_D\n
     *              #TV_TIMER_F\n
     *              #TV_TIMER_H\n
     *              #TV_TIMER_I\n
     *              #TV_TIMER_J\n
     *              #TV_TIMER_K
     * @param nValue Timer value to be set
     */
    void SetValue(IN IMS_SINT32 nType, IN IMS_UINT32 nValue);

    /**
     * @brief Creates SIPTimerValues based on T1 and T2.
     *
     * @param nT1 T1 timer value
     * @param nT2 T2 timer value
     * @return The newly created SIPTimerValues.
     */
    static SIPTimerValues CreateTimerValues(IN IMS_SINT32 nT1, IN IMS_SINT32 nT2);

public:
    /// Types of SIP transaction timer
    enum
    {
        /// T1: RTT estimate
        TIMER_T1 = 0x0001,
        /// T2: The maximum retransmit interval for non-INVITE requests and INVITE responses
        TIMER_T2 = 0x0002,
        /// Timer B: INVITE transaction timeout timer
        TV_TIMER_B = 0x0004,
        /// Timer D: Wait time for response retransmits
        TV_TIMER_D = 0x0008,
        /// Timer F: non-INVITE transaction timeout timer
        TV_TIMER_F = 0x0010,
        /// Tiimer H: Wait time for ACK receipt
        TV_TIMER_H = 0x0020,
        /// Timer I: Wait time for ACK retransmits
        TV_TIMER_I = 0x0040,
        /// Timer J: Wait time for non-INVITE request retransmits
        TV_TIMER_J = 0x0080,
        /// Timer K: Wait time for response retransmits
        TV_TIMER_K = 0x0100,
        TV_ALL = 0x01FF
    };

private:
    IMS_UINT32 nTV_Flags;

    IMS_UINT32 nTV_T1;
    IMS_UINT32 nTV_T2;
    IMS_UINT32 nTV_TimerB; // UAC, INVITE
    IMS_UINT32 nTV_TimerD; // UAC, INVITE response retransmit received
    IMS_UINT32 nTV_TimerF; // UAC, non-INVITE
    IMS_UINT32 nTV_TimerH; // ACK receipt
    IMS_UINT32 nTV_TimerI; // ACK retransmit received
    IMS_UINT32 nTV_TimerJ; // UAS, non-INVITE
    IMS_UINT32 nTV_TimerK; // UAC, non-INVITE
};

#endif // _SIP_TIMER_VALUES_H_
