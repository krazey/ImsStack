/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120102  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_PHONE_INFO_CALL_H_
#define _INTERFACE_IMS_PHONE_INFO_CALL_H_

#include "AString.h"

class ICallInfo
{
public:
    /*
     Checks if the MERONG number is blocked number or not.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strNumber               Originator's number to be checked
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The specified number is blocked
    IMS_FALSE               The specified number is not blocked
    </table>
    */
    virtual IMS_BOOL IsEmergencyNumber(IN const AString &strNumber) const = 0;

    /*
        static final int TTY_MODE_OFF = 0;
        static final int TTY_MODE_FULL = 1;
        static final int TTY_MODE_HCO = 2;
        static final int TTY_MODE_VCO = 3;
    */
    virtual IMS_UINT32 GetTtyMode() const = 0;

    /*
        #define IMS_RTT_MODE_NONE (0)
        #define IMS_RTT_VISIBLE_DURING_CALLS (1)
        #define IMS_RTT_ALWAYS_VISIBLE (2)
        #define IMS_RTT_CAPABLE_OFF (3)
    */
    virtual IMS_UINT32 GetRttMode() const = 0;

    /*
    Get Wifi Calling Mode Preferences

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_UINT32              0: wfc preferred, 1: wfc only, 2: cellular preferred
    </table>
    */
    virtual IMS_BOOL IsWifiCallingEnabled() = 0;

    /*
    Get Wifi Calling Mode Preferences

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_UINT32              0: wfc preferred, 1: wfc only, 2: cellular preferred
    </table>
    */
    virtual IMS_UINT32 GetWifiCallingPreferences() = 0;

    /*
    Get Wifi Calling Provisioned

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_BOOL                TRUE : provisioned, FALSE: not provisioned
    </table>
    */
    virtual IMS_BOOL IsWifiCallingProvisioned() = 0;

    /*
    Get Wifi Calling Provisioned

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Address ID
    </table>
    */
    virtual AString GetWifiCallingAddressId() = 0;

    /*
    Get Call State in Other Slot

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_UINT32              call state
    </table>
    */
    virtual IMS_SINT32 GetCsCallStateInOtherSlot() const = 0;

};

#endif // _INTERFACE_IMS_PHONE_INFO_CALL_H_
