/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#ifndef _SERVICE_IMS_MSG_H_
#define _SERVICE_IMS_MSG_H_

#include "ServiceMemory.h"

#include "ServiceTrace.h"
#include "AString.h"
#include "ImsMessage.h"

class IThread;
class IMSActivity;

class MessageService
{
public:
    static IMS_BOOL PostMessage(IN const AString &strTarget, IN ImsMessage &objMSG);

    static IMS_BOOL PostMessageThread(IN IThread *piTargetThread, IN ImsMessage &objMSG);

    static IMS_BOOL PostMessageActivity(IN IMSActivity *pTargetActivity, IN ImsMessage &objMSG);

private:
    static AString GetThreadName(IN const AString& strTargetName);
};

//-------------------------------------------------------------------------------------------------

#define IMS_MSG_PostThreadMessage(piThread, objMSG) \
        MessageService::PostMessageThread(piThread, objMSG)

#define IMS_MSG_PostThreadMessageByName(strTarget, objMSG) \
        MessageService::PostMessage(strTarget, objMSG)

//-------------------------------------------------------------------------------------------------

#ifdef __IMS_MSG_TRACE__

#define IMS_MSG_CreateNPostThreadMessage(piThread, MSG, wParam, lParam) \
do \
{ \
    ImsMessage objMSG(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(wParam), (IMS_UINTP)(lParam)); \
    MessageService::PostMessageThread(piThread, objMSG); \
    IMS_TRACE_I("MessageService::PostThreadMessage >> [%s] [%d]", #MSG, MSG, 0); \
} while (0)

#define IMS_MSG_CreateNPostThreadMessageByName(strTarget, MSG, wParam, lParam) \
do \
{ \
    ImsMessage objMSG(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(wParam), (IMS_UINTP)(lParam)); \
    MessageService::PostMessage(strTarget, objMSG); \
    IMS_TRACE_I("MessageService::PostThreadMessage >> [%s] [%d]", #MSG, MSG, 0); \
} while (0)

#else

#define IMS_MSG_CreateNPostThreadMessage(piThread, MSG, wParam, lParam) \
do \
{ \
    ImsMessage objMSG(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(wParam), (IMS_UINTP)(lParam)); \
    MessageService::PostMessageThread(piThread, objMSG); \
} while (0)

#define IMS_MSG_CreateNPostThreadMessageByName(strTarget, MSG, wParam, lParam) \
do \
{ \
    ImsMessage objMSG(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(wParam), (IMS_UINTP)(lParam)); \
    MessageService::PostMessage(strTarget, objMSG); \
} while (0)

#endif // __IMS_MSG_TRACE__

//-------------------------------------------------------------------------------------------------

#define IMS_MSG_PostActivityMessage(pTarget, objMSG) \
        MessageService::PostMessageActivity(pTarget, objMSG)

#define IMS_MSG_PostActivityMessageByName(strTarget, objMSG) \
        MessageService::PostMessage(strTarget, objMSG)

//-------------------------------------------------------------------------------------------------

#ifdef __IMS_MSG_TRACE__

#define IMS_MSG_CreateNPostActivityMessage(pTarget, MSG, wParam, lParam) \
do \
{ \
    ImsMessage objMSG(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(wParam), (IMS_UINTP)(lParam)); \
    MessageService::PostActivityMessage(pTarget, objMSG); \
    IMS_TRACE_I("MessageService::PostActivityMessage >> [%s] [%d]", #MSG, MSG, 0); \
} while (0)

#define IMS_MSG_CreateNPostActivityMessageByName(strTarget, MSG, wParam, lParam) \
do \
{ \
    ImsMessage objMSG(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(wParam), (IMS_UINTP)(lParam)); \
    MessageService::PostMessage(strTarget, objMSG); \
    IMS_TRACE_I("MessageService::PostActivityMessage >> [%s] [%d]", #MSG, MSG, 0); \
} while (0)

#else

#define IMS_MSG_CreateNPostActivityMessage(pTarget, MSG, wParam, lParam) \
do \
{ \
    ImsMessage objMSG(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(wParam), (IMS_UINTP)(lParam)); \
    MessageService::PostMessageActivity(pTarget, objMSG); \
} while (0)

#define IMS_MSG_CreateNPostActivityMessageByName(strTarget, MSG, wParam, lParam) \
do \
{ \
    ImsMessage objMSG(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(wParam), (IMS_UINTP)(lParam)); \
    MessageService::PostMessage(strTarget, objMSG); \
} while (0)

#endif // __IMS_MSG_TRACE__

#endif // _SERVICE_IMS_MSG_H_
