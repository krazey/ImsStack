/**
 * SIMStateAgent
 *    Role
 *
 */

package com.android.imsstack.core.agents;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.SystemClock;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.core.agents.ImsPhoneProxyApi;
import com.android.imsstack.core.agents.agentif.IIMSPhoneAgent;
import com.android.imsstack.core.agents.agentif.IISIM;
import com.android.imsstack.core.agents.agentif.ISharedState;
import com.android.imsstack.core.agents.agentif.ISIMState;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.core.agents.agentif.SubscriptionListener;
import com.android.imsstack.core.config.ProviderInterface.SMS;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.AosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosInfo.IsimState;
import com.android.imsstack.enabler.aos.IAosInfo.PhoneNumberState;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPISIM;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsExtApi;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;

import java.util.Arrays;
import java.util.Set;

public class SIMStateAgent implements ISIMState, ISystemAPISIM {
    // Constants--------------------------------------------------
    // FIXME ::
    public static final int SYS_INFO_ICC_EF_READ = 7;

    private static final int EVENT_SIM_STATE_CHANGED = 1001;
    private static final int EVENT_ISIM_STATE_CHANGED = 1002;
    private static final int EVENT_RETRY_GET_PSI_INFO = 1003;
    private static final int EVENT_SERVICE_REGISTERED = 1004;
    private static final int EVENT_DATA_SUBSCRIPTION_CHANGED = 1006;
    private static final int EVENT_UPDATE_ISIM_STATE = 1007;
    private static final int EVENT_NATIVE_BOOT_COMPLETED = 1008;
    // IMS-LAMPLITE : CACHE_ECC
    private static final int EVENT_CACHE_ECC_LIST = 1009;

    // IMS-LAMPLITE : CACHE_ECC, 8 times including first try
    private static final int MAX_RETRY_COUNT_FOR_ECC_LIST = 7;
    // private static final int USIM_EF_ID_PSI = 0x6FE5;
    // For explicit ISIM state update, waits for 5 seconds.
    private static final long INTERVAL_FOR_EXPLICIT_ISIM_STATE_UPDATE = 5000L;

    public static final int IMS_RETRY_GET_PSI_INFO_MAX_COUNT = 20;
    /////
    // ISIM/USIM Request for Phone (Java)
    ////

    // 1 : Read ISIM EF file attrubute : (int EF_type)
    public static final int REQUEST_READ_FILE_ATTRIBUTE = 1;
    // 2 : Read ISIM EF one record using index : (int EF_type, int index)
    public static final int REQUEST_READ_RECORD = 2;
    // 3 : Request ISIM AUTH : (String nonce, int owner)
    public static final int REQUEST_ISIM_AUTH = 3;
    // immediate return
    // 4 : Request ISIM state : (no arg)
    public static final int REQUEST_GET_STATE = 4;
     // 5 : Request USIM AUTH : (String nonce, int owner)
    public static final int REQUEST_USIM_AUTH = 5;

    // EF_type - same as AndroidISIM.h
    public static final int EF_ID_IMPI = 0x6F02;
    public static final int EF_ID_DOMAIN = 0x6F03;
    public static final int EF_ID_IMPU = 0x6F04;
    public static final int EF_ID_IST = 0x6F07;    // ISIM service table
    public static final int EF_ID_PCSCF = 0x6F09;

    // UICC notification
    // ISIM
    public static final int NOTIFICATION_ISIM_STATE_REFRESH = 101;
    public static final int NOTIFICATION_ISIM_STATE_CHANGED = 102;
    public static final int NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE = 103;
    public static final int NOTIFICATION_ISIM_READ_RECORD = 104;
    public static final int NOTIFICATION_ISIM_AUTH = 105;
    // USIM
    public static final int NOTIFICATION_USIM_AUTH = 106;

    /**
     * It's to identify whether SIM agent is stopped.
     * If SIM agent is stopped by any reasons, it will be delivered to Native ISIM module.
     */
    private static final String ISIM_STATE_SIM_REMOVED = ImsExtApi.Uicc.SIM_REMOVED;

    // Variables--------------------------------------------------
    private Handler mSIMStateHandler;
    private SIMStateReceiverListener mReceiverListener;
    private SubscriptionListenerProxy mSubscriptionListener;
    private RegistrationListener mRegistrationListener;

    private RegistrantList mSimStateChangedRegistrants;
    private RegistrantList mIsimStateChangedRegistrants;

    private Context mContext;

    private boolean mIccLoaded = false;
    private boolean mNoIsimAppNotificationRequired = false;
    private boolean mPsiReadDone = false;
    private boolean mPsiResetRequired = false;
    private int mRetryCountForPsi = 0;
    private int mRetryCountForCacheEccList = 0;

    private String mIccState = IccCardConstants.INTENT_VALUE_ICC_UNKNOWN;
    private String mIsimState = IISIM.STATE_NOT_READY;

    private String mSMSPAddress = "";

    private int mSlotId = 0;
    private ISystem mSystem;


    private class IsimEvent {
        private final int mSlotId;
        private final int mSubId;
        private final String mState;

        public IsimEvent(int slotId, int subId, String state) {
            mSlotId = slotId;
            mSubId = subId;
            mState = state;
        }

        public int getSlotId() {
            return mSlotId;
        }

        public int getSubId() {
            return mSubId;
        }

        public String getState() {
            return mState;
        }
    }

    // Static loading materials ----------------------------------
    // Public methods --------------------------------------------
    public SIMStateAgent(int slotId) {
        mSlotId = slotId;
    }

    // Interface implementation methods --------------------------
    @Override
    public void init(Context context) {
        if (context == null) {
            return;
        }

        mContext = context;

        mSystem = SystemInterface.getInstance().getSystem(mSlotId);
        if (mSystem != null) {
            mSystem.setISystemAPISIM(this);
        }

        mSimStateChangedRegistrants = new RegistrantList();
        mIsimStateChangedRegistrants = new RegistrantList();

        mSIMStateHandler = new SIMStateHandler();

        mReceiverListener = new SIMStateReceiverListener();
        mContext.registerReceiver(mReceiverListener,
                mReceiverListener.getFilter(), Context.RECEIVER_EXPORTED);

        if (MSimUtils.isMultiSimEnabled()) {
            mSubscriptionListener = new SubscriptionListenerProxy();

            ISubscription subs = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
            if (subs != null) {
                subs.addListener(mSubscriptionListener);
            }
        } else {
            mSubscriptionListener = null;
        }

        // TODO : Need to create Config
        if (isPsiRequired()) {
            IAosRegistration aosRegistration = AosFactory.getInstance().
                    getAosRegistration(mSlotId);
            if (aosRegistration != null) {
                mRegistrationListener = new RegistrationListener();
                aosRegistration.addListener(mRegistrationListener);
            }
        }

        ISharedState iss = (ISharedState)AgentFactory.getAgent(AgentFactory.SHARED_STATE, mSlotId);
        if (iss != null) {
            iss.registerForNativeBootComplete(mSIMStateHandler, EVENT_NATIVE_BOOT_COMPLETED, null);
        }

    }

    @Override
    public void cleanup() {
        mIccLoaded = false;
        mNoIsimAppNotificationRequired = false;
        mPsiReadDone = false;
        mRetryCountForPsi = 0;
        mIccState = IccCardConstants.INTENT_VALUE_ICC_NOT_READY;
        mIsimState = IISIM.STATE_NOT_READY;

        ISharedState iss = (ISharedState)AgentFactory.getAgent(AgentFactory.SHARED_STATE, mSlotId);
        if (iss != null) {
            iss.unregisterForNativeBootComplete(mSIMStateHandler);
        }

        if (mSystem != null) {
            mSystem.notifyISIMState(NOTIFICATION_ISIM_STATE_CHANGED, ISIM_STATE_SIM_REMOVED);
        }

        if (mRegistrationListener != null) {
            IAosRegistration aosRegistration = AosFactory.getInstance().
                    getAosRegistration(mSlotId);
            if (aosRegistration != null) {
                aosRegistration.removeListener(mRegistrationListener);
            }
        }

        if (mSubscriptionListener != null) {
            ISubscription subs = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
            if (subs != null) {
                subs.removeListener(mSubscriptionListener);
            }

            mSubscriptionListener = null;
        }

        if (mReceiverListener != null && mContext != null) {
            mContext.unregisterReceiver(mReceiverListener);
            mReceiverListener = null;
        }

        if (mSIMStateHandler != null) {
            mSIMStateHandler.removeCallbacksAndMessages(null);
            mSIMStateHandler = null;
        }

        mSimStateChangedRegistrants = null;
        mIsimStateChangedRegistrants = null;

        if (mSystem != null) {
            mSystem.setISystemAPISIM(null);
            mSystem = null;
        }
    }

    @Override
    public void requestPSIInfo() {
        if (mSIMStateHandler != null) {
            Message.obtain(mSIMStateHandler, EVENT_RETRY_GET_PSI_INFO, 0, 0).sendToTarget();
        }
    }

    @Override
    public void setPSIValueToDB(String psi) {
        ImsLog.i(mSlotId, "");

        if (mContext == null) {
            return;
        }

        try {
            ContentResolver cr = mContext.getContentResolver();
            ContentValues newValue = new ContentValues();

            newValue.put(SMS.PSI, psi);
            cr.update(SMS.CONTENT_URI, newValue, null, null);
        } catch (Exception e) {
            ImsLog.e(mSlotId, e.toString());
        }
    }

    @Override
    public String getIccState() {
        return mIccState;
    }

    @Override
    public String getIsimState() {
        return mIsimState;
    }

    @Override
    public String getSMSPAddress() {
        if (TextUtils.isEmpty(mSMSPAddress)) {
            mSMSPAddress = ImsExtApi.Uicc.getEfSmsP();
            ImsLog.i(mSlotId, "SMSP=" + ImsLog.hiddenString(mSMSPAddress));
        }

        return mSMSPAddress;
    }

    @Override
    public boolean isIccLoaded() {
        return mIccLoaded;
    }

    @Override
    public boolean isIsimLoaded() {
        return IISIM.STATE_LOADED.equals(mIsimState);
    }

    @Override
    public void registerForSimStateChanged(Handler h, int what, Object obj) {
        if (mSimStateChangedRegistrants != null) {
            mSimStateChangedRegistrants.add(new Registrant(h, what, obj));
        }
    }

    @Override
    public void unregisterForSimStateChanged(Handler h) {
        if (mSimStateChangedRegistrants != null) {
            mSimStateChangedRegistrants.remove(h);
        }
    }

    @Override
    public void registerForIsimStateChanged(Handler h, int what, Object obj) {
        if (mIsimStateChangedRegistrants != null) {
            mIsimStateChangedRegistrants.add(new Registrant(h, what, obj));
        }
    }

    @Override
    public void unregisterForIsimStateChanged(Handler h) {
        if (mIsimStateChangedRegistrants != null) {
            mIsimStateChangedRegistrants.remove(h);
        }
    }

    @Override
    public String getISIMState4Sys() {
        return getIsimState();
    }

    @Override
    public int readISIMFileAttributes4Sys(int field) {
        if ((mSIMStateHandler != null) && !checkIsimProvisioningViaOta()) {
            Message msg = Message.obtain(mSIMStateHandler, REQUEST_READ_FILE_ATTRIBUTE, field, 0);
            msg.sendToTarget();
        } else {
            final int fileId = field;
            executeOnIsimThread(new Runnable() {
                @Override
                public void run() {
                    waitAndReadIsimFileAttributes(fileId);
                }
            });
        }
        return 1;
    }

    @Override
    public int readISIMRecord4Sys(int field, int index) {
        if (mSIMStateHandler != null) {
            Message msg = Message.obtain(mSIMStateHandler, REQUEST_READ_RECORD, field, index);
            msg.sendToTarget();
        } else {
            final int fileId = field;
            final int recordIndex = index;
            executeOnIsimThread(new Runnable() {
                @Override
                public void run() {
                    handleRequestForIsimFileRecord(fileId, recordIndex);
                }
            });
        }

        return 1;
    }

    @Override
    public int requestISIMAuthentication4Sys(String nonce, long owner) {
        final AuthData ad = new AuthData(nonce, owner);
        executeOnIsimThread(new Runnable() {
            @Override
            public void run() {
                handleRequestForIsimAuthentication(ad);
            }
        });

        return 1;
    }

    @Override
    public int requestUSIMAuthentication4Sys(String nonce, long owner) {
        final AuthData ad = new AuthData(nonce, owner);
        executeOnIsimThread(new Runnable() {
            @Override
            public void run() {
                handleRequestForUsimAuthentication(ad);
            }
        });

        return 1;
    }

    // Private/Protected methods ---------------------------------
    // IMS-LAMPLITE : CACHE_ECC
    private void cacheEccList() {
        if (!ImsExtApi.Uicc.cacheEccList(mSlotId)) {
            if (mRetryCountForCacheEccList >= MAX_RETRY_COUNT_FOR_ECC_LIST) {
                ImsLog.i(mSlotId, "cacheEccList :: RetryCount is over.");
                mRetryCountForCacheEccList = 0;
                return;
            }

            if (mSIMStateHandler != null) {
                mSIMStateHandler.sendEmptyMessageDelayed(
                        EVENT_CACHE_ECC_LIST,
                        1000 * (1 << mRetryCountForCacheEccList));

                mRetryCountForCacheEccList++;
            }
        } else {
            ImsLog.i(mSlotId, "EccList cached - " + mRetryCountForCacheEccList);
            mRetryCountForCacheEccList = 0;
        }
    }

    private boolean getPSIInfo() {
        if (mPsiReadDone == true) {
            ImsLog.w(mSlotId, "PSI already read, no need to read");
            return true;
        }

        ISharedState iss = (ISharedState)AgentFactory.getAgent(AgentFactory.SHARED_STATE, mSlotId);
        if (iss == null) {
            return false;
        }

        if (iss.isNativeBootCompleted() == false) {
            ImsLog.w(mSlotId, "Native process is not ready, do retry");
            return false;
        }

        if (mPsiResetRequired) {
            mPsiResetRequired = false;
            setPSIValueToDB("");
        }

        IIMSPhoneAgent ipa = (IIMSPhoneAgent)AgentFactory.getAgent(AgentFactory.IMS_PHONE, mSlotId);
        if (ipa == null) {
            ImsLog.w(mSlotId, "IIMSPhoneAgent is null");
            return false;
        }

        return (ipa.getPsiRecord() != ImsPhoneProxyApi.RESULT_NOK);

    }

    private void waitAndReadIsimFileAttributes(final int fileId) {
        final int pollingInterval = 2000;
        final int maxRetryCount = 150;
        int count = 0;

        while (checkIsimProvisioningViaOta()) {
            if ((count == 0) || ((count % 10) == 0)) {
                ImsLog.w(mSlotId, "Waits for OTA opening; fileId=" + fileId);
            }

            SystemClock.sleep(pollingInterval);

            count++;

            // Waits for the maximum 5 minutes
            if (count == maxRetryCount) {
                ImsLog.w(mSlotId, "No more waits for OTA opening; fileId=" + fileId);
                break;
            }
        }

        handleRequestForIsimFileAttributes(fileId);
    }

    private void handleRequestForIsimFileAttributes(int fileId) {
        String[] fileContent = null;

        IISIM isim = (IISIM)AgentFactory.getAgent(AgentFactory.ISIM, mSlotId);
        if (isim == null) {
            ImsLog.w(mSlotId, "IISIM is null");
            return;
        }

        if (fileId == EF_ID_IMPI) {
            String impi = isim.getImpi();

            if (impi != null) {
                fileContent = new String[] { impi };
            }
        } else if (fileId == EF_ID_DOMAIN) {
            String domain = isim.getDomain();

            if (domain != null) {
                fileContent = new String[] { domain };
            }
        } else if (fileId == EF_ID_IMPU) {
            String[] impu = isim.getImpu();

            if (impu != null) {
                fileContent = Arrays.copyOf(impu, impu.length);
            }
        } else if (fileId == EF_ID_IST) {
            String ist = isim.getIst();

            if (ist != null) {
                fileContent = new String[] { ist };
            }
        } else if (fileId == EF_ID_PCSCF) {
            String[] pcscf = isim.getPcscf();

            if (pcscf != null) {
                fileContent = Arrays.copyOf(pcscf, pcscf.length);
            }
        } else {
            ImsLog.w(mSlotId, "Unknown ISIM record field; id=" + fileId);
            return;
        }

        int recordCount = (fileContent != null) ? fileContent.length : 0;

        ImsLog.i(mSlotId, "id=" + Integer.toHexString(fileId)
                + ", name=" + getIsimFileName(fileId)
                + ", count=" + recordCount);

        if (mSystem != null) {
            mSystem.notifyISIMFileAttributeResponse(NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE,
                    fileId, recordCount, fileContent);
        }
    }

    private void handleRequestForIsimFileRecord(int fileId, int recordIndex) {
        String record = null;
        IISIM isim = (IISIM)AgentFactory.getAgent(AgentFactory.ISIM, mSlotId);
        if (isim == null) {
            ImsLog.w(mSlotId, "IISIM is null");
            return;
        }

        if (fileId == EF_ID_IMPI) {
            record = isim.getImpi();
        } else if (fileId == EF_ID_DOMAIN) {
            record = isim.getDomain();
        } else if (fileId == EF_ID_IMPU) {
            String[] impu = isim.getImpu();

            if ((impu != null) && (recordIndex >= 0) && (recordIndex < impu.length)) {
                record = impu[recordIndex];
            }
        } else if (fileId == EF_ID_IST) {
            record = isim.getIst();
        } else if (fileId == EF_ID_PCSCF) {
            String[] pcscf = isim.getPcscf();

            if ((pcscf != null) && (recordIndex >= 0) && (recordIndex < pcscf.length)) {
                record = pcscf[recordIndex];
                // Skip "address type" field (FQDN(0)/IPV4(1)/IPV6(2))
                if ((record != null) && (record.length() > 0)) {
                    record = record.substring(1);
                }
            }
        } else {
            ImsLog.w(mSlotId, "Unknown ISIM record field; id=" + fileId);
            return;
        }

        ImsLog.d(mSlotId, "id=" + Integer.toHexString(fileId)
                + ", name=" + getIsimFileName(fileId)
                + ", index=" + recordIndex
                + ", record=" + record);

        if (mSystem != null) {
            mSystem.notifyISIMFileFileValueResponse(NOTIFICATION_ISIM_READ_RECORD,
                    fileId, recordIndex, record);
        }
    }

    private void handleRequestForIsimAuthentication(AuthData ad) {
        if (ad == null) {
            return;
        }

        IISIM isim = (IISIM)AgentFactory.getAgent(AgentFactory.ISIM, mSlotId);
        if (isim == null) {
            ImsLog.w(mSlotId, "IISIM is null");
            return;
        }

        String response = isim.getChallengeResponse(ad.mNonce);
        ImsLog.i(mSlotId, "nonce=" + ad.mNonce + ", owner=" + ad.mOwner + ", response=" + response);

        if (mSystem != null) {
            mSystem.notifyISIMAuthResponse(NOTIFICATION_ISIM_AUTH, response, ad.mOwner);
        }
    }

    private void handleRequestForUsimAuthentication(AuthData ad) {
        if (ad == null) {
            return;
        }

        String response = "";
        TelephonyManager tm = null;

        int subId = MSimUtils.getSubId(mSlotId);
        if (MSimUtils.isValidSubId(subId)) {
            tm = AppContext.getTelephonyManager(subId);
        }

        if (tm != null) {
            response = tm.getIccAuthentication(TelephonyManager.APPTYPE_USIM,
                    TelephonyManager.AUTHTYPE_EAP_AKA, ad.mNonce);
        }

        ImsLog.i(mSlotId, "nonce=" + ad.mNonce + ", owner=" + ad.mOwner + ", response=" + response);

        if (mSystem != null) {
            mSystem.notifyUSIMAuthResponse(NOTIFICATION_USIM_AUTH, response, ad.mOwner);
        }
    }

    private boolean isPsiRequired() {
        // FIXME: If the configuration is changed on runtime,
        // it needs to be improved.
        String operator = OperatorInfo.getOperator(mSlotId);
        String country = OperatorInfo.getCountry(mSlotId);
        return (OperatorInfo.equalsOperator("ATT", operator)
                || OperatorInfo.equalsOperator("RJIL", operator)
                || OperatorInfo.equalsOperator("KT", operator)
                || OperatorInfo.isGroupTMUS(mSlotId, operator, country));
    }

    private String getIsimFileName(int fileId) {
        if (fileId == EF_ID_IMPI) {
            return "IMPI";
        } else if (fileId == EF_ID_DOMAIN) {
            return "DOMAIN";
        } else if (fileId == EF_ID_IMPU) {
            return "IMPU";
        } else if (fileId == EF_ID_IST) {
            return "IST";
        } else if (fileId == EF_ID_PCSCF) {
            return "PCSCF";
        } else {
            return "UNKNOWN";
        }
    }

    private void handleRetryGetPsiInfo() {
        if (!getPSIInfo()) {

            mRetryCountForPsi++;

            if (mRetryCountForPsi < IMS_RETRY_GET_PSI_INFO_MAX_COUNT) {
                ImsLog.w(mSlotId, "mRetryCountForPsi =" + mRetryCountForPsi);
                readPsiDelayed();
            }
        } else {
            mPsiReadDone = true;
        }
    }

    private void readPsiDelayed() {
        if (mSIMStateHandler != null) {
            // retry after 2 sec, 20 times
            mSIMStateHandler.sendEmptyMessageDelayed(EVENT_RETRY_GET_PSI_INFO, 2000);
        }
    }

    private void handleDataSubscriptionChanged(int subId) {
        int slotId = MSimUtils.getSlotId(subId);
        if (mSlotId != slotId) {
            return;
        }

        String simState = MSimUtils.getSimState(slotId);

        if (!TextUtils.isEmpty(simState)) {
            setIccState(simState, subId);
        } else {
            String imsi = MSimUtils.getImsi(subId);

            if (!TextUtils.isEmpty(imsi)) {
                setIccState(IccCardConstants.INTENT_VALUE_ICC_IMSI, subId);
            } else {
                setIccState(IccCardConstants.INTENT_VALUE_ICC_NOT_READY, subId);
            }
        }

        updateIsimState();

        /*
        if (IISIM.STATE_NOT_PRESENT.equals(isimState)) {
            // Keeps the current ISIM state
        } else {
            // FIXME: need to correct the codes
            if (isIsimLoaded()) {
                setIsimState(IISIM.STATE_REFRESH_STARTED, subId);
            }

            if (IISIM.STATE_LOADED.equals(isimState)) {
                if (IISIM.STATE_REFRESH_STARTED.equals(mIsimState)
                        || IISIM.STATE_NOT_READY.equals(mIsimState)) {
                    setIsimState(IISIM.STATE_REFRESH_COMPLETED, subId);
                } else if (IIISIM.STATE_NOT_PRESENT.equals(mIsimState)) {
                    setIsimState(IISIM.STATE_LOADED, subId);
                } else {
                    setIsimState(IISIM.STATE_REFRESH_STARTED, subId);
                    setIsimState(IISIM.STATE_REFRESH_COMPLETED, subId);
                }
            }
        }

        setIsimState(isimState, subId);
        */
    }

    private void handleSIMStateChanged(Intent intent) {
        if (intent == null) {
            return;
        }

        int subId = intent.getIntExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX,
                MSimUtils.INVALID_SUB_ID);

        if (MSimUtils.isMultiSimEnabled()) {
            if (!MSimUtils.isValidSubId(subId)) {
                return;
            }

            int slotId = intent.getIntExtra(SubscriptionManager.EXTRA_SLOT_INDEX,
                    MSimUtils.INVALID_SLOT_ID);

            if (slotId != mSlotId) {
                return;
            }
        }

        boolean isSimLoaded = isIccLoaded();
        String iccState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

        if (iccState == null) {
            ImsLog.w(mSlotId, "iccState is null");
            return;
        }

        // ISSUE: SIM activation/deactivation procedure of MTK chipset
        if (MSimUtils.isMultiSimEnabled()) {
            if (isSimLoadedBySimActivation(getIccState(), iccState, subId)) {
                ImsLog.i(mSlotId, "SimActivation :: " + iccState + " >> LOADED");
                iccState = IccCardConstants.INTENT_VALUE_ICC_LOADED;
            }
        }

        setIccState(iccState, subId);

        // Adaptation for legacy interface {
        if (isSimLoaded && !isIccLoaded()) {
            sendIsimState(mSlotId, subId, IISIM.STATE_REFRESH_STARTED);
        } else if (isIccLoaded()) {
            String oldIsimState = getIsimState();

            if (IISIM.STATE_REFRESH_STARTED.equals(oldIsimState)) {
                sendIsimState(mSlotId, subId, IISIM.STATE_REFRESH_COMPLETED);
            } else {
                TelephonyManager tm = AppContext.getTelephonyManager(subId);

                // The isApplicationOnUicc is a hidden API, so it will be removed
                // when a formal ISIM interface is adapted.
                if (tm != null && tm.isApplicationOnUicc(TelephonyManager.APPTYPE_ISIM)) {
                    sendIsimState(mSlotId, subId, IISIM.STATE_LOADED);
                } else {
                    sendIsimState(mSlotId, subId, IISIM.STATE_NOT_PRESENT);
                }
            }
        } else if (IccCardConstants.INTENT_VALUE_ICC_NOT_READY.equals(getIccState())) {
            sendIsimState(mSlotId, subId, IISIM.STATE_NOT_READY);
        }
        // }

        if (!isSimLoaded && isIccLoaded() && (mSIMStateHandler != null)) {
            // Check ISIM state explicitly after 3 seconds
            mSIMStateHandler.sendEmptyMessageDelayed(
                    EVENT_UPDATE_ISIM_STATE,
                    INTERVAL_FOR_EXPLICIT_ISIM_STATE_UPDATE);
        }
    }

    private void handleISIMStateChanged(IsimEvent event) {
        if (event == null) {
            return;
        }

        int subId = event.getSubId();

        if (MSimUtils.isMultiSimEnabled()) {
            int slotId = event.getSlotId();

            if (slotId != mSlotId) {
                ImsLog.d(mSlotId, "Slot is not matched in ISIM state.");
                return;
            }
        }

        String isimState = event.getState();

        if (isimState == null) {
            ImsLog.w(mSlotId, "isimState is null");
            return;
        }

        setIsimState(isimState, subId);
    }

    private void handleServiceRegistered() {
        if (mPsiReadDone == false) {
            mRetryCountForPsi = 0;
            readPsiDelayed();
        }
    }

    private void processOperatorSpecificOnSimStateChanged() {
        if (isIccLoaded() || IccCardConstants.INTENT_VALUE_ICC_IMSI.equals(getIccState())) {
            if (isPsiRequired()) {
                ImsLog.i(mSlotId, "Reset the EFpsismsc value in IMS DB");
                mPsiReadDone = false;

                // Reset PSI value for new SIM card
                ISharedState iss = (ISharedState)AgentFactory.getAgent(
                        AgentFactory.SHARED_STATE, mSlotId);

                if (iss != null && iss.isNativeBootCompleted()) {
                    mPsiResetRequired = false;
                    setPSIValueToDB("");
                } else {
                    mPsiResetRequired = true;
                }

                readPsiDelayed();
                readSMSPInfo();
            }
        }

        // IMS-LAMPLITE : CACHE_ECC
        if (ImsConstants.PLATFORM_LAMPLITE) {
            if (mSIMStateHandler != null) {
                mSIMStateHandler.removeMessages(EVENT_CACHE_ECC_LIST);
            }

            if (isIccLoaded()) {
                mRetryCountForCacheEccList = 0;
                cacheEccList();
            }
        }
    }

    private void setIccState(String state, int subId) {
        if (!mIccState.equals(state)) {
            ImsLog.i(mSlotId, "IccState :: " + mIccState + " >> " + state + ", subId=" + subId);

            mIccState = state;
            if (state.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                mIccLoaded = true;
            } else {
                mIccLoaded = false;
                mNoIsimAppNotificationRequired = true;
            }

            // Notify the state changed to the applications
            if (mSimStateChangedRegistrants != null) {
                mSimStateChangedRegistrants.notifyResult(mIccState);
            }

            if (isIccLoaded()) {
                IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
                if (aosInfo != null) {
                    aosInfo.notifyPhoneNumberState(false, PhoneNumberState.SIM_LOADED);
                }
            }

            processOperatorSpecificOnSimStateChanged();
        }
    }

    private void setIsimState(String state, int subId) {
        if (!mIsimState.equals(state)) {
            ImsLog.i(mSlotId, "IsimState :: " + mIsimState + " >> " + state + ", subId=" + subId);

            boolean isNotReady = false;

            if (state.equals(IISIM.STATE_REFRESH_COMPLETED)) {
                if (mIsimState.equals(IISIM.STATE_NOT_READY)) {
                    isNotReady = true;
                }

                mIsimState = IISIM.STATE_LOADED;
            } else if (state.equals(IISIM.STATE_REFRESH_STARTED)
                    && mIsimState.equals(IISIM.STATE_NOT_READY)) {
                // Keep the current ISIM state: NOT_READY
                ImsLog.i(mSlotId, "IsimState :: Keep NOT_READY state on REFRESH_STARTED");
            } else {
                mIsimState = state;
            }

            notifyIsimState(isNotReady ? mIsimState : state);

            if (mIsimStateChangedRegistrants != null) {
                mIsimStateChangedRegistrants.notifyResult(mIsimState);
            }
        } else {
            // OTA_OPENING ::
            // This is for OTA opening scenario when no ISIM application in UICC.
            if ("KR".equals(OperatorInfo.getCountry(mSlotId))
                    && mNoIsimAppNotificationRequired
                    && isIccLoaded()
                    && state.equals(IISIM.STATE_NOT_PRESENT)) {
                mNoIsimAppNotificationRequired = false;
                notifyIsimState(state);
            }
        }
    }

    private void updateIsimState() {
        if (isIccLoaded() && !isIsimLoaded()) {
            String oldIsimState = getIsimState();
            String newIsimState = oldIsimState;
            int subId = MSimUtils.getSubId(mSlotId);

            if (IISIM.STATE_REFRESH_STARTED.equals(oldIsimState)) {
                newIsimState = IISIM.STATE_REFRESH_COMPLETED;
            } else {
                TelephonyManager tm = AppContext.getTelephonyManager(subId);

                // The isApplicationOnUicc is a hidden API, so it will be removed
                // when a formal ISIM interface is adapted.
                if (tm != null && tm.isApplicationOnUicc(TelephonyManager.APPTYPE_ISIM)) {
                    newIsimState = IISIM.STATE_LOADED;
                } else {
                    newIsimState = IISIM.STATE_NOT_PRESENT;
                }
            }

            ImsLog.i(mSlotId, "updateIsimState: slotId=" + mSlotId +
                    ", subId=" + subId + ", state=" + newIsimState);

            handleISIMStateChanged(new IsimEvent(mSlotId, subId, newIsimState));
        }
    }

    private void readSMSPInfo() {
        AppContext.runTask(() -> {
                mSMSPAddress = ImsExtApi.Uicc.getEfSmsP();
                ImsLog.i(mSlotId, "SMS Center Address: " + ImsLog.hiddenString(mSMSPAddress));
            }, 0);
    }

    private void notifyIsimState(String state) {
        ImsLog.i(mSlotId, "isim state =" + state);

        //Notify ISIM state change to native, to handle race condition.
        int nState = IsimState.NOT_PRESENT;
        if (state.equals(IISIM.STATE_NOT_READY)) {
            nState = IsimState.NOT_READY;
        } else if (state.equals(IISIM.STATE_LOADED)) {
            nState = IsimState.LOADED;
        } else if (state.equals(IISIM.STATE_REFRESH_STARTED)) {
            nState = IsimState.REFRESH_STARTED;
        } else if (state.equals(IISIM.STATE_REFRESH_COMPLETED)) {
            nState = IsimState.REFRESH_COMPLETED;
        }

        IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
        if (aosInfo != null) {
            aosInfo.notifyIsimState(nState);
        }

        if (mSystem != null) {
            mSystem.notifyISIMState(NOTIFICATION_ISIM_STATE_CHANGED, state);
        }
    }

    private void sendIsimState(int slotId, int subId, String state) {
        IsimEvent isimEvent = new IsimEvent(slotId, subId, state);

        ImsLog.d(slotId, "[IMS-ISIM][Slot" + slotId + "] subId=" + subId + ", state=" + state);

        Message.obtain(mSIMStateHandler, EVENT_ISIM_STATE_CHANGED, isimEvent).sendToTarget();
    }

    private static boolean checkIsimProvisioningViaOta() {
        // SKT_OTA_OPENING, NOT_USED from N-OS. Leave it for history.
        return false;
    }

    private static void executeOnIsimThread(Runnable r) {
        new Thread(r, "IsimThread").start();
    }

    public static boolean isSimLoadedBySimActivation(
            String oldState, String newState, int subId) {
        if (ImsProperties.isChipVendorMtk()
                && IccCardConstants.INTENT_VALUE_ICC_READY.equals(newState)
                && (IccCardConstants.INTENT_VALUE_ICC_NOT_READY.equals(oldState)
                        || IccCardConstants.INTENT_VALUE_ICC_UNKNOWN.equals(oldState)
                        || IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(oldState))) {
            int slotId = MSimUtils.getSlotId(subId);
            return IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(
                    MSimUtils.getSimState(slotId));
        }

        return false;
    }

    // -----------------------------------------------------------
    private final class AuthData {
        public String mNonce;
        public long mOwner;

        AuthData(String nonce, long owner) {
            mNonce = nonce;
            mOwner = owner;
        }
    }

    private class SIMStateReceiverListener extends BroadcastReceiver {
        IntentFilter mIntentFilter = new IntentFilter();

        public SIMStateReceiverListener() {
            mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        }

        public IntentFilter getFilter() {
            return mIntentFilter;
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (mSIMStateHandler == null) {
                ImsLog.d(mSlotId, "SIMStateAgent is stopped; " + ImsLog.lastSubString(action, "."));
                return;
            }

            ImsLog.i(mSlotId, ImsLog.lastSubString(action, "."));

            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
                Message.obtain(mSIMStateHandler,
                    EVENT_SIM_STATE_CHANGED, intent).sendToTarget();
            }
        }
    }

    private final class SIMStateHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.i(mSlotId, "handleMessage :: what=" + msg.what);

            switch (msg.what) {
                case EVENT_RETRY_GET_PSI_INFO:
                    handleRetryGetPsiInfo();
                    break;
                case REQUEST_READ_FILE_ATTRIBUTE:
                    handleRequestForIsimFileAttributes(msg.arg1);
                    break;
                case REQUEST_READ_RECORD:
                    handleRequestForIsimFileRecord(msg.arg1, msg.arg2);
                    break;
                case REQUEST_ISIM_AUTH:
                    handleRequestForIsimAuthentication((AuthData)msg.obj);
                    break;
                case REQUEST_USIM_AUTH:
                    handleRequestForUsimAuthentication((AuthData)msg.obj);
                    break;
                case EVENT_NATIVE_BOOT_COMPLETED:
                    setIccState(MSimUtils.getSimState(mSlotId), MSimUtils.getSubId(mSlotId));
                    // setIsimState(getIsimStateFromPhone(), MSimUtils.getSubId(mSlotId));
                    break;
                default:
                    handleInternalMessage(msg);
                    break;
            }
        }

        private void handleInternalMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SIM_STATE_CHANGED:
                    handleSIMStateChanged((Intent)msg.obj);
                    break;
                case EVENT_ISIM_STATE_CHANGED:
                    handleISIMStateChanged((IsimEvent)msg.obj);

                    // Remove a message if delayed ISIM state update is posted
                    if (mSIMStateHandler != null) {
                        boolean hasUpdateMessage =
                                mSIMStateHandler.hasMessages(EVENT_UPDATE_ISIM_STATE);

                        mSIMStateHandler.removeMessages(EVENT_UPDATE_ISIM_STATE);

                        if (!isIsimLoaded() && hasUpdateMessage) {
                            updateIsimState();
                        }
                    }
                    break;
                case EVENT_SERVICE_REGISTERED:
                    handleServiceRegistered();
                    break;
                case EVENT_DATA_SUBSCRIPTION_CHANGED:
                    handleDataSubscriptionChanged(msg.arg1);
                    break;
                case EVENT_UPDATE_ISIM_STATE:
                    updateIsimState();
                    break;
                // IMS-LAMPLITE : CACHE_ECC
                case EVENT_CACHE_ECC_LIST:
                    cacheEccList();
                    break;
                default:
                    break;
            }
        }
    }

    private final class SubscriptionListenerProxy extends SubscriptionListener {
        public SubscriptionListenerProxy() {
        }

        @Override
        public void onDefaultDataSubscriptionChanged(int subId) {
            if (MSimUtils.isMultiImsEnabled() || MSimUtils.isMultiImsEnabledOnDssv()) {
                return;
            }

            // If a default data subscription is changed,
            // sets the SIM state to NOT_READY explicitly.
            setIccState(IccCardConstants.INTENT_VALUE_ICC_NOT_READY, MSimUtils.INVALID_SUB_ID);

            if (mSIMStateHandler != null) {
                Message.obtain(mSIMStateHandler,
                        EVENT_DATA_SUBSCRIPTION_CHANGED, subId, 0).sendToTarget();
            } else {
                handleDataSubscriptionChanged(subId);
            }
        }
    }

    private class RegistrationListener extends AosRegistrationListener {
        @Override
        public void notifyRegistered(int networkType, int featureTagBits,
                Set<String> featureTags) {
            ImsLog.i(mSlotId, "notifyRegistered");
            Message.obtain(mSIMStateHandler, EVENT_SERVICE_REGISTERED).sendToTarget();
        }
    }
}
