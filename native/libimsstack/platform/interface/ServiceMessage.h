/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SERVICE_MESSAGE_H_
#define SERVICE_MESSAGE_H_

#include "AString.h"
#include "ImsMessage.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

class IThread;
class IMSActivity;

class MessageService
{
public:
    MessageService(IN const MessageService&) = delete;
    MessageService& operator=(IN const MessageService&) = delete;

public:
    static IMS_BOOL PostMessage(IN const AString& strTarget, IN ImsMessage& objMsg);

    static IMS_BOOL PostMessageThread(IN IThread* piTargetThread, IN ImsMessage& objMsg);

    static IMS_BOOL PostMessageActivity(IN IMSActivity* pTargetActivity, IN ImsMessage& objMsg);

private:
    static AString GetThreadName(IN const AString& strTargetName);
};

#define IMS_MSG_PostThreadMessage(THREAD, MESSAGE) \
        MessageService::PostMessageThread(THREAD, MESSAGE)

#define IMS_MSG_PostThreadMessageByName(TARGET, MESSAGE) \
        MessageService::PostMessage(TARGET, MESSAGE)

#ifdef __IMS_MSG_TRACE__

#define IMS_MSG_CreateNPostThreadMessage(THREAD, MSG, WPARAM, LPARAM) \
do\
{\
    ImsMessage objMsg(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(WPARAM), (IMS_UINTP)(LPARAM));\
    MessageService::PostMessageThread(THREAD, objMsg);\
    IMS_TRACE_I("MessageService::PostThreadMessage >> [%s] [%d]", #MSG, MSG, 0);\
} while (0)

#define IMS_MSG_CreateNPostThreadMessageByName(TARGET, MSG, WPARAM, LPARAM) \
do\
{\
    ImsMessage objMsg(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(WPARAM), (IMS_UINTP)(LPARAM));\
    MessageService::PostMessage(TARGET, objMsg);\
    IMS_TRACE_I("MessageService::PostThreadMessage >> [%s] [%d]", #MSG, MSG, 0);\
} while (0)

#else

#define IMS_MSG_CreateNPostThreadMessage(THREAD, MSG, WPARAM, LPARAM) \
do\
{\
    ImsMessage objMsg(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(WPARAM), (IMS_UINTP)(LPARAM));\
    MessageService::PostMessageThread(THREAD, objMsg);\
} while (0)

#define IMS_MSG_CreateNPostThreadMessageByName(TARGET, MSG, WPARAM, LPARAM) \
do\
{\
    ImsMessage objMsg(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(WPARAM), (IMS_UINTP)(LPARAM));\
    MessageService::PostMessage(TARGET, objMsg);\
} while (0)

#endif // __IMS_MSG_TRACE__

#define IMS_MSG_PostActivityMessage(TARGET, MESSAGE) \
        MessageService::PostMessageActivity(TARGET, MESSAGE)

#define IMS_MSG_PostActivityMessageByName(TARGET, MESSAGE) \
        MessageService::PostMessage(TARGET, MESSAGE)

#ifdef __IMS_MSG_TRACE__

#define IMS_MSG_CreateNPostActivityMessage(TARGET, MSG, WPARAM, LPARAM) \
do\
{\
    ImsMessage objMsg(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(WPARAM), (IMS_UINTP)(LPARAM));\
    MessageService::PostActivityMessage(TARGET, objMsg);\
    IMS_TRACE_I("MessageService::PostActivityMessage >> [%s] [%d]", #MSG, MSG, 0);\
} while (0)

#define IMS_MSG_CreateNPostActivityMessageByName(TARGET, MSG, WPARAM, LPARAM) \
do\
{\
    ImsMessage objMsg(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(WPARAM), (IMS_UINTP)(LPARAM));\
    MessageService::PostMessage(TARGET, objMsg);\
    IMS_TRACE_I("MessageService::PostActivityMessage >> [%s] [%d]", #MSG, MSG, 0);\
} while (0)

#else

#define IMS_MSG_CreateNPostActivityMessage(TARGET, MSG, WPARAM, LPARAM) \
do\
{\
    ImsMessage objMsg(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(WPARAM), (IMS_UINTP)(LPARAM));\
    MessageService::PostMessageActivity(TARGET, objMsg);\
} while (0)

#define IMS_MSG_CreateNPostActivityMessageByName(TARGET, MSG, WPARAM, LPARAM) \
do\
{\
    ImsMessage objMsg(static_cast<IMS_SINT32>(MSG), (IMS_UINTP)(WPARAM), (IMS_UINTP)(LPARAM));\
    MessageService::PostMessage(TARGET, objMsg);\
} while (0)

#endif // __IMS_MSG_TRACE__

#endif
