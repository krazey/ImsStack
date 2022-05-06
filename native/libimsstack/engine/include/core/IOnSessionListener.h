/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_ON_SESSION_LISTENER_H_
#define _INTERFACE_ON_SESSION_LISTENER_H_

class ISipServerConnection;
class Session;
class Reference;

/*

IOnSessionListener interface

Example

See Also

*/
class IOnSessionListener
{
public:
    /*
     Notifies the application that the remote part's terminal is alerting the user
    of this session invitation.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_Alerting(IN Session* pSession) = 0;

    /*
     Notifies the application that a reference request has been received from a remote endpoint.
    Only references that are created in a session are notified in this method.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    pReference              Pointer to Reference object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_ReferenceReceived(IN Session* pSession, IN Reference* pReference) = 0;

    /*
     Notifies the application that the session has been established.
    This callback is invoked at both involved endpoints.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_Started(IN Session* pSession) = 0;

    /*
     Notifies the application that the session could not be established.
    This callback is invoked at both involved endpoints.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_StartFailed(IN Session* pSession) = 0;

    /*
     Notifies the application that the session has been terminated or that the session could
    no longer stay established.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_Terminated(IN Session* pSession) = 0;

    /*
     Notifies the application that the session has been updated.
    This callback is invoked at both involved endpoints.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_Updated(IN Session* pSession) = 0;

    /*
     Notifies the application that the session update has been rejected.
    This callback is invoked at both involved endpoints.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_UpdateFailed(IN Session* pSession) = 0;

    /*
     Notifies the application that the remote endpoint adds more media components to
    an established session and the local endpoint is now expected to accept or reject the update.
     If the remote removed a media component, changed media directions, or updated
    application-specific SDP attributes, then this method is not called.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_UpdateReceived(IN Session* pSession) = 0;

    /*
     Notifies the application that the CANCEL operation is successfully done
    during an active call.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnSession_CancelDelivered(IN Session* pSession) = 0;

    /*
     Notifies the application that the CANCEL operation is failed during an active call.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnSession_CancelDeliveryFailed(IN Session* pSession) = 0;

    /*
     Notifies the application that the session has been received the forked response
    except for 100.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    pForkedSession          Pointer to Session object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_BOOL                IMS_TRUE if the transaction is successfully processed
                            IMS_FALSE if the transaction is not processed
    </table>

    */
    virtual IMS_BOOL OnSession_ForkedResponseReceived(
            IN Session* pSession, IN Session* pForkedSession) = 0;

    /*
     Notifies the application that the session has been received the provisional response
    except for 100.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    nIndex                  Index of the current response message
                            (0xFFFFFFFF : most recent message)
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnSession_ProvisionalResponseReceived(
            IN Session* pSession, IN IMS_UINT32 nIndex = 0xFFFFFFFF) = 0;

    /*
     Notifies the application that the remote endpoint starts a new SIP transaction
    within the session.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSession                Pointer to Session object
    piSSC                   Pointer to ISipServerConnection object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_BOOL                IMS_TRUE if the transaction is successfully processed
                            IMS_FALSE if the transaction is not processed
    </table>

    */
    virtual IMS_BOOL OnSession_TransactionReceived(
            IN Session* pSession, IN ISipServerConnection* piSSC) = 0;
};

#endif  // _INTERFACE_ON_SESSION_LISTENER_H_
