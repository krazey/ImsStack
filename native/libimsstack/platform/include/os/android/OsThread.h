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
#ifndef OS_THREAD_H_
#define OS_THREAD_H_

#include <pthread.h>

#include "ImsThread.h"
#include "ImsVector.h"
#include "OsMutex.h"

class OsThread : public ImsThread
{
public:
    OsThread();
    virtual ~OsThread();

    OsThread(IN const OsThread&) = delete;
    OsThread& operator=(IN const OsThread&) = delete;

public:
    // IThread class
    IMS_BOOL Activate() override;
    void Deactivate() override;
    IMS_BOOL Equals(IN const IThread* piThread) const override;
    inline const AString& GetName() const override { return m_strName; }
    inline IMS_BOOL IsRunning() const override { return m_bIsRunning; }
    IMS_BOOL PostMessageI(IN ImsMessage& objMsg) override;
    IMS_BOOL PostMessageI(IN IMS_UINT32 nMsg, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;
    inline void SetRunnable(IN IRunnable* piListener) override { m_piListener = piListener; }
    IMS_SINT32 RemoveMessages(IN ImsMessage::IMessageCallback* piCallback,
            OUT ImsList<ImsMessage>* pImsMsgs = IMS_NULL) override;

    // ImsThread class
    inline IMS_BOOL Create(IN const AString& strName) override
    {
        m_strName = strName;
        return IMS_TRUE;
    }
    inline IMS_ULONG GetThreadId() const override { return static_cast<IMS_ULONG>(m_nThreadId); }
    static IMS_ULONG GetCurrentThreadId();

    virtual IMS_ULONG Run();
    virtual void PostMessage(IN IMS_UINT32 nMessage);

protected:
    virtual void OnStart(IN ImsMessage& objMsg);
    virtual void OnTerminate(IN ImsMessage& objMsg);
    virtual void OnSystemMessage(IN ImsMessage& objMsg);
    virtual void OnThreadMessage(IN ImsMessage& objMsg);

    // Overridable methods for accessing "pthread_xxx" functions
    /**
     * @brief Creates a pthread.
     *
     * @return Returns a thread identifier or 0 if thread creation is failed.
     */
    virtual pthread_t CreateThread();
    /**
     * @brief Waits for terminating this thread.
     */
    virtual void JoinThread();
    /**
     * @brief Sends a condition signal to this thread.
     * @return IMS_TRUE if the signal is successfully sent, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL SendSignal();
    /**
     * @brief Waits for thread wake up by a condition signal or timeout.
     *
     * @param nMsgCount The message count of this thread's message queue
     * @return The result of condition waiting.
     */
    virtual IMS_SINT32 WaitForSignal(IN IMS_SINT32 nMsgCount);

    static IMS_BOOL IsSystemMessage(IN IMS_SINT32 nMsg);

private:
    // Internal signal flag to avoid timing issue
    inline void SetSignal() { m_bSignalFlag = IMS_TRUE; }
    inline void ClearSignal() { m_bSignalFlag = IMS_FALSE; }
    inline IMS_BOOL IsSignaled() const { return m_bSignalFlag; }

    IMS_SINT32 RemoveMessages(IN_OUT ImsVector<ImsMessage>& objMsgQueue,
            IN IMS_SINT32 nStartingIndex, IN ImsMessage::IMessageCallback* piCallback,
            OUT ImsList<ImsMessage>* pImsMsgs);

private:
    // Name of this thread
    AString m_strName;
    pthread_t m_nThreadId;
    IMS_BOOL m_bIsRunning;

    // Signal condition
    pthread_cond_t m_stCond;
    OsMutex m_objCondMutex;
    IMS_BOOL m_bSignalFlag;

    // Message queue
    ImsVector<ImsMessage> m_objMsgQueue;
    OsMutex m_objMsgQueueMutex;

    // A message list that is currently processing
    ImsVector<ImsMessage> m_objProcessingMsgs;
    OsMutex m_objProcessingMsgsMutex;
    // Track the index of the message being processed.
    // Used as the starting position for m_objProcessingMsgs when requesting a removal operation
    // for the messages that have already been posted but have not been processed.
    IMS_SINT32 m_nProcessingMsgIndex;

    IRunnable* m_piListener;
};

#endif
