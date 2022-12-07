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
package com.android.imsstack.core.agents;

import android.content.Context;

import com.android.imsstack.util.MSimUtils;

public final class LocationAgentManager implements ILocationAgentManager {
    private final Context mContext;
    private final LocationAgent[] mAgents = new LocationAgent[MSimUtils.getSupportedSimCount()];

    public LocationAgentManager(Context context) {
        mContext = context;
    }

    @Override
    public void cleanup() {
        synchronized (mAgents) {
            for (int i = 0; i < mAgents.length; i++) {
                if (mAgents[i] != null) {
                    mAgents[i].dispose();
                    mAgents[i] = null;
                }
            }
        }
    }

    @Override
    public void start(int slotId) {
        // LocationAgent will be instantiated by init(...) method.
    }

    @Override
    public void stop(int slotId) {
        if (slotId < 0 || slotId >= mAgents.length) {
            return;
        }

        synchronized (mAgents) {
            if (mAgents[slotId] != null) {
                mAgents[slotId].dispose();
                mAgents[slotId] = null;
            }
        }
    }

    @Override
    public ILocationAgent getAgent(int slotId) {
        if (slotId < 0 || slotId >= mAgents.length) {
            return null;
        }

        synchronized (mAgents) {
            if (mAgents[slotId] == null) {
                mAgents[slotId] = new LocationAgent(mContext, slotId);
            }

            return mAgents[slotId];
        }
    }
}
