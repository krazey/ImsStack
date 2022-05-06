/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140129  hwangoo.park@             Created
    </table>

    Description
    A listener type for receiving an incoming SIP request notifications.
    All the transactions are managed by the application.

*/

#ifndef _INTERFACE_ON_DIRECT_CORE_SERVICE_LISTENER_H_
#define _INTERFACE_ON_DIRECT_CORE_SERVICE_LISTENER_H_

class CoreService;
class ISIPConnectionFactory;

/*

IOnDirectCoreServiceListener interface

Example

See Also
ICoreService

*/
class IOnDirectCoreServiceListener
{
public:
    /*
    Notifies the application when the SIP server transaction is created and received.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piService               Pointer to the concerned IService
    piSCF                   Pointer to the ISIPConnectionFactory
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              CoreService::RESULT_DIRECT_TXN_xxx
    </table>

    */
    virtual IMS_SINT32 OnDirectCoreService_TransactionReceived(
            IN CoreService* pService, IN ISIPConnectionFactory* piSCF) = 0;
};

#endif  // _INTERFACE_ON_DIRECT_CORE_SERVICE_LISTENER_H_
