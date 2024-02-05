/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.base;

import android.annotation.CallbackExecutor;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.QosCallback;
import android.net.QosCallbackException;
import android.net.QosSession;
import android.net.QosSocketInfo;
import android.os.Handler;
import android.telephony.data.EpsBearerQosSessionAttributes;
import android.telephony.data.NrQosSessionAttributes;
import android.util.ArraySet;
import android.util.SparseArray;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;

import java.net.InetSocketAddress;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.Executor;

/**
 * An implementation class to access the {@link ConnectivityManager}.
 */
public class ConnectivityManagerProxyImpl implements ConnectivityManagerProxy {
    private final ArraySet<NetworkCallbackRecord> mNetworkCallbackRecords = new ArraySet<>();
    private final ArraySet<QosCallbackRecord> mQosCallbackRecords = new ArraySet<>();
    private final SparseArray<LinkProperties> mLinkProperties = new SparseArray<>();

    @Override
    public void registerQosCallback(@NonNull final QosSocketInfo socketInfo,
            @CallbackExecutor @NonNull final Executor executor,
            @NonNull final QosCallback callback) {
        mQosCallbackRecords.add(new QosCallbackRecord(socketInfo, callback, executor));
    }

    @Override
    public void unregisterQosCallback(@NonNull final QosCallback callback) {
        final ArraySet<QosCallbackRecord> recordsToRemove = new ArraySet<>();
        mQosCallbackRecords.forEach((r) -> {
            if (r.hasCallback(callback)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mQosCallbackRecords::remove);
    }

    @Override
    public void registerNetworkCallback(@NonNull NetworkRequest request,
            @NonNull NetworkCallback networkCallback, @NonNull Handler handler) {
        mNetworkCallbackRecords.add(new NetworkCallbackRecord(request, networkCallback, handler));
    }

    @Override
    public void unregisterNetworkCallback(@NonNull NetworkCallback networkCallback) {
        final ArraySet<NetworkCallbackRecord> recordsToRemove = new ArraySet<>();
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.hasCallback(networkCallback)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mNetworkCallbackRecords::remove);
    }

    @Override
    public @Nullable LinkProperties getLinkProperties(@Nullable Network network) {
        return mLinkProperties.get(network.getNetId());
    }

    @Override
    public void requestNetwork(@NonNull NetworkRequest request,
            @NonNull NetworkCallback networkCallback, @NonNull Handler handler) {
        mNetworkCallbackRecords.add(new NetworkCallbackRecord(request, networkCallback, handler));
    }

    @Override
    public void registerDefaultNetworkCallback(@NonNull NetworkCallback networkCallback,
            @NonNull Handler handler) {
        mNetworkCallbackRecords.add(new NetworkCallbackRecord(null, networkCallback, handler));
    }

    /**
     * Sets the {@link LinkProperties} for the given {@link Network}.
     *
     * @param network The {@link Network} object identifying the network.
     * @param properties The {@link LinkProperties} for the specified network.
     */
    public void setLinkProperties(Network network, LinkProperties properties) {
        mLinkProperties.put(network.getNetId(), properties);
    }

    /**
     * Notifies the application that the network is connected and has declared
     * a new network ready for use.
     *
     * @param network The {@link Network} of the satisfying network.
     * @param networkCapabilities The {@link NetworkCapabilities} of the satisfying network.
     * @param linkProperties The {@link LinkProperties} of the satisfying network.
     */
    public void notifyNetworkAvailable(@NonNull Network network,
            @NonNull NetworkCapabilities networkCapabilities,
            @NonNull LinkProperties linkProperties) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (!r.isForDefaultNetwork()) {
                r.dispatchNetworkAvailable(network, networkCapabilities, linkProperties);
            }
        });
    }

    /**
     * Notifies the application that if no network is found within the timeout time specified in
     * {@link #requestNetwork(NetworkRequest, NetworkCallback, int)} call or if the
     * requested network request cannot be fulfilled (whether or not a timeout was specified).
     *
     * @param network The {@link Network} that is about to be unavailable.
     */
    public void notifyNetworkUnavailable(@NonNull Network network) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (!r.isForDefaultNetwork()) {
                r.dispatchNetworkUnavailable(network);
            }
        });
    }

    /**
     * Notifies the application that the network is about to be lost, typically because there are
     * no outstanding requests left for it. This may be paired with a
     * {@link #notifyNetworkAvailable} call with the new replacement network for graceful
     * handover. This method is not guaranteed to be called before {@link #notifyNetworkLost}
     * is called, for example in case a network is suddenly disconnected.
     *
     * @param network The {@link Network} that is about to be lost.
     * @param maxMsToLive The time in milliseconds the system intends to keep the network
     *                    connected for graceful handover; note that the network may still
     *                    suffer a hard loss at any time.
     */
    public void notifyNetworkLosing(@NonNull Network network, int maxMsToLive) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (!r.isForDefaultNetwork()) {
                r.dispatchNetworkLosing(network, maxMsToLive);
            }
        });
    }

    /**
     * Notifies the application that a network disconnects or otherwise no longer satisfies
     * this request or callback.
     *
     * @param network The {@link Network} that is about to be lost.
     */
    public void notifyNetworkLost(@NonNull Network network) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (!r.isForDefaultNetwork()) {
                r.dispatchNetworkLost(network);
            }
        });
    }

    /**
     * Notifies the application that the default network is connected and has declared
     * a new network ready for use.
     *
     * @param network The {@link Network} of the satisfying network.
     * @param networkCapabilities The {@link NetworkCapabilities} of the satisfying network.
     * @param linkProperties The {@link LinkProperties} of the satisfying network.
     */
    public void notifyDefaultNetworkAvailable(@NonNull Network network,
            @NonNull NetworkCapabilities networkCapabilities,
            @NonNull LinkProperties linkProperties) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.isForDefaultNetwork()) {
                r.dispatchNetworkAvailable(network, networkCapabilities, linkProperties);
            }
        });
    }

    /**
     * Notifies the application that if no network is found within the timeout time specified in
     * {@link #requestNetwork(NetworkRequest, NetworkCallback, int)} call or if the
     * requested network request cannot be fulfilled (whether or not a timeout was specified).
     *
     * @param network The {@link Network} that is about to be unavailable.
     */
    public void notifyDefaultNetworkUnavailable(@NonNull Network network) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.isForDefaultNetwork()) {
                r.dispatchNetworkUnavailable(network);
            }
        });
    }

    /**
     * Notifies the application that the network is about to be lost, typically because there are
     * no outstanding requests left for it. This may be paired with a
     * {@link #notifyDefaultNetworkAvailable} call with the new replacement network for graceful
     * handover. This method is not guaranteed to be called before {@link #notifyDefaultNetworkLost}
     * is called, for example in case a network is suddenly disconnected.
     *
     * @param network The {@link Network} that is about to be lost.
     * @param maxMsToLive The time in milliseconds the system intends to keep the network
     *                    connected for graceful handover; note that the network may still
     *                    suffer a hard loss at any time.
     */
    public void notifyDefaultNetworkLosing(@NonNull Network network, int maxMsToLive) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.isForDefaultNetwork()) {
                r.dispatchNetworkLosing(network, maxMsToLive);
            }
        });
    }

    /**
     * Notifies the application that a network disconnects or otherwise no longer satisfies
     * this request or callback.
     *
     * @param network The {@link Network} that is about to be lost.
     */
    public void notifyDefaultNetworkLost(@NonNull Network network) {
        mNetworkCallbackRecords.forEach((r) -> {
            if (r.isForDefaultNetwork()) {
                r.dispatchNetworkLost(network);
            }
        });
    }

    /**
     * Notifies the application that an error occurs on a registered callback. Once called,
     * the callback is automatically unregistered and the callback will no longer receive calls.
     */
    public void notifyQosError() {
        mQosCallbackRecords.forEach((r) -> r.dispatchError());
    }

    /**
     * Notifies the application that a Qos Session for EPS first becomes available to the callback
     * or if its attributes have changed.
     */
    public void notifyEpsQosSessionAvailable() {
        mQosCallbackRecords.forEach((r) -> r.dispatchSessionAvailable(QosSession.TYPE_EPS_BEARER));
    }

    /**
     * Notifies the application that a Qos Session is lost.
     */
    public void notifyEpsQosSessionLost() {
        mQosCallbackRecords.forEach((r) -> r.dispatchSessionLost(QosSession.TYPE_EPS_BEARER));
    }

    /**
     * Notifies the application that a Qos Session for NR first becomes available to the callback
     * or if its attributes have changed.
     */
    public void notifyNrQosSessionAvailable() {
        mQosCallbackRecords.forEach((r) -> r.dispatchSessionAvailable(QosSession.TYPE_NR_BEARER));
    }

    /**
     * Notifies the application that a Qos Session is lost.
     */
    public void notifyNrQosSessionLost() {
        mQosCallbackRecords.forEach((r) -> r.dispatchSessionLost(QosSession.TYPE_NR_BEARER));
    }

    private static final class QosCallbackRecord {
        private static final QosSession EPS_SESSION =
                new QosSession(10001, QosSession.TYPE_EPS_BEARER);
        private static final QosSession NR_SESSION =
                new QosSession(10002, QosSession.TYPE_NR_BEARER);
        private final QosSocketInfo mSocketInfo;
        private final QosCallback mCallback;
        private final Executor mScheduler;

        QosCallbackRecord(QosSocketInfo socketInfo, QosCallback callback, Executor scheduler) {
            mSocketInfo = socketInfo;
            mCallback = callback;
            mScheduler = scheduler;
        }

        boolean hasCallback(QosCallback callback) {
            return mCallback.equals(callback);
        }

        void dispatchError() {
            mScheduler.execute(() -> {
                mCallback.onError(new QosCallbackException("Qos session unavailable"));
            });
        }

        void dispatchSessionAvailable(int qosSessionType) {
            mScheduler.execute(() -> {
                final List<InetSocketAddress> remoteAddresses;
                if (mSocketInfo.getRemoteSocketAddress() != null) {
                    remoteAddresses = List.of(mSocketInfo.getRemoteSocketAddress());
                } else {
                    remoteAddresses = Collections.emptyList();
                }
                if (qosSessionType == QosSession.TYPE_EPS_BEARER) {
                    mCallback.onQosSessionAvailable(EPS_SESSION,
                            new EpsBearerQosSessionAttributes(1, 41, 41, 39, 39, remoteAddresses));
                } else if (qosSessionType == QosSession.TYPE_NR_BEARER) {
                    mCallback.onQosSessionAvailable(NR_SESSION,
                            new NrQosSessionAttributes(1, 11, 41, 41, 39, 39, 20, remoteAddresses));
                }
            });
        }

        void dispatchSessionLost(int qosSessionType) {
            mScheduler.execute(() -> {
                if (qosSessionType == QosSession.TYPE_EPS_BEARER) {
                    mCallback.onQosSessionLost(EPS_SESSION);
                } else if (qosSessionType == QosSession.TYPE_NR_BEARER) {
                    mCallback.onQosSessionLost(NR_SESSION);
                }
            });
        }
    }

    private static final class NetworkCallbackRecord {
        private final NetworkRequest mRequest;
        private final NetworkCallback mCallback;
        private final Handler mScheduler;

        NetworkCallbackRecord(NetworkRequest request, NetworkCallback callback, Handler scheduler) {
            mRequest = request;
            mCallback = callback;
            mScheduler = scheduler;
        }

        boolean hasCallback(NetworkCallback callback) {
            return mCallback.equals(callback);
        }

        boolean isForDefaultNetwork() {
            return mRequest == null;
        }

        void dispatchNetworkAvailable(@NonNull Network network,
                @NonNull NetworkCapabilities networkCapabilities,
                @NonNull LinkProperties linkProperties) {
            mScheduler.post(() -> {
                mCallback.onAvailable(network);
                mCallback.onCapabilitiesChanged(network, networkCapabilities);
                mCallback.onLinkPropertiesChanged(network, linkProperties);
            });
        }

        void dispatchNetworkUnavailable(@NonNull Network network) {
            mScheduler.post(() -> {
                mCallback.onUnavailable();
            });
        }

        void dispatchNetworkLosing(@NonNull Network network, int maxMsToLive) {
            mScheduler.post(() -> {
                mCallback.onLosing(network, maxMsToLive);
            });
        }

        void dispatchNetworkLost(@NonNull Network network) {
            mScheduler.post(() -> {
                mCallback.onLost(network);
            });
        }
    }
}
