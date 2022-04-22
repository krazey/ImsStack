
/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100927  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_ISIM_H_
#define _INTERFACE_IMS_ISIM_H_

#include "ImsTypeDef.h"

class IDigestAKA;
class IISIMListener;

class IISIM
{
public:
    /*
     Clears all the records which are obtained during the initialization or refresh procedure.
    This method can be invoked to read ISIM records only excluding the EF attributes.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void ClearRecords() = 0;

    /*

    Creates the Digest AKA.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IDigestAKA*             Pointer to IDigestAKA
    </table>

    */
    virtual IDigestAKA* CreateDigestAKA() = 0;

    /*

    Gets the value of the specific field.
    The field type will be determined if needed.

    NOTE:
     The result of this operation will be reported by the IISIMListener interface.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nField                  Field type to be read
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SUCCESS             The operation is done successfully
    IMS_FAILURE             The operation is failed
    </table>

    */
    virtual IMS_RESULT GetField(IN IMS_SINT32 nField) = 0;

    /*

    Gets the home domain name.

    NOTE:
     The result of this operation will be reported by the IISIMListener interface.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SUCCESS             The operation is done successfully
    IMS_FAILURE             The operation is failed
    </table>

    */
    virtual IMS_RESULT GetHomeDomainName() = 0;

    /*

    Gets the private user identity.

    NOTE:
     The result of this operation will be reported by the IISIMListener interface.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SUCCESS             The operation is done successfully
    IMS_FAILURE             The operation is failed
    </table>

    */
    virtual IMS_RESULT GetIMPI() = 0;

    /*

    Gets the public user identities.

    NOTE:
     The result of this operation will be reported by the IISIMListener interface.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SUCCESS             The operation is done successfully
    IMS_FAILURE             The operation is failed
    </table>

    */
    virtual IMS_RESULT GetIMPU() = 0;

    /*
     Returns the state of ISIM.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              STATE_xxx (refer to below enum value)
    </table>

    */
    virtual IMS_SINT32 GetState() const = 0;

    /*

    Checks if the ISIM is ready or not.
    If the return value is IMS_FALSE, the application MUST wait for the invocation of
    ISIM_OnStateChanged(...) method in the IISIMListener interface with STATE_READY.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                ISIM is ready;so, the application can retrieve the items.
    IMS_FALSE               ISIM is not ready
    </table>

    */
    virtual IMS_BOOL IsReady() = 0;

    /*

    Add the listener to receive the result of operation.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piListener              Listener to be added
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void AddListener(IN IISIMListener *piListener) = 0;

    /*

    Remove the listener to receive the result of operation.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piListener              Listener to be removed
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void RemoveListener(IN IISIMListener *piListener) = 0;


    /*

    Starts the ISIM module to establish a session to communicate with the ISIM application
    of the device.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nEFs                    Selected EFs to be retrieved from ISIM
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SUCCESS             The operation succeeds
    IMS_FAILURE             The operation fails
    </table>

    */
    virtual IMS_RESULT Start(IN IMS_SINT32 nEFs = EF_ALL) = 0;

    /*

    Starts the ISIM module to register a client and its event callback to communicate
    with the ISIM application of the device.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SUCCESS             The operation succeeds
    IMS_FAILURE             The operation fails
    </table>

    */
    virtual IMS_RESULT Init() = 0;

    /*
     Releases all the resource related to the ISIM.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void Release() = 0;

public:
    // Identifiers of ISIM record fields
    enum
    {
        EF_NONE = 0x0000,

        EF_IMPI = 0x0001,
        EF_DOMAIN = 0x0002,
        EF_IMPU = 0x0004,
        EF_IST = 0x0008,
        EF_PCSCF = 0x0010,

        EF_ALL = 0xFFFF
    };

    // Additional field values : TBD
    enum
    {
        // For other field values
        FIELD_NONE = 0,
        FIELD_IST,
        FIELD_PCSCF_ADDRESS,
        FIELD_MAX
    };

    /*    ISIM status.
        STATE_IDLE    means the default state.
        STATE_INIT means that a client is registered to ISIM application.
        STATE_READY means that User can execute ISIM operation like reading IMPI
                    or making the authentication.
        STATE_REFRESHING means that ISIM application is refreshing.
        STATE_REFRESHED means that ISIM application refresh is completed.*/
    enum
    {
        STATE_IDLE = 0,
        STATE_INIT,
        STATE_READY,
        STATE_REFRESHING,
        STATE_REFRESHED
    };

    /*     error codes
        ERROR_INIT_FAILED means
            that it failed to register a client and its event callback. it's a total failure.
        ERROR_START_FAILED means
            that it failed to establish a session. User can recover with STATE_INIT.
        ERROR_READ_IMPI_FAILED, ERROR_READ_IMPU_FAILED, and ERROR_READ_DOMAIN_FAILED mean
            that it failed to read IMPI, IMPU, and DOMAIN for some reason.
            User can resume with STATE_INIT.
        ERROR_READ_PCSCF_ADDRESS_FAILED means
            that it failed to read PCSCF Address for some reason. User can resume with STATE_INIT.
        ERROR_READ_DENIED means
            that it is blocked to access the ISIM application because of PIN code.
            User should wait for the following STATE_READY event and read again.
        ERROR_CARD_ERROR means
            a general UICC card error. User can resume the operation with STATE_INIT.
        ERROR_CARD_REMOVED means
            that the UICC card is removed. User can resume the operation with STATE_INIT.
        ERROR_NO_ISIM_APPLICATION means
            that there is no ISIM application in UICC.
    */
    enum
    {
        ERROR_INIT_FAILED = 0,
        ERROR_START_FAILED,
        ERROR_READ_IMPI_FAILED,
        ERROR_READ_IMPU_FAILED,
        ERROR_READ_DOMAIN_FAILED,
        ERROR_READ_IST_FAILED,
        ERROR_READ_PCSCF_ADDRESS_FAILED,
        ERROR_READ_DENIED,
        ERROR_CARD_ERROR,
        ERROR_CARD_REMOVED,
        ERROR_REFRESH_REG_FAILED,
        ERROR_INTERFACE_CHANNEL_ERROR,
        ERROR_REFRESH_ERROR,
        ERROR_NO_ISIM_APPLICATION
    };

    // Exception result of GET operation
    enum
    {
        RESULT_NO_RECORDS = (-2)
    };
};

#endif // _INTERFACE_IMS_ISIM_H_
