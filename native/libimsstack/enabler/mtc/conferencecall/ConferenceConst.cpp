#include "IMSTypeDef.h"
#include "conferencecall/ConferenceConst.h"

PUBLIC GLOBAL const IMS_CHAR ConferenceConst::LEG_ID[] = "legid";

PUBLIC GLOBAL const IMS_CHAR ConferenceConst::FACTORY_URI_FORMAT[] =
        "sip:mmtel@conf-factory.ims.mnc%s.mcc%s.3gppnetwork.org";

PUBLIC GLOBAL const IMS_CHAR ConferenceConst::ANONYMOUS[] = "anonymous";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::ANONYMOUS_URI[] = "sip:anonymous@anonymous.invalid";

PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_CONNECTED[] = "connected";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_DISCONNECTED[] = "disconnected";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_ON_HOLD[] = "on-hold";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_MUTED_VIA_FOCUS[] = "muted-via-focus";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_PENDING[] = "pending";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_ALERTING[] = "alerting";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_DIALING_IN[] = "dialing-in";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_DIALING_OUT[] = "dialing-out";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_DISCONNECTING[] = "disconnecting";
PUBLIC GLOBAL const IMS_CHAR ConferenceConst::STR_STATUS_CONNECT_FAIL[] = "connect-fail";

PUBLIC GLOBAL const IMS_CHAR ConferenceConst::APPLICATION_CONFERENCEINFO[] =
        "application/conference-info+xml";
