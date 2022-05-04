/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120611  saurabh31.srivastava@           Created
    20130820  jaesik.kong@               Re-Factorying for one source
    </table>

    Description

*/

#include "def/UceDef.h"

PUBLIC
UceNamespace::UceNamespace() {}

PUBLIC GLOBAL const IMS_CHAR UceNamespace::UCE_APP_NAME_PREFIX[] = "UceApp";

PUBLIC GLOBAL const IMS_CHAR UceNamespace::PRESENCE[] = "presence";

PUBLIC GLOBAL const IMS_CHAR UceNamespace::PUBMNGR_NAME[] = "UCEPubMngr";

PUBLIC GLOBAL const IMS_CHAR UceNamespace::SUBMNGR_NAME[] = "UCESubMngr";

PUBLIC GLOBAL const IMS_CHAR UceNamespace::OPTMNGR_NAME[] = "UCEOptMngr";

PUBLIC
UceTag::UceTag() {}
PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_IARI[] = "+g.3gpp.iari-ref=";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_ICSI[] = "+g.3gpp.icsi-ref=";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_STANDALONE_PAGER_MESSAGING[] =
    "urn%3Aurn-7%3A3gppservice.ims.icsi.oma.cpm.msg";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_STANDALONE_LARGE_MESSAGING[] =
    "urn%3Aurn-7%3A3gppservice.ims.icsi.oma.cpm.largemsg";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_CHAT[] =
    "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_IM[] =
    "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im";  // KR RCS

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_FULL_STORE_AND_FORWARD_GROUP_CHAT[] =
    "urn%3Aurn-7%3A3gppapplication.ims.iari.rcs.fullsfgroupchat";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_FILE_TRANSFER_THUMBNAIL[] =
    "urn%3Aurn-7%3A3gppapplication.ims.iari.rcs.ftthumb";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_FILE_TRANSFER_STORE_AND_FORWARD[] =
    "urn%3Aurn-7%3A3gppapplication.ims.iari.rcs.ftstandfw";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_FILE_TRANSFER_HTTP[] =
    "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp";  // KR RCS

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_GEOLOCATION_PUSH[] =
    "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush";  // KR RCS

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_FT_SMS[] =
    "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms";  // KR RCS

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_GEOLOCATIONPUSH_SMS[] =
    "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms";  // KR RCS

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_PRESENCE[] =
    "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_IP_VOICE_CALL[] =
    "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_IP_VIDEO_CALL[] = "video";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_SHARED_MAP[] =
    "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_SHARED_SKETCH[] =
    "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_CALL_COMPOSER[] =
    "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_POST_CALL[] =
    "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_CHATBOT_SESSION[] =
    "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_CHATBOT_STANDALONE_MESSAGE[] =
    "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_CHATBOT_VERSION_V1[] =
    "+g.gsma.rcs.botversion=\"#=1\"";

PUBLIC GLOBAL const IMS_CHAR UceTag::TAG_CHATBOT_VERSION_V2[] =
    "+g.gsma.rcs.botversion=\"#=1\",\"#=2\"";
