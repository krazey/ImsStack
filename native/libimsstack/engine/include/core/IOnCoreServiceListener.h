/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100524  hwangoo.park@             Created
    </table>

    Description
    A listener type for receiving notifications on remotely initiated core service methods.

*/

#ifndef _INTERFACE_ON_CORE_SERVICE_LISTENER_H_
#define _INTERFACE_ON_CORE_SERVICE_LISTENER_H_

class ReasonInfo;
class Message;
class CoreService;
class PageMessage;
class Reference;
class SessionEx;
class Capabilities;

/*

IOnCoreServiceListener interface

Example

See Also
CoreService

*/
class IOnCoreServiceListener
{
public:
    /*

    Notifies the application when a page message is received from a remote endpoint.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pService                Pointer to the concerned Service
    pMessage                Pointer to the received PageMessage
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnCoreService_PageMessageReceived(
            IN CoreService* pService, IN PageMessage* pMessage) = 0;

    /*

    Notifies the application when a reference request is received from a remote endpoint.
    Only references that are created outside of a session are notified in this method.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pService                Pointer to the concerned Service
    pReference              Pointer to the received Reference
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnCoreService_ReferenceReceived(
            IN CoreService* pService, IN Reference* pReference) = 0;

    /*

    Notifies the application when a CoreService is closed.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pService                Pointer to the concerned Service
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnCoreService_ServiceClosed(
            IN CoreService* pService, IN ReasonInfo* pReasonInfo) = 0;

    /*

    Notifies the application when a session invitation is received from a remote endpoint.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pService                Pointer to the concerned Service
    pSession                Pointer to the received session invitation
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnCoreService_SessionInvitationReceived(
            IN CoreService* pService, IN SessionEx* pSession) = 0;

    /*

    Notifies the application when an unsolicited notify is received.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pService                Pointer to the concerned Service
    pNotify                 Pointer to the received NOTIFY message
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnCoreService_UnsolicitedNotifyReceived(
            IN CoreService* pService, IN Message* pNotify) = 0;

    //// IMS extensions
    /*

    Notifies the application when a capability query is received from a remote endpoint.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pService                Pointer to the concerned Service
    pCapabilities           Pointer to the received capability query
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnCoreService_CapabilityQueryReceived(
            IN CoreService* pService, IN Capabilities* pCapabilities) = 0;
};

#endif  // _INTERFACE_ON_CORE_SERVICE_LISTENER_H_
