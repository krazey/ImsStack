package com.android.imsstack.enabler.mtc;

public class IUMtcService {

    public static final int EVENT_U2I = 1000;
    public static final int EVENT_I2U = 1100;

    // Event : UI to IMS
    public static final int OPEN_SESSION                    = (EVENT_U2I + 1);
    public static final int ATTCH_SESSION                   = (EVENT_U2I + 2);
    public static final int CLOSE_SESSION                   = (EVENT_U2I + 3);
    public static final int REGISTER_SERVICE                = (EVENT_U2I + 4);
    public static final int SRVCC_STATE_CHANGED             = (EVENT_U2I + 5);
    public static final int SET_TERMINAL_BASED_CALL_WAITING = (EVENT_U2I + 6);
    public static final int OPEN_EMERGENCY_SERVICE          = (EVENT_U2I + 7);

    // Event : IMS to UI
    public static final int SERVICE_CHANGED                = (EVENT_I2U + 1);
    public static final int E_SERVICE_CHANGED            = (EVENT_I2U + 2);
    public static final int PRE_INCOMING_CALL            = (EVENT_I2U + 3);
    public static final int INCOMING_CALL_INFO            = (EVENT_I2U + 4);
    public static final int AUTO_REJECTED_CALL            = (EVENT_I2U + 5);

    // Service Status
    public static final int SERVICE_NONE            = 0;
    public static final int SERVICE_VOIP            = 1;
    public static final int SERVICE_VT                = 2;
    public static final int SERVICE_UC                = 3;
    public static final int SERVICE_EMERGENCY        = 4;
    public static final int SERVICE_OPENING            = 5;

    // Service Status Reason
    public static final int SERVICESTATUS_REASON_UNKNOWN            = 0;
    public static final int SERVICESTATUS_REASON_NETWORKDISABLE        = 1;
    public static final int SERVICESTATUS_REASON_SIMSINVALID        = 2;
    public static final int SERVICESTATUS_REASON_BYSERVER            = 3;
    public static final int SERVICESTATUS_REASON_USERSELECT            = 4;
    public static final int SERVICESTATUS_REASON_FORBIDDEN              = 5;

    // Emergency Service Mode
    public static final int ES_IDLE            = 0;
    public static final int ES_OPENING        = 1;
    public static final int ES_OPENED        = 2;
    public static final int ES_UNAVAILABLE    = 3;
    public static final int ES_ES            = 4;
    public static final int ES_READY            = 5;

    // Emergency Service Reason
    public static final int ES_IDLE_REASON_UNKNOWN      = -1;
    public static final int ES_IDLE_REASON_NONE         = 0;
    public static final int ES_IDLE_REASON_WITH_ECM     = 1;

    public static final int ES_UNAVAILABLE_REASON_UNKNOWN = -1;
    public static final int ES_UNAVAILABLE_REASON_NONE    = 0;
    public static final int ES_UNAVAILABLE_REASON_NO_CSFB = 1;
    public static final int ES_UNAVAILABLE_REASON_SSAC    = 2;
};
