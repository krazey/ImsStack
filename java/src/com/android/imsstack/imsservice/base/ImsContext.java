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
package com.android.imsstack.imsservice.base;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.util.MessageExecutor;

import java.util.concurrent.Executor;

public class ImsContext implements IContext {
    private final Context mContext;
    private final MessageExecutor mExecutor;
    private final int mSlotId;

    public ImsContext(Context context, MessageExecutor executor, int slotId) {
        mContext = context;
        mExecutor = executor;
        mSlotId = slotId;
    }

    /**
     * Utilities for this context.
     */
    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public Executor getExecutor() {
        return mExecutor;
    }

    @Override
    public Handler getDefaultHandler() {
        return AppContext.getInstance().getMainHandler();
    }

    @Override
    public Looper getDefaultLooper() {
        return AppContext.getInstance().getMainLooper();
    }

    /**
     * Returns the phone id of this context.
     */
    @Override
    public int getPhoneId() {
        // FIXME: phoneId - same as slotId
        return getSlotId();
    }

    /**
     * Returns the slot-id of this context.
     */
    @Override
    public int getSlotId() {
        return mSlotId;
    }

    /**
     * Returns the subscription id of this context.
     */
    @Override
    public int getSubId() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        return (sim != null) ? sim.getSubId() : MSimUtils.getSubId(getPhoneId());
    }

    /**
     * Returns the looper associated with handler.
     */
    @Override
    public Looper getLooper() {
        return mExecutor.getLooper();
    }
}
