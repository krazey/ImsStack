/*
    Author
    IMSers
    <table>
    Date            Author                    Description
    --------      -----------------        ------------------
    20120327    saurabh31.srivastava@               Created

    </table>

    Description
    --------------------------------------------------

*/

#ifndef _IU_UCE_SERVICE_H_
#define _IU_UCE_SERVICE_H_

#include "AString.h"
#include "ImsMessageDef.h"

#define UI2UCEAPP IMS_MSG_BASE_UCE    // 3000
#define UCEAPP2UI IMS_MSG_BASE_UCE + 50

class IUUceService
{
public:
    /*
     * Commands
     */
    // Publish Commands
    static const IMS_SINT32 UCE_SEND_PUBLISH_CMD               = UI2UCEAPP + 1;
    // Subscribe Commands
    static const IMS_SINT32 UCE_SEND_SINGLE_SUBSCRIBE_CMD      = UI2UCEAPP + 2;
    static const IMS_SINT32 UCE_SEND_LIST_SUBSCRIBE_CMD        = UI2UCEAPP + 3;
    // Options Commands
    static const IMS_SINT32 UCE_SEND_OPTIONS_CMD               = UI2UCEAPP + 4;
    static const IMS_SINT32 UCE_SEND_OPTIONS_RESP_CMD          = UI2UCEAPP + 5;

    /*
     * Indications
     */
     // Publish Indication
     static const IMS_SINT32 UCE_PUBLISH_RESPONSE_IND           = UCEAPP2UI + 1;
     static const IMS_SINT32 UCE_PUBLISH_UPDATED_IND            = UCEAPP2UI + 2;
     static const IMS_SINT32 UCE_UNPUBLISHED_IND                = UCEAPP2UI + 3;
     static const IMS_SINT32 UCE_PUBLISH_CMD_ERROR_IND          = UCEAPP2UI + 4;

     // Subscribe Indication
     static const IMS_SINT32 UCE_SUBSCRIBE_RESPONSE_IND        = UCEAPP2UI + 10;
     static const IMS_SINT32 UCE_PRESENCE_NOTIFY_IND           = UCEAPP2UI + 11;
     static const IMS_SINT32 UCE_SUBSCRIBE_CMD_ERROR_IND       = UCEAPP2UI + 12;
     static const IMS_SINT32 UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND = UCEAPP2UI + 13;
     static const IMS_SINT32 UCE_SUBSCRIBE_TERMINATED_IND      = UCEAPP2UI + 14;

     // Options Indication
     static const IMS_SINT32 UCE_OPTIONS_RESPONSE_IND          = UCEAPP2UI + 20;
     static const IMS_SINT32 UCE_OPTIONS_CMD_ERROR_IND         = UCEAPP2UI + 21;
     static const IMS_SINT32 UCE_OPTIONS_RECEIVED_IND          = UCEAPP2UI + 22;

     // connection Indication
     static const IMS_SINT32 UCE_IMS_AGENT_CONNECTED_IND       = UCEAPP2UI + 30;
     static const IMS_SINT32 UCE_IMS_AGENT_DISCONNECTED_IND    = UCEAPP2UI + 31;
     static const IMS_SINT32 UCE_IMS_AGENT_REFRESHED_IND       = UCEAPP2UI + 32;
     static const IMS_SINT32 UCE_NETWORK_CHANGED               = UCEAPP2UI + 33;

     static const IMS_SINT32 UCE_XML_PARSE_COMPLETED_IND       = UCEAPP2UI + 34;
     static const IMS_SINT32 UCE_SUBSCRIBE_DELETED_IND         = UCEAPP2UI + 35;
     static const IMS_SINT32 UCE_OPTIONS_DELETED_IND           = UCEAPP2UI + 36;
     // command error
     /**
      * Service is unknown.
      */
     static const IMS_UINT32 COMMAND_CODE_SERVICE_UNKNOWN = 0;

     /**
      * The command failed with an unknown error.
      */
     static const IMS_UINT32 COMMAND_CODE_GENERIC_FAILURE = 1;

     /**
      * Invalid parameter(s).
      */
     static const IMS_UINT32 COMMAND_CODE_INVALID_PARAM = 2;

     /**
      * Fetch error.
      */
     static const IMS_UINT32 COMMAND_CODE_FETCH_ERROR = 3;

     /**
      * Request timed out.
      */
     static const IMS_UINT32 COMMAND_CODE_REQUEST_TIMEOUT = 4;

     /**
      * Failure due to insufficient memory available.
      */
     static const IMS_UINT32 COMMAND_CODE_INSUFFICIENT_MEMORY = 5;

     /**
      * Network connection is lost.
      */
     static const IMS_UINT32 COMMAND_CODE_LOST_NETWORK_CONNECTION = 6;

     /**
      * Requested feature/resource is not supported.
      */
     static const IMS_UINT32 COMMAND_CODE_NOT_SUPPORTED = 7;

     /**
      * Contact or resource is not found.
      */
     static const IMS_UINT32 COMMAND_CODE_NOT_FOUND = 8;

     /**
      * Service is not available.
      */
     static const IMS_UINT32 COMMAND_CODE_SERVICE_UNAVAILABLE = 9;

     /**
      * Command resulted in no change in state, ignoring.
      */
     static const IMS_UINT32 COMMAND_CODE_NO_CHANGE = 10;
};
#endif    // _IU_UCE_SERVICE_H_
