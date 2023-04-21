/*
 * Copyright (C) 2023 The Android Open Source Project
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

package com.android.imsstack.core.agents;

import com.android.imsstack.system.DefaultSystemCallInterface;
import com.android.imsstack.system.SystemInterface;

/**
 * An agent class to handle the default system calls.
 */
public class DefaultSystemCallAgent implements DefaultSystemCallInterface {

    public DefaultSystemCallAgent() {
        SystemInterface.getInstance().setSystemCallInterface(this);
    }

    /**
     * Destroys this agent - clean up the internal resources.
     */
    public void destroy() {
        SystemInterface.getInstance().setSystemCallInterface(null);
    }

    /**
     * Starts a timer with the specified duration for the native service.
     *
     * @param tid The timer id to be started.
     * @param duration The timer duration as milli-seconds.
     * @return {@code true} if a timer is successfully started, {@code false} otherwise.
     */
    @Override
    public boolean startTimer(long tid, long duration) {
        TimerAgent ta = (TimerAgent) AgentFactory.getInstance().getAgent(TimerInterface.class);
        if (ta != null) {
            return ta.startNativeTimer(tid, duration);
        }
        return false;
    }

    /**
     * Stops the specified timer for the native service.
     *
     * @param tid The timer id to be stopped.
     */
    @Override
    public void stopTimer(long tid) {
        TimerAgent ta = (TimerAgent) AgentFactory.getInstance().getAgent(TimerInterface.class);
        if (ta != null) {
            ta.stopNativeTimer(tid);
        }
    }

    /**
     * Returns the Wi-Fi interface.
     *
     * @return A WifiInterface object.
     */
    @Override
    public WifiInterface getWifiInterface() {
        return AgentFactory.getInstance().getAgent(WifiInterface.class);
    }
}
