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

package com.android.imsstack.enabler.uce.impl;

import android.content.Context;
import android.net.Uri;

import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.enabler.uce.interf.OptionsResponse;
import com.android.imsstack.enabler.uce.interf.PublishResponse;
import com.android.imsstack.enabler.uce.interf.SubscribeResponse;
import com.android.imsstack.enabler.uce.interf.UceEventListener;
import com.android.imsstack.util.ImsLog;

import java.util.Collection;
import java.util.Set;

public class UceImpl implements IUceApi{

    private UceAgent mUceAgent = null;

    public UceImpl(Context context, int nSimSlot){
        ImsLog.d(nSimSlot, "Create UceImpl");
        mUceAgent = new UceAgent(context, "UceAgentThread" + nSimSlot, nSimSlot);
        if (mUceAgent.isAlive() == false) {
            ImsLog.d(nSimSlot, "UceAgent Thread isn't alive");
            mUceAgent.start();
        }
    }

    @Override
    public void setListener(UceEventListener listener) {
        mUceAgent.setListener(listener);
    }

    @Override
    public void carrierConfigChanged() {
        mUceAgent.carrierConfigChanged();
    }

    @Override
    public void publishCapabilities(String pidfXml, PublishResponse cb) {
        mUceAgent.publishCapabilities(pidfXml, cb);
    }

    @Override
    public void subscribeCapabilities(Collection<Uri> uris, SubscribeResponse cb) {
        mUceAgent.subscribeCapabilities(uris, cb);
    }

    @Override
    public void sendOptionsCapabilityRequest(Uri contactUri, Set<String> myCapabilities,
        OptionsResponse cb) {
        mUceAgent.sendOptionsCapabilityRequest(contactUri, myCapabilities, cb);
    }

    public void release() {
        if (mUceAgent != null) {
            mUceAgent.release();
        }
    }
}
