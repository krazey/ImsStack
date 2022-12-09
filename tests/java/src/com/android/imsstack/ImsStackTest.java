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

package com.android.imsstack;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Matchers.anyBoolean;
import static org.mockito.Matchers.nullable;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.spy;

import android.app.ActivityManager;
import android.app.IActivityManager;
import android.content.ContentProvider;
import android.content.ContentResolver;
import android.content.Context;
import android.content.IIntentSender;
import android.content.Intent;
import android.location.LocationManager;
import android.net.ConnectivityManager;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.MessageQueue;
import android.os.RegistrantList;
import android.os.StrictMode;
import android.provider.BlockedNumberContract;
import android.provider.DeviceConfig;
import android.provider.Settings;
import android.provider.Telephony;
import android.telephony.CarrierConfigManager;
import android.telephony.CellIdentity;
import android.telephony.CellLocation;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.test.mock.MockContentProvider;
import android.test.mock.MockContentResolver;
import android.testing.TestableLooper;
import android.util.Log;
import android.util.Singleton;

import org.mockito.Mockito;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public abstract class ImsStackTest {
    protected static String TAG;

    private static final int MAX_INIT_WAIT_MS = 30000; // 30 seconds

    private static final Field MESSAGE_QUEUE_FIELD;
    private static final Field MESSAGE_WHEN_FIELD;
    private static final Field MESSAGE_NEXT_FIELD;

    static {
        try {
            MESSAGE_QUEUE_FIELD = MessageQueue.class.getDeclaredField("mMessages");
            MESSAGE_QUEUE_FIELD.setAccessible(true);
            MESSAGE_WHEN_FIELD = Message.class.getDeclaredField("when");
            MESSAGE_WHEN_FIELD.setAccessible(true);
            MESSAGE_NEXT_FIELD = Message.class.getDeclaredField("next");
            MESSAGE_NEXT_FIELD.setAccessible(true);
        } catch (NoSuchFieldException e) {
            throw new RuntimeException("Failed to initialize ImsStackTest", e);
        }
    }

    // Mocked classes
    protected RegistrantList mRegistrantList;
    protected ServiceState mServiceState;
    protected Singleton<IActivityManager> mIActivityManagerSingleton;
    protected IActivityManager mIActivityManager;
    protected IIntentSender mIIntentSender;
    protected IBinder mIBinder;
    protected SignalStrength mSignalStrength;
    protected WifiManager mWifiManager;
    protected WifiInfo mWifiInfo;
    protected LocationManager mLocationManager;
    protected CellIdentity mCellIdentity;
    protected CellLocation mCellLocation;

    // Initialized classes
    protected ActivityManager mActivityManager;
    protected TelephonyManager mTelephonyManager;
    protected SubscriptionManager mSubscriptionManager;
    protected ConnectivityManager mConnectivityManager;
    protected CarrierConfigManager mCarrierConfigManager;
    protected ContextFixture mContextFixture;
    protected Context mContext;
    private final ContentProvider mContentProvider = spy(new ContextFixture.FakeContentProvider());
    private final Object mLock = new Object();
    private boolean mReady;
    protected NetworkRegistrationInfo mNetworkRegistrationInfo =
            new NetworkRegistrationInfo.Builder()
                    .setAccessNetworkTechnology(TelephonyManager.NETWORK_TYPE_LTE)
                    .setRegistrationState(NetworkRegistrationInfo.REGISTRATION_STATE_HOME)
                    .build();
    protected List<TestableLooper> mTestableLoopers = new ArrayList<>();
    protected TestableLooper mTestableLooper;

    private final HashMap<InstanceKey, Object> mOldInstances = new HashMap<>();

    private final ArrayList<InstanceKey> mInstanceKeys = new ArrayList<>();

    private static class InstanceKey {
        public final Class mClass;
        public final String mInstName;
        public final Object mObj;
        InstanceKey(final Class c, final String instName, final Object obj) {
            mClass = c;
            mInstName = instName;
            mObj = obj;
        }

        @Override
        public int hashCode() {
            return (mClass.getName().hashCode() * 31 + mInstName.hashCode()) * 31;
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof InstanceKey)) {
                return false;
            }

            InstanceKey other = (InstanceKey) obj;
            return (other.mClass == mClass && other.mInstName.equals(mInstName)
                    && other.mObj == mObj);
        }
    }

    @SuppressWarnings("WaitNotInLoop")
    protected void waitUntilReady() {
        synchronized (mLock) {
            if (!mReady) {
                try {
                    mLock.wait(MAX_INIT_WAIT_MS);
                } catch (InterruptedException ie) {
                }

                if (!mReady) {
                    fail("Telephony tests failed to initialize");
                }
            }
        }
    }

    protected void setReady(boolean ready) {
        synchronized (mLock) {
            mReady = ready;
            mLock.notifyAll();
        }
    }

    protected synchronized void replaceInstance(final Class c, final String instanceName,
                                                final Object obj, final Object newValue)
            throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);

        InstanceKey key = new InstanceKey(c, instanceName, obj);
        if (!mOldInstances.containsKey(key)) {
            mOldInstances.put(key, field.get(obj));
            mInstanceKeys.add(key);
        }
        field.set(obj, newValue);
    }

    protected synchronized void restoreInstance(final Class c, final String instanceName,
                                                final Object obj) throws Exception {
        InstanceKey key = new InstanceKey(c, instanceName, obj);
        if (mOldInstances.containsKey(key)) {
            Field field = c.getDeclaredField(instanceName);
            field.setAccessible(true);
            field.set(obj, mOldInstances.get(key));
            mOldInstances.remove(key);
            mInstanceKeys.remove(key);
        }
    }

    protected synchronized void restoreInstances() throws Exception {
        for (int i = mInstanceKeys.size() - 1; i >= 0; --i) {
            InstanceKey key = mInstanceKeys.get(i);
            Field field = key.mClass.getDeclaredField(key.mInstName);
            field.setAccessible(true);
            field.set(key.mObj, mOldInstances.get(key));
        }

        mInstanceKeys.clear();
        mOldInstances.clear();
    }

    // TODO: Unit tests that do not extend ImsStackTest or ImsTestBase should enable strict mode
    //   by calling this method.
    public static void enableStrictMode() {
        StrictMode.setVmPolicy(new StrictMode.VmPolicy.Builder()
                .detectLeakedSqlLiteObjects()
                .detectLeakedClosableObjects()
                .detectIncorrectContextUse()
                .detectLeakedRegistrationObjects()
                .detectUnsafeIntentLaunch()
                .detectActivityLeaks()
                .penaltyLog()
                .penaltyDeath()
                .build());
    }

    protected void setUp(String tag) throws Exception {
        TAG = tag;
        enableStrictMode();
        mRegistrantList = Mockito.mock(RegistrantList.class);
        mServiceState = Mockito.mock(ServiceState.class);
        mIActivityManagerSingleton = Mockito.mock(Singleton.class);
        mIActivityManager = Mockito.mock(IActivityManager.class);
        mIIntentSender = Mockito.mock(IIntentSender.class);
        mIBinder = Mockito.mock(IBinder.class);
        mSignalStrength = Mockito.mock(SignalStrength.class);
        mWifiManager = Mockito.mock(WifiManager.class);
        mWifiInfo = Mockito.mock(WifiInfo.class);
        mCellIdentity = Mockito.mock(CellIdentity.class);
        mCellLocation = Mockito.mock(CellLocation.class);

        TelephonyManager.disableServiceHandleCaching();
        // For testing do not allow Log.WTF as it can cause test process to crash
        Log.setWtfHandler((tagString, what, system) -> Log.d(TAG, "WTF captured, ignoring. Tag: "
                + tagString + ", exception: " + what));

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        ((MockContentResolver) mContext.getContentResolver()).addProvider(
                Settings.AUTHORITY, mContentProvider);
        ((MockContentResolver) mContext.getContentResolver()).addProvider(
                Telephony.ServiceStateTable.AUTHORITY, mContentProvider);
        replaceContentProvider(mContentProvider);

        Settings.Global.getInt(mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0);

        mTelephonyManager = mContext.getSystemService(TelephonyManager.class);
        mActivityManager = mContext.getSystemService(ActivityManager.class);
        mSubscriptionManager = mContext.getSystemService(SubscriptionManager.class);
        mConnectivityManager = mContext.getSystemService(ConnectivityManager.class);
        mCarrierConfigManager = mContext.getSystemService(CarrierConfigManager.class);
        mLocationManager = mContext.getSystemService(LocationManager.class);

        doReturn(true).when(mTelephonyManager).getSmsReceiveCapableForPhone(anyInt(), anyBoolean());
        doReturn(true).when(mTelephonyManager).getSmsSendCapableForPhone(anyInt(), anyBoolean());

        //Misc
        doReturn(ServiceState.RIL_RADIO_TECHNOLOGY_UMTS).when(mServiceState).
                getRilDataRadioTechnology();
        doReturn(mIBinder).when(mIIntentSender).asBinder();
        doReturn(mIIntentSender).when(mIActivityManager).getIntentSenderWithFeature(anyInt(),
                nullable(String.class), nullable(String.class), nullable(IBinder.class),
                nullable(String.class), anyInt(), nullable(Intent[].class),
                nullable(String[].class), anyInt(), nullable(Bundle.class), anyInt());
        doReturn(mTelephonyManager).when(mTelephonyManager).createForSubscriptionId(anyInt());
        doReturn(true).when(mTelephonyManager).isDataCapable();

        doReturn(TelephonyManager.PHONE_TYPE_GSM).when(mTelephonyManager).getPhoneType();
        doReturn(mNetworkRegistrationInfo).when(mServiceState).getNetworkRegistrationInfo(
                anyInt(), anyInt());
        doReturn(2).when(mSignalStrength).getLevel();

        // WiFi
        doReturn(mWifiInfo).when(mWifiManager).getConnectionInfo();
        doReturn(2).when(mWifiManager).calculateSignalLevel(anyInt());
        doReturn(4).when(mWifiManager).getMaxSignalLevel();

        //SIM
        doReturn(1).when(mTelephonyManager).getSimCount();
        doReturn(1).when(mTelephonyManager).getPhoneCount();
        doReturn(1).when(mTelephonyManager).getActiveModemCount();
        // Have getMaxPhoneCount always return the same value with getPhoneCount by default.
        doAnswer((invocation)->Math.max(mTelephonyManager.getActiveModemCount(),
                mTelephonyManager.getPhoneCount()))
                .when(mTelephonyManager).getSupportedModemCount();

        //Data
        //Initial state is: userData enabled, provisioned.
        ContentResolver resolver = mContext.getContentResolver();
        Settings.Global.putInt(resolver, Settings.Global.MOBILE_DATA, 1);
        Settings.Global.putInt(resolver, Settings.Global.DEVICE_PROVISIONED, 1);
        Settings.Global.putInt(resolver,
                Settings.Global.DEVICE_PROVISIONING_MOBILE_DATA_ENABLED, 1);

        // Metrics
        doReturn(null).when(mContext).getFileStreamPath(anyString());
        doReturn(mWifiManager).when(mContext).getSystemService(eq(Context.WIFI_SERVICE));

        //Use reflection to mock singletons
        replaceInstance(ActivityManager.class, "IActivityManagerSingleton", null,
                mIActivityManagerSingleton);
        replaceInstance(Singleton.class, "mInstance", mIActivityManagerSingleton,
                mIActivityManager);
        replaceInstance(TelephonyManager.class, "sInstance", null,
                mContext.getSystemService(Context.TELEPHONY_SERVICE));
        replaceInstance(TelephonyManager.class, "sServiceHandleCacheEnabled", null, false);

        setReady(false);
        // create default TestableLooper for test and add to list of monitored loopers
        mTestableLooper = TestableLooper.get(ImsStackTest.this);
        if (mTestableLooper != null) {
            monitorTestableLooper(mTestableLooper);
        }
    }

    protected void tearDown() throws Exception {
        // Clear all remaining messages
        if (!mTestableLoopers.isEmpty()) {
            for (TestableLooper looper : mTestableLoopers) {
                looper.getLooper().quit();
            }
        }
        // unmonitor TestableLooper for ImsStackTest class
        if (mTestableLooper != null) {
            unmonitorTestableLooper(mTestableLooper);
        }
        // destroy all newly created TestableLoopers so they can be reused
        for (TestableLooper looper : mTestableLoopers) {
            looper.destroy();
        }
        TestableLooper.remove(ImsStackTest.this);

        //SharedPreferences sharedPreferences = mContext.getSharedPreferences((String) null, 0);
        //sharedPreferences.edit().clear().commit();

        restoreInstances();
        TelephonyManager.enableServiceHandleCaching();

        mNetworkRegistrationInfo = null;
        mActivityManager = null;
        mTelephonyManager = null;
        mSubscriptionManager = null;
        mConnectivityManager = null;
        mCarrierConfigManager = null;
        mContextFixture = null;
        mContext = null;
        mTestableLoopers.clear();
        mTestableLoopers = null;
        mTestableLooper = null;
    }

    protected static void logd(String s) {
        Log.d(TAG, s);
    }

    public static class FakeBlockedNumberContentProvider extends MockContentProvider {
        public Set<String> mBlockedNumbers = new HashSet<>();
        public int mNumEmergencyContactNotifications = 0;

        @Override
        public Bundle call(String method, String arg, Bundle extras) {
            switch (method) {
                case BlockedNumberContract.SystemContract.METHOD_SHOULD_SYSTEM_BLOCK_NUMBER:
                    Bundle bundle = new Bundle();
                    int blockStatus = mBlockedNumbers.contains(arg)
                            ? BlockedNumberContract.STATUS_BLOCKED_IN_LIST
                            : BlockedNumberContract.STATUS_NOT_BLOCKED;
                    bundle.putInt(BlockedNumberContract.RES_BLOCK_STATUS, blockStatus);
                    return bundle;
                case BlockedNumberContract.SystemContract.METHOD_NOTIFY_EMERGENCY_CONTACT:
                    mNumEmergencyContactNotifications++;
                    return new Bundle();
                default:
                    fail("Method not expected: " + method);
            }
            return null;
        }
    }

    public static class FakeSettingsConfigProvider extends MockContentProvider {
        private static final String PROPERTY_DEVICE_IDENTIFIER_ACCESS_RESTRICTIONS_DISABLED =
                DeviceConfig.NAMESPACE_PRIVACY + "/"
                        + "device_identifier_access_restrictions_disabled";

        @Override
        public Bundle call(String method, String arg, Bundle extras) {
            switch (method) {
                case Settings.CALL_METHOD_GET_CONFIG: {
                    switch (arg) {
                        case PROPERTY_DEVICE_IDENTIFIER_ACCESS_RESTRICTIONS_DISABLED: {
                            Bundle bundle = new Bundle();
                            bundle.putString(
                                    PROPERTY_DEVICE_IDENTIFIER_ACCESS_RESTRICTIONS_DISABLED,
                                    "0");
                            return bundle;
                        }
                        default: {
                            fail("arg not expected: " + arg);
                        }
                    }
                    break;
                }
                default:
                    fail("Method not expected: " + method);
            }
            return null;
        }
    }

    private void replaceContentProvider(ContentProvider contentProvider) throws Exception {
        Class c = Class.forName("android.provider.Settings$Config");
        Field field = c.getDeclaredField("sNameValueCache");
        field.setAccessible(true);
        Object cache = field.get(null);

        c = Class.forName("android.provider.Settings$NameValueCache");
        field = c.getDeclaredField("mProviderHolder");
        field.setAccessible(true);
        Object providerHolder = field.get(cache);

        field = MockContentProvider.class.getDeclaredField("mIContentProvider");
        field.setAccessible(true);
        Object iContentProvider = field.get(contentProvider);

        replaceInstance(Class.forName("android.provider.Settings$ContentProviderHolder"),
                "mContentProvider", providerHolder, iContentProvider);
    }

    protected final void waitForDelayedHandlerAction(Handler h, long delayMillis,
            long timeoutMillis) {
        final CountDownLatch lock = new CountDownLatch(1);
        h.postDelayed(lock::countDown, delayMillis);
        while (lock.getCount() > 0) {
            try {
                lock.await(delayMillis + timeoutMillis, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                // do nothing
            }
        }
    }

    protected final void waitForHandlerAction(Handler h, long timeoutMillis) {
        final CountDownLatch lock = new CountDownLatch(1);
        h.post(lock::countDown);
        while (lock.getCount() > 0) {
            try {
                lock.await(timeoutMillis, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                // do nothing
            }
        }
    }

    /**
     * Wait for up to 1 second for the handler message queue to clear.
     */
    protected final void waitForLastHandlerAction(Handler h) {
        CountDownLatch lock = new CountDownLatch(1);
        // Allow the handler to start work on stuff.
        h.postDelayed(lock::countDown, 100);
        int timeoutCount = 0;
        while (timeoutCount < 5) {
            try {
                if (lock.await(200, TimeUnit.MILLISECONDS)) {
                    // no messages in queue, stop waiting.
                    if (!h.hasMessagesOrCallbacks()) break;
                    lock = new CountDownLatch(1);
                    // Delay to allow the handler thread to start work on stuff.
                    h.postDelayed(lock::countDown, 100);
                }

            } catch (InterruptedException e) {
                // do nothing
            }
            timeoutCount++;
        }
        assertTrue("Handler was not empty before timeout elapsed", timeoutCount < 5);
    }

    public static Object invokeMethod(
            Object instance, String methodName, Class<?>[] parameterClasses, Object[] parameters) {
        try {
            Method method = instance.getClass().getDeclaredMethod(methodName, parameterClasses);
            method.setAccessible(true);
            return method.invoke(instance, parameters);
        } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException e) {
            fail(instance.getClass() + " " + methodName + " " + e.getClass().getName());
        }
        return null;
    }

    /**
     * Add a TestableLooper to the list of monitored loopers
     * @param looper added if it doesn't already exist
     */
    public void monitorTestableLooper(TestableLooper looper) {
        if (!mTestableLoopers.contains(looper)) {
            mTestableLoopers.add(looper);
        }
    }

    /**
     * Remove a TestableLooper from the list of monitored loopers
     * @param looper removed if it does exist
     */
    public void unmonitorTestableLooper(TestableLooper looper) {
        if (mTestableLoopers.contains(looper)) {
            mTestableLoopers.remove(looper);
        }
    }

    /**
     * Handle all messages that can be processed at the current time
     * for all monitored TestableLoopers
     */
    public void processAllMessages() {
        if (mTestableLoopers.isEmpty()) {
            fail("mTestableLoopers is empty. Please make sure to add @RunWithLooper annotation");
        }
        while (!areAllTestableLoopersIdle()) {
            for (TestableLooper looper : mTestableLoopers) looper.processAllMessages();
        }
    }

    /**
     * @return The longest delay from all the message queues.
     */
    private long getLongestDelay() {
        long delay = 0;
        for (TestableLooper looper : mTestableLoopers) {
            MessageQueue queue = looper.getLooper().getQueue();
            try {
                Message msg = (Message) MESSAGE_QUEUE_FIELD.get(queue);
                while (msg != null) {
                    delay = Math.max(msg.getWhen(), delay);
                    msg = (Message) MESSAGE_NEXT_FIELD.get(msg);
                }
            } catch (IllegalAccessException e) {
                throw new RuntimeException("Access failed in ImsStackTest", e);
            }
        }
        return delay;
    }

    /**
     * @return {@code true} if there are any messages in the queue.
     */
    private boolean messagesExist() {
        for (TestableLooper looper : mTestableLoopers) {
            MessageQueue queue = looper.getLooper().getQueue();
            try {
                Message msg = (Message) MESSAGE_QUEUE_FIELD.get(queue);
                if (msg != null) return true;
            } catch (IllegalAccessException e) {
                throw new RuntimeException("Access failed in ImsStackTest", e);
            }
        }
        return false;
    }

    /**
     * Handle all messages including the delayed messages.
     */
    public void processAllFutureMessages() {
        while (messagesExist()) {
            moveTimeForward(getLongestDelay());
            processAllMessages();
        }
    }

    /**
     * Check if there are any messages to be processed in any monitored TestableLooper
     * Delayed messages to be handled at a later time will be ignored
     * @return true if there are no messages that can be handled at the current time
     *         across all monitored TestableLoopers
     */
    private boolean areAllTestableLoopersIdle() {
        for (TestableLooper looper : mTestableLoopers) {
            if (!looper.getLooper().getQueue().isIdle()) return false;
        }
        return true;
    }

    /**
     * Effectively moves time forward by reducing the time of all messages
     * for all monitored TestableLoopers
     * @param milliSeconds number of milliseconds to move time forward by
     */
    public void moveTimeForward(long milliSeconds) {
        for (TestableLooper looper : mTestableLoopers) {
            MessageQueue queue = looper.getLooper().getQueue();
            try {
                Message msg = (Message) MESSAGE_QUEUE_FIELD.get(queue);
                while (msg != null) {
                    long updatedWhen = msg.getWhen() - milliSeconds;
                    if (updatedWhen < 0) {
                        updatedWhen = 0;
                    }
                    MESSAGE_WHEN_FIELD.set(msg, updatedWhen);
                    msg = (Message) MESSAGE_NEXT_FIELD.get(msg);
                }
            } catch (IllegalAccessException e) {
                throw new RuntimeException("Access failed in ImsStackTest", e);
            }
        }
    }
}
