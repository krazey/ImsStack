/*
    Author
    <table>
    date          author                description
    --------      --------------        ----------
    20211021     hhshin@                 Created
    </table>

    Description
    UCE Message Value (JNI / Handler)
*/


package com.android.imsstack.enabler.uce.impl.define;

public class UceMessage {

    /* Native CMD and Indication */


    // Event base
    private static final int UCE_EVENT_U2I     = 3000;
    private static final int UCE_EVENT_I2U     = 3050;

    /*
     * Commands
     */
    // Publish Commands
    public static final int UCE_SEND_PUBLISH_CMD                            = (UCE_EVENT_U2I + 1);

    // Subscribe Commands
    public static final int UCE_SEND_SINGLE_SUBSCRIBE_CMD                   = (UCE_EVENT_U2I + 2);
    public static final int UCE_SEND_LIST_SUBSCRIBE_CMD                     = (UCE_EVENT_U2I + 3);

    // Options Commands
    public static final int UCE_SEND_OPTIONS_CMD                            = (UCE_EVENT_U2I + 4);
    public static final int UCE_SEND_OPTIONS_RESP_CMD                       = (UCE_EVENT_U2I + 5);

    // Registration Commands
    public static final int UCE_GET_IMS_REGISTRATION_CMD                    = (UCE_EVENT_U2I + 6);

    /*
     * Indications
     */
    // Publish Indication
    public static final int UCE_PUBLISH_RESPONSE_IND                        = (UCE_EVENT_I2U + 1);
    public static final int UCE_PUBLISH_UPDATED_IND                         = (UCE_EVENT_I2U + 2);
    public static final int UCE_UNPUBLISHED_IND                             = (UCE_EVENT_I2U + 3);
    public static final int UCE_PUBLISH_CMD_ERROR_IND                       = (UCE_EVENT_I2U + 4);

    // Subscribe Indication
    public static final int UCE_SUBSCRIBE_RESPONSE_IND                      = (UCE_EVENT_I2U + 10);
    public static final int UCE_PRESENCE_NOTIFY_IND                         = (UCE_EVENT_I2U + 11);
    public static final int UCE_SUBSCRIBE_CMD_ERROR_IND                     = (UCE_EVENT_I2U + 12);
    public static final int UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND           = (UCE_EVENT_I2U + 13);
    public static final int UCE_SUBSCRIBE_TERMINATED_IND                    = (UCE_EVENT_I2U + 14);

    // Options Indication
    public static final int UCE_OPTIONS_RESPONSE_IND                        = (UCE_EVENT_I2U + 20);
    public static final int UCE_OPTIONS_CMD_ERROR_IND                       = (UCE_EVENT_I2U + 21);
    public static final int UCE_OPTIONS_RECEIVED_IND                        = (UCE_EVENT_I2U + 22);

    // connection Indication
    public static final int UCE_IMS_AGENT_CONNECTED_IND                     = (UCE_EVENT_I2U + 30);
    public static final int UCE_IMS_AGENT_DISCONNECTED_IND                  = (UCE_EVENT_I2U + 31);
    public static final int UCE_IMS_AGENT_REFRESHED_IND                     = (UCE_EVENT_I2U + 32);
    public static final int UCE_NETWORK_CHANGED                             = (UCE_EVENT_I2U + 33);

    /* Internal Message */
    private static final int UCE_INTERNAL_EVENT_BASE = 500;

    /* associated with Always on Service */
    public static final int UCE_MSG_IMS_AGENT_CONNECTED = ( UCE_INTERNAL_EVENT_BASE + 1 );
    public static final int UCE_MSG_IMS_AGENT_DISCONNECTED = ( UCE_MSG_IMS_AGENT_CONNECTED + 1 );
    public static final int UCE_MSG_IMS_AGENT_REFRESHED = ( UCE_MSG_IMS_AGENT_DISCONNECTED + 1 );

    /* connectivity */
    public static final int UCE_MSG_NETWORK_CHANGED = ( UCE_MSG_IMS_AGENT_REFRESHED + 1 );

    /* publication */
    public static final int UCE_MSG_PUBLISH_RESPONSE = ( UCE_MSG_NETWORK_CHANGED + 1 );
    public static final int UCE_MSG_PUBLISH_CMD_ERROR = ( UCE_MSG_PUBLISH_RESPONSE + 1 );
    public static final int UCE_MSG_UNPUBLISHED = ( UCE_MSG_PUBLISH_CMD_ERROR + 1 );
    public static final int UCE_MSG_PUBLISH_UPDATED = ( UCE_MSG_UNPUBLISHED + 1 );

    /* subscription */
    public static final int UCE_MSG_SUBSCRIBE_RESPONSE = ( UCE_MSG_PUBLISH_UPDATED + 1 );
    public static final int UCE_MSG_PRESENCE_NOTIFY = ( UCE_MSG_SUBSCRIBE_RESPONSE + 1 );
    public static final int UCE_MSG_SUBSCRIBE_CMD_ERROR = ( UCE_MSG_PRESENCE_NOTIFY + 1 );
    public static final int UCE_MSG_SUBSCRIBE_TERMINATED = ( UCE_MSG_SUBSCRIBE_CMD_ERROR + 1 );
    public static final int UCE_MSG_SUBSCRIBE_RESOURCE_TERMINATED
        = ( UCE_MSG_SUBSCRIBE_TERMINATED + 1 );

    /* options */
    public static final int UCE_MSG_OPTIONS_RESPONSE = (UCE_MSG_SUBSCRIBE_RESOURCE_TERMINATED + 1);
    public static final int UCE_MSG_OPTIONS_CMD_ERROR = ( UCE_MSG_OPTIONS_RESPONSE + 1 );
    public static final int UCE_MSG_OPTIONS_RECEIVED = ( UCE_MSG_OPTIONS_CMD_ERROR + 1 );

    public static final int UCE_MSG_RESULT_SUCCESS = 0;

    public static final String[] szInternalMessage = {
        "UCE_MSG_IMS_AGENT_CONNECTED",
        "UCE_MSG_IMS_AGENT_DISCONNECTED",
        "UCE_MSG_IMS_AGENT_REFRESHED",
        "UCE_MSG_NETWORK_CHANGED",
        "UCE_MSG_PUBLISH_RESPONSE",
        "UCE_MSG_UNPUBLISHED",
        "UCE_MSG_PUBLISH_UPDATED",
        "UCE_MSG_PUBLISH_CMD_ERROR",
        "UCE_MSG_SUBSCRIBE_RESPONSE",
        "UCE_MSG_PRESENCE_NOTIFY",
        "UCE_MSG_SUBSCRIBE_CMD_ERROR",
        "UCE_MSG_SUBSCRIBE_TERMINATED",
        "UCE_MSG_SUBSCRIBE_RESOURCE_TERMINATED",
        "UCE_MSG_OPTIONS_RESPONSE",
        "UCE_MSG_OPTIONS_CMD_ERROR",
        "UCE_MSG_OPTIONS_RECEIVED",
        "Unknown Message"
        };
}
