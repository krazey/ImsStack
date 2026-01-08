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
package com.android.imsstack.util;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import java.util.concurrent.Executor;

/**
 * Executes the tasks in the other thread rather than the calling thread.
 */
public class MessageExecutor extends Handler implements Executor {
    public MessageExecutor(String name) {
        this(createLooper(name));
    }

    public MessageExecutor(Looper looper) {
        super(looper);
    }

    @Override
    public void execute(Runnable r) {
        Message m = Message.obtain(this, 0 /* don't care */, r);
        m.sendToTarget();
    }

    @Override
    public void handleMessage(Message msg) {
        if (msg.obj instanceof Runnable) {
            executeInternal((Runnable) msg.obj);
        } else {
            Log.d(this, "handleMessage: Not runnable object - " + msg);
        }
    }

    private void executeInternal(Runnable r) {
        try {
            r.run();
        } catch (Throwable t) {
            Log.e(this, "executeInternal: " + r);
            t.printStackTrace();
        }
    }

    private static Looper createLooper(String name) {
        HandlerThread thread = new HandlerThread(name);
        thread.start();
        Looper looper = thread.getLooper();
        Log.d(MessageExecutor.class, "createLooper: name=" + name
                + ", tid=" + thread.getThreadId());
        return looper;
    }
}
