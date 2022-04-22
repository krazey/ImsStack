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

#ifndef _IMS_CONST_DEF_H_
#define _IMS_CONST_DEF_H_

/**
 * N : /data/data/[package-name]
 * N-MR1 (Device Encrypted storage) : /data/user_de/0/[package-name]
 */
#define IMS_SOLUTION_STORAGE_ROOT_DIR "/data/user_de/0/com.android.imsstack"
#define IMS_SOLUTION_IMS_CONFIG_DB (IMS_SOLUTION_STORAGE_ROOT_DIR "/databases/gims.db")

#define IMS_SOLUTION_ID_LEN                         64
#define IMS_SOLUTION_MSG_SOURCE_LEN                 64
#define IMS_SOLUTION_MSISDN_LEN                     45
#define IMS_SOLUTION_MDN_LEN                        20
#define IMS_SOLUTION_DISPLAY_NAME_LEN               80
#define IMS_SOLUTION_RESOURCELIST_LEN               10

#define IMS_SOLUTION_FILETYPE_LEN                   16
#define IMS_SOLUTION_FILEPURENAME_LEN               64
#define IMS_SOLUTION_FILENAME_LEN                   300
#define IMS_SOLUTION_FILE_NAME_LEN                  300
#define IMS_SOLUTION_TIME_LEN                       128

#define IMS_SOLUTION_HOMEPAGE                       256
#define IMS_SOLUTION_URI_LEN                        128
#define IMS_SOLUTION_ETAG_LEN                       128
#define IMS_SOLUTION_MEMO_LEN                       100
#define IMS_SOLUTION_EMAILADDRESS                   256
#define IMS_SOLUTION_BIRTHDAY                       20
#define IMS_SOLUTION_ACCOUNT                        256
#define IMS_SOLUTION_HTTPURL_LEN                    256

#define IMS_SOLUTION_SESSIONID_LEN                  128

#define IMS_SOLUTION_CONTENTTYPE_LEN                128
#define IMS_SOLUTION_MESSAGE_LEN                    10240
#define IMS_SOLUTION_THUMBNAIL_LEN                  16000
#define IMS_SOLUTION_PIDF_LEN                       2048
#define IMS_SOLUTION_REPLY_TIMER_VAL_LEN            20
#define IMS_SOLUTION_CB_STRING_LEN                  10
#define IMS_SOLUTION_DTMF_LEN                       64

#define IMS_SOLUTION_VERSION_LEN                    4

#define IMS_SOLUTION_SERVICE_ID_LEN                 80
#define IMS_SOLUTION_SERVICE_VERSION_LEN             4

// This definition will be provided regardless of IP version.
/* 128bit -> 21DA:00D3:0000:2F3B:02AA:00FF:FE28:9C5A = 39byte+1(null)byte */
#define IMS_SOLUTION_IP_LEN                         39

#define IMS_SOLUTION_256                            256

#endif // _IMS_CONST_DEF_H_
