/*
    Author
    IMSers
    <table>
    Date      Description
    --------  ----------
    20090711  Created
    20100330  re-arrange
    </table>

    Description
    This file defines a top-level event category for IMS client platform.
*/

#ifndef _IMS_MSGDEF_H_
#define _IMS_MSGDEF_H_

#include "ImsConstDef.h"

#define IMS_MSG_THREAD_BASE                0 // for thread operation
#define IMS_MSG_SYSTEM_BASE                500 // for system operation
#define IMS_MSG_APP_BASE                   1000
#define IMS_MSG_APP_INTERNAL               10000
#define IMS_MSG_USER_BASE                  100000

//// THREAD_BASE
// Event for Thread Start
#define IMS_MSG_START                       (IMS_MSG_THREAD_BASE + 1)
#define IMS_MSG_TERMINATE                   (IMS_MSG_THREAD_BASE + 2)
#define IMS_MSG_APP_CONTROL                 (IMS_MSG_THREAD_BASE + 3)
// For special thread specific message definitions
#define IMS_MSG_THREAD_INTERNAL_BASE        (IMS_MSG_THREAD_BASE + 101)
// For thread user message definitions - callback handling
#define IMS_MSG_THREAD_USER_BASE            (IMS_MSG_THREAD_BASE + 201)

//// SYSTEM_BASE
// Event for Packet Data Connection
#define IMS_MSG_NETWORK                     (IMS_MSG_SYSTEM_BASE + 1)
// Event for Socket
#define IMS_MSG_SOCKET                      (IMS_MSG_SYSTEM_BASE + 2)
// Event for Timer
#define IMS_MSG_TIMER                       (IMS_MSG_SYSTEM_BASE + 3)
// Event for Battery
#define IMS_MSG_BATTERY                     (IMS_MSG_SYSTEM_BASE + 4)
// Event for mobile network status
#define IMS_MSG_NETWORK_STATUS              (IMS_MSG_SYSTEM_BASE + 5)
// Event for Wi-Fi network status
#define IMS_MSG_WIFI_STATUS                 (IMS_MSG_SYSTEM_BASE + 6)
// Event for ISIM
#define IMS_MSG_ISIM                        (IMS_MSG_SYSTEM_BASE + 7)
// Event for USIM
#define IMS_MSG_USIM                        (IMS_MSG_SYSTEM_BASE + 8)
// Event for Configuration
#define IMS_MSG_CONFIGURATION               (IMS_MSG_SYSTEM_BASE + 9)
// Event for TRM service priority status
#define IMS_MSG_TRM_PRIORITY_STATUS         (IMS_MSG_SYSTEM_BASE + 10)
// Event for VoNR
#define IMS_MSG_VONR                        (IMS_MSG_SYSTEM_BASE + 11)

#define IMS_MSG_SYSTEM_MAX                  (IMS_MSG_SYSTEM_BASE + 100)

//// APP_BASE
#define IMS_MSG_BASE_SERVICE                (IMS_MSG_APP_BASE + 0)
#define IMS_MSG_BASE_SESSION                (IMS_MSG_APP_BASE + 200)
//1400
#define IMS_MSG_BASE_STREAMEDMEDIA          (IMS_MSG_APP_BASE + 400)
#define IMS_MSG_BASE_FRAMEDMEDIA            (IMS_MSG_APP_BASE + 600)
#define IMS_MSG_BASE_UCE                    (IMS_MSG_APP_BASE + 2000)//3000

//// APP_INTERNAL
// Event for IMS application(for internal message)
#define IMS_MSG_UC                          (IMS_MSG_APP_INTERNAL + 2000)
#define IMS_MSG_MTC                         (IMS_MSG_APP_INTERNAL + 2000)
#define IMS_MSG_AOS                         (IMS_MSG_APP_INTERNAL + 5000)

#define IMS_MSG_XML                         (IMS_MSG_APP_INTERNAL + 5200)
#define IMS_MSG_MMTEL_PROV                  (IMS_MSG_APP_INTERNAL + 5300)
#define IMS_MSG_CIQ                         (IMS_MSG_APP_INTERNAL + 5400)
#define IMS_MSG_UCE                         (IMS_MSG_APP_INTERNAL + 5500)
#define IMS_MSG_EAB_XDM                     (IMS_MSG_APP_INTERNAL + 5600)
#define IMS_MSG_SMS                         (IMS_MSG_APP_INTERNAL + 5700)
#define IMS_MSG_PRES                        (IMS_MSG_APP_INTERNAL + 5800)

//// USER_BASE
#define IMS_MSG_USER                        (IMS_MSG_USER_BASE + 0)

#endif // _IMS_MSGDEF_H_
