/*
 * Copyright (C) 2025 The Android Open Source Project
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
#ifndef ASYNC_EXECUTOR_H_
#define ASYNC_EXECUTOR_H_

#include "IThread.h"
#include "ImsMessage.h"
#include "ImsMessageDef.h"

/**
 * @brief An abstract helper class to execute a single task asynchronously.
 *
 * This class finds and holds the owner thread inside ImsStack at the time of creation, and
 * performs asynchronous operations though that thread.
 *
 * If the caller only want to process one event and want the object to be automatically destroyed,
 * set the bAutoDestroy to true when instantiating the object so that it's automatically destroyed
 * after one execution.
 */
class AsyncExecutor : public ImsMessage::IMessageCallback
{
public:
    /**
     * @brief A listener interface to notify the application that a task has been completed.
     */
    class IListener
    {
    protected:
        virtual ~IListener() = default;

    public:
        /**
         * @brief Called when a task is completed.
         *
         * NOTE: This is called after OnExecute method is processed.
         */
        virtual void AsyncExecutor_OnExecuteCompleted(IN AsyncExecutor* pExecutor) = 0;
    };

    /**
     * @brief An executor interface to run a task asynchronously.
     */
    class IExecutor
    {
    protected:
        virtual ~IExecutor() = default;

    public:
        /**
         * @brief Called when a task is completed.
         *
         * NOTE: This is called after OnExecute method is processed.
         */
        virtual void AsyncExecutor_OnExecute(
                IN AsyncExecutor* pExecutor, IN IMS_UINTP nParam1, IN IMS_UINTP nParam2) = 0;
    };

public:
    explicit AsyncExecutor(IN IMS_BOOL bAutoDestroy);
    AsyncExecutor(IN IListener* piListener, IN IMS_BOOL bAutoDestroy);
    AsyncExecutor(IN IThread* piOwnerThread, IN IMS_BOOL bAutoDestroy);
    AsyncExecutor(IN IThread* piOwnerThread, IN IListener* piListener, IN IMS_BOOL bAutoDestroy);
    virtual ~AsyncExecutor();

    AsyncExecutor(IN const AsyncExecutor&) = delete;
    AsyncExecutor& operator=(IN const AsyncExecutor&) = delete;

public:
    /**
     * @brief Execute a task asynchronously.
     */
    inline void Execute() { Execute(0, 0); }

    /**
     * @brief Execute a task asynchronously.
     *
     * The input arguments are passed as the parameter values of ImsMessage of OnExecute method.
     */
    void Execute(IN IMS_UINTP nParam1, IN IMS_UINTP nParam2);

    /**
     * @brief Destroy this object asynchronously.
     */
    void Destroy();

    /**
     * @brief Set the IExecutor interface.
     */
    inline void SetExecutor(IN IExecutor* piExecutor) { m_piExecutor = piExecutor; }

protected:
    void MessageCallback_OnMessage(IN ImsMessage& objMsg) override final;

    // As a default implementation, if IExecutor exists, it will be invoked.
    inline virtual void OnExecute(IN IMS_UINTP nParam1, IN IMS_UINTP nParam2)
    {
        if (m_piExecutor != IMS_NULL)
        {
            m_piExecutor->AsyncExecutor_OnExecute(this, nParam1, nParam2);
        }
    }

protected:
    static constexpr IMS_SINT32 MSG_DESTROY = IMS_MSG_USER;
    static constexpr IMS_SINT32 MSG_EXECUTE = IMS_MSG_USER + 1;

private:
    IMS_BOOL m_bAutoDestroy;
    IThread* m_piOwnerThread;
    IExecutor* m_piExecutor;
    IListener* m_piListener;
};

#endif
