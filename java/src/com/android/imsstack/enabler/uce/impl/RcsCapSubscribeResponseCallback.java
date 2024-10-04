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

import android.net.Uri;
import android.telephony.ims.ImsException;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase.SubscribeResponseCallback;
import android.util.Log;
import android.util.Pair;

import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.enabler.uce.interf.SubscribeResponse;

import java.util.List;
import java.util.concurrent.Executor;

public class RcsCapSubscribeResponseCallback implements SubscribeResponse {

    private static final String LOG_TAG = RcsCapSubscribeResponseCallback.class.getSimpleName();
    private SubscribeResponseCallback mSubscribeResponseCallBack;
    private final Executor mMessageExecutor;

    public RcsCapSubscribeResponseCallback(Executor messageExecutor) {
        mMessageExecutor = messageExecutor;
    }

    /**
     * set callback SubscribeResponseCallback of subscribe request
     * to send the response from the network back to the framework
     *
     * @param subscribeCallback The callback of the subscribe request
     */
    public void setCallback(SubscribeResponseCallback subscribeCallback) {
        mSubscribeResponseCallBack = subscribeCallback;
    }

    /**
     * Notify the framework of the latest XML PIDF documents included in the network response
     * for the requested contacts' capabilities requested by the Framework.
     * The expected format for the PIDF XML is defined in RFC3861. Each XML document must be an
     * "application/pidf+xml" object and start with a root presence element. For NOTIFY
     * responses that contain RLMI information and potentially multiple PIDF XMLs, each
     * PIDF XML should be separated and added as a separate item in the List. This should be
     * called every time a new NOTIFY event is received with new capability information.
     *
     * @param pidfXmls The list of the PIDF XML data for the contact URIs that it subscribe for.
     */
    @Override
    public void onNotifyCapabilitiesUpdate(List<String> pidfXmls) {
        if (mSubscribeResponseCallBack == null) {
            Log.d(LOG_TAG, "SubscribeResponseCallBack  object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mSubscribeResponseCallBack.onNotifyCapabilitiesUpdate(pidfXmls);
                Log.d(LOG_TAG, "onNotifyCapabilitiesUpdate pidfXmls is sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onNotifyCapabilitiesUpdate exception = " + e.toString());
            }
        });
    }

    /**
     * Notify the framework that the command associated with this callback has failed.
     *
     * @param code The reason why the associated command has failed.
     * COMMAND_CODE_SERVICE_UNKNOWN = 0 (Service is unknown)
     * COMMAND_CODE_GENERIC_FAILURE = 1 (The command failed with an unknown error)
     * COMMAND_CODE_INVALID_PARAM = 2 (Invalid parameter(s))
     * COMMAND_CODE_FETCH_ERROR = 3 (Fetch error)
     * COMMAND_CODE_REQUEST_TIMEOUT = 4 (Request timed out)
     * COMMAND_CODE_INSUFFICIENT_MEMORY = 5 (Failure due to insufficient memory available)
     * COMMAND_CODE_LOST_NETWORK_CONNECTION = 6 (Network connection is lost)
     * COMMAND_CODE_NOT_SUPPORTED = 7 (Requested feature/resource is not supported)
     * COMMAND_CODE_NOT_FOUND = 8 (Contact or resource is not found)
     * COMMAND_CODE_SERVICE_UNAVAILABLE = 9 (Service is not available)
     * COMMAND_CODE_NO_CHANGE = 10 (Command resulted in no change in state, ignoring)
     */
    @Override
    public void onCommandError(int code) {
        if (mSubscribeResponseCallBack == null) {
            Log.d(LOG_TAG, "SubscribeResponseCallBack  object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mSubscribeResponseCallBack.onCommandError(code);
                Log.d(LOG_TAG, "onCommandError SubscribeResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onCommandError exception = " + e.toString());
            }
        });
    }

    /**
     * Notify the framework of the response to the SUBSCRIBE request from
     * {@link IUceApi#subscribeCapabilities(Collection, SubscribeResponse)}.
     * If the carrier network responds to the SUBSCRIBE request with a 2XX response, then the
     * framework will expect the IMS stack to call {@link #onNotifyCapabilitiesUpdate},
     * {@link #onResourceTerminated}, and {@link #onTerminated} as required for the
     * subsequent NOTIFY responses to the subscription.
     *
     * If this network response also contains a “Reason” header, then the
     * {@link #onNetworkResponse(int, String, int, String)} method should be used instead.
     *
     * @param sipCode The SIP response code sent from the network for the operation
     * token specified.
     * @param reason The optional reason response from the network. If the network
     * provided no reason with the sip code, the string should be empty.
     */
    @Override
    public void onNetworkResponse(int sipCode, String reason) {
        if (mSubscribeResponseCallBack == null) {
            Log.d(LOG_TAG, "SubscribeResponseCallBack  object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mSubscribeResponseCallBack.onNetworkResponse(sipCode, reason);
                Log.d(LOG_TAG, "onNetworkResponse SubscribeResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onNetworkResponse exception = " + e.toString());
            }
        });
    }

    /**
     * Notify the framework  of the response to the SUBSCRIBE request from
     * {@link IUceApi#subscribeCapabilities(Collection, SubscribeResponse)} that also
     * includes a reason provided in the “reason” header. See RFC3326 for more
     * information.
     *
     * @param sipCode The SIP response code sent from the network,
     * @param reason The optional reason response from the network. If the
     * network provided no reason with the sip code, the string should be empty.
     * @param reasonHeaderCause The “cause” parameter of the “reason” header
     * included in the SIP message.
     * @param reasonHeaderText The “text” parameter of the “reason” header
     * included in the SIP message.
     */
    @Override
    public void onNetworkResponse(int sipCode, String reason, int reasonHeaderCause,
            String reasonHeaderText) {
        if (mSubscribeResponseCallBack == null) {
            Log.d(LOG_TAG, "SubscribeResponseCallBack  object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mSubscribeResponseCallBack.onNetworkResponse(sipCode, reason, reasonHeaderCause,
                        reasonHeaderText);
                Log.d(LOG_TAG, "onNetworkResponse SubscribeResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onNetworkResponse exception = " + e.toString());
            }
        });
    }

    /**
     * The subscription associated operation has been terminated.
     * This will mostly be due to the network sending a final
     * NOTIFY response due to the subscription expiring, but this may also happen due to a
     * network error.
     *
     * @param reason the reason for the request being unable to process.
     * @param retryAfterMilliseconds The time in milliseconds the requesting application should
     * wait before retrying, if non-zero.
     */
    @Override
    public void onTerminated(String reason, long retryAfterMilliseconds) {
        if (mSubscribeResponseCallBack == null) {
            Log.d(LOG_TAG, "SubscribeResponseCallBack  object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mSubscribeResponseCallBack.onTerminated(reason, retryAfterMilliseconds);
                Log.d(LOG_TAG, "onTerminated SubscribeResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onTerminated exception = " + e.toString());
            }
        });
    }

    /**
     * Notify the framework that a resource in the RLMI XML contained in the NOTIFY response
     * for the ongoing SUBSCRIBE dialog has been terminated.
     * This will be used to notify the framework that a contact URI that the IMS stack has
     * subscribed to on the Resource List Server has been terminated as well as the reason why.
     * Usually this means that there will not be any capability information for the contact URI
     * that they subscribed for. See RFC 4662 for more information.
     *
     * @param uriTerminatedReason The contact URIs which have been terminated. Each pair in the
     * list is the contact URI and its terminated reason.
     */
    @Override
    public void onResourceTerminated(List<Pair<Uri, String>> uriTerminatedReason) {
        if (mSubscribeResponseCallBack == null) {
            Log.d(LOG_TAG, "SubscribeResponseCallBack  object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mSubscribeResponseCallBack.onResourceTerminated(uriTerminatedReason);
                Log.d(LOG_TAG, "onResourceTerminated SubscribeResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onResourceTerminated exception = " + e.toString());
            }
        });
    }

    private void postAndRunTask(Runnable task) {
        if (mMessageExecutor != null) {
            mMessageExecutor.execute(task);
        } else {
            Log.d(LOG_TAG, "MessageExecutor object is null");
        }
    }
}
