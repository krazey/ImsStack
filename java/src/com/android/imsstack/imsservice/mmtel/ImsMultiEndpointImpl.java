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

package com.android.imsstack.imsservice.mmtel;

import android.telephony.ims.ImsExternalCallState;
import android.telephony.ims.stub.ImsMultiEndpointImplBase;

import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import java.util.List;

/**
  * Implementation of ImsMultiEndpointImplBase class.
  */
public class ImsMultiEndpointImpl extends ImsMultiEndpointImplBase {
    private List<ImsExternalCallState> mImsExternalCallStates;

    public ImsMultiEndpointImpl(ICallContext callContext) {
    }

    /**
     * Disposes the object
     */
    public void dispose() {
        mImsExternalCallStates = null;
    }

    /**
     * Notifies framework when Dialog Event Package update is received from native.
     * Notification is sent to framework via callback method.
     */
    public void updateDialogState(List<ImsExternalCallState> imsExternalCallStates) {
        mImsExternalCallStates = imsExternalCallStates;
        onImsExternalCallStateUpdate(imsExternalCallStates);
    }

    /**
     * ImsExternalCallState is cached.
     */
    private List<ImsExternalCallState> getImsExternalCallState() {
        return mImsExternalCallStates;
    }

    /**
     * Framework will trigger this to get the latest Dialog Event Package information.
     * Notification is sent to framework via callback method.
     */
    @Override
    public void requestImsExternalCallStateInfo() {
        final List<ImsExternalCallState> imsExternalCallState = getImsExternalCallState();
        onImsExternalCallStateUpdate(imsExternalCallState);
    }
}
