/**
 * IMSProvisioning
 * Role
 *         Internal setting menu which used only for IMS solution
 *         Access to gims.db file.
 */

package com.android.imsstack.test.menu;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.widget.Toast;

import com.android.imsstack.R;
import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.core.config.ImsDbController;
import com.android.imsstack.core.config.ProviderDBUpdateHelper;
import com.android.imsstack.core.config.ProviderInterface;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosInfo.ServiceSetting;
import com.android.imsstack.internal.enabler.ImsStateStore;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SettingsUtils;

import java.io.File;
import java.util.Locale;

public class IMSProvisioning extends IMSSetting_BASE {

    // Constants--------------------------------------------------
    private static final int FEATURE_TIMER = 0x1;
    private static final int FEATURE_100REL = 0x2;
    private static final int FEATURE_PRECONDITION = 0x4;
    private static final int FEATURE_PEM = 0x8;
    // deprecated. use bConfSub configuration.
    //private static final int FEATURE_CONFSUB = 0x10;
    private static final int FEATURE_UPDATE_PROVISION = 0x20;
    private static final int FEATURE_TERMINATE_EARLYDIALOG = 0x40;

    private static final String IMS_AOS = "gims_aos";

    // UC
    private static final String IMS_UC_APP = "gims_uc_app";
    private static final String IMS_UC_SERVICE_VOIP = "gims_uc_service_voip";
    private static final String IMS_UC_SERVICE_VT = "gims_uc_service_vt";
    private static final String IMS_UC_SERVICE_UC = "gims_uc_service_uc";
    private static final String IMS_UC_SERVICE_EMERGENCY = "gims_uc_service_emergency";
    private static final String IMS_UC_SESSION_VOIP = "gims_uc_session_voip";
    private static final String IMS_UC_SESSION_VT = "gims_uc_session_vt";
    private static final String IMS_UC_SESSION_COMMON = "gims_uc_session_common";
    private static final String IMS_UC_EMERGENCY = "gims_uc_emergency";
    private static final String IMS_UC_CONFERENCE = "gims_uc_conference";

    // refer to ConferenceConfiguration.h
    private static final long UC_CONFERENCE_FLAG_CONFERENCE                     = 0x00000001;
    private static final long UC_CONFERENCE_FLAG_CONFERENCE_SUBSCRIPTION        = 0x00000002;
    private static final long UC_CONFERENCE_FLAG_REFER_SUBSCRIPTION             = 0x00000004;
    private static final long UC_CONFERENCE_FLAG_SUBSCRIPTION_OUTDIALOG         = 0x00000008;
    private static final long UC_CONFERENCE_FLAG_REFERRED_BY                    = 0x00010000;
    private static final long UC_CONFERENCE_FLAG_ADD_USER_PARAMETER             = 0x00100000;
    private static final long UC_CONFERENCE_FLAG_SUBSCRIPTION_FOR_PARTICIPANT   = 0x00200000;
    private static final long UC_CONFERENCE_FLAG_PACKAGE_VERSION_CHECK          = 0x01000000;
    private static final long UC_CONFERENCE_FLAG_SUBSCRIPTION_FIRST             = 0x04000000;
    private static final long UC_CONFERENCE_FLAG_PAID_PREFERRED                 = 0x08000000;
    private static final long UC_CONFERENCE_FLAG_REUSE_REFERTO_URI              = 0x10000000;
    private static final long UC_CONFERENCE_FLAG_USE_REFER                      = 0x20000000;
    private static final long UC_CONFERENCE_FLAG_USE_DISCONNECTING_STATUS       = 0x40000000;
    private static final long UC_CONFERENCE_FLAG_USE_REFERTO_EX_HDR             = 0x80000000L;

    //// AUDIO for Wifi Env
    private static final String IMS_COM_MEDIA_AUDIO = "gims_com_media_audio";

    //// SMS
    private static final String KEY_SMS_PREFIX = "sms_";
    private static final String IMS_SMS = "ims_service_mts";
    private static final String IMS_SMS_SIP = "gims_com_sip_sms";
    ////Ut
                                                  // Base, ATT, GSMA
    private static final String IMS_MMTEL_SERVER_INFO = "gims_com_service_mmtel";
    private static final String IMS_MMTEL_GBA_INFO = "gims_com_service_gba";
    private static final String IMS_MMTEL_SERVER_CF_INFO = "gims_com_service_mmtel_cf";
    private static final String IMS_MMTEL_SERVER_CB_INFO = "gims_com_service_mmtel_cb";

    // Emergency call configuration
    private static final String IMS_COM_EMERGENCY = "gims_com_emergency";

    private boolean mAdministrativeConfigChanged = false;

    //localFeature flag setting value
    private int localFeatureValue_voip = 0;
    private int localFeatureValue_vt = 0;

    //new Conference Call config
    private long mUCConferenceFeature = 0;

    //IMSProvisioningList
    private IMSProvisioningList mProvisioningList = null;

    private SettingSlot mSettingSlot = null;
    private IMSProvisionSlotListener mIMSProvisionSlotListener = null;

    // Static loading materials ----------------------------------
    // Public methods --------------------------------------------
    // Interface implementation methods --------------------------
    @SuppressWarnings("deprecation")
        @Override
    public void onCreate(Bundle savedInstanceState) {
        ImsLog.d("");

        super.onCreate(savedInstanceState);

        int nSlotID = getSlotID();

        if (nSlotID < 0) {
            mSettingSlot = new SettingSlot(this, getListView());
            mIMSProvisionSlotListener = new IMSProvisionSlotListener();
            mSettingSlot.setSlotList(mIMSProvisionSlotListener);
        } else {
            initProvisioning(nSlotID);
        }

    }

    private void initProvisioning(int slotID) {

        setSlotID(slotID);

        Context appContext = getApplicationContext();
        removeTypeChangedKeys(appContext);

        addPreferencesFromResource(R.xml.imsprovisioning);

        // CarrierConfig {
        Preference carrierConfigMenu = getPreference("carrier_config_menu");

        if (carrierConfigMenu != null) {
            Intent intent = carrierConfigMenu.getIntent();

            if (intent != null) {
                intent.putExtra(MSimUtils.EXTRA_KEY_SLOT_ID, slotID);
            }
        }
        // }

        SettingUtil.init(appContext);

        mProvisioningList = new IMSProvisioningList(this, getSlotID());
        //register listeners concerning about common features which are used without operator.
        registerCommonListener();
        //register listeners concerning about operator specific features used in global operator
        //(ATT, TMO, KDDI...)
        registerGlobalOperatorListener();
        //register listeners concerning about operator specific features used in domestic operator
        //(SKT, KT, LGU)
        registerDomesticOperatorListener();
    }

    private void removeTypeChangedKeys(Context context) {

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = sp.edit();
        editor.remove("bUsingEPDN_LTE");
        editor.remove("bUsingEPDN_WIFI");
        editor.apply();
    }

    private void registerCommonListener() {

        //Subscriber information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.Subscriber.TABLE_NAME, getSlotID()),
                mProvisioningList.mSubscriberFeature);

        // AoS Application information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.AoSApplication.TABLE_NAME, getSlotID()),
                mProvisioningList.mAoSApplicationFeature);

         // AoS Condition information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.AoSCondition.TABLE_NAME, getSlotID()),
                mProvisioningList.mAoSConditionFeature);

        // AoS Connection information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.AoSConnection.TABLE_NAME, getSlotID()),
                mProvisioningList.mAoSConnectionFeature);

        // AoS Handle information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.AoSHandle.TABLE_NAME, getSlotID()),
                mProvisioningList.mAoSHandleFeature);

        // AoS Provider information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.AoSProvider.TABLE_NAME, getSlotID()),
                mProvisioningList.mAoSProviderFeature);

        // AoS Reg information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.AoSReg.TABLE_NAME, getSlotID()),
                mProvisioningList.mAoSRegFeature);

        // Engine information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.Engine.TABLE_NAME, getSlotID()),
                mProvisioningList.mEngineFeature);

        // SIP information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.SIP.TABLE_NAME, getSlotID()),
                mProvisioningList.mSIPFeature);

        // COM SIP information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                ProviderInterface.SIP.TABLE_NAME_COM, getSlotID()),
                mProvisioningList.mComSIPFeature);

        // Test
        registerTestChangeListener(new TestChangeListener());

        //UC Speicific information
        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "", IMS_UC_APP, "UC_APP::"), mProvisioningList.mUCAppFeature);
        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "voip_", IMS_UC_SERVICE_VOIP, "UC_Service_Voip::"),
                mProvisioningList.mUCServiceFeature);
        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "vt_", IMS_UC_SERVICE_VT, "UC_Service_Vt::"), mProvisioningList.mUCServiceFeature);
        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "uc_", IMS_UC_SERVICE_UC, "UC_Service_Uc::"), mProvisioningList.mUCServiceFeature);
        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "emergency_", IMS_UC_SERVICE_EMERGENCY, "UC_Service_Emergency::"),
                mProvisioningList.mUCServiceFeature);

        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "", IMS_UC_SESSION_COMMON, "UC_Session_Common::"),
                mProvisioningList.mUCSessionCommonFeature);

        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "voip_", IMS_UC_SESSION_VOIP, "UC_Session_Voip::"),
                mProvisioningList.mUCSessionFeature);
        registerUCSessionLocalFeatureChangeListener(new UcSpecificChangeListener(imsDB,
                "voip_", IMS_UC_SESSION_VOIP, "UC_Session_Voip::"));
        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "vt_", IMS_UC_SESSION_VT, "UC_Session_Vt::"), mProvisioningList.mUCSessionFeature);
        registerUCSessionLocalFeatureChangeListener(new UcSpecificChangeListener(imsDB,
                "vt_", IMS_UC_SESSION_VT, "UC_Session_Vt::"));

        registerUCConferenceChangeListener(new UcSpecificChangeListener(imsDB,
                "", IMS_UC_CONFERENCE, "UC_Conference::"));

        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "", IMS_UC_EMERGENCY, "UC_Emergency::"), mProvisioningList.mUCEmergencyFeature);
        registerPrefixSpecificChangeListener(new UcSpecificChangeListener(imsDB,
                "", IMS_UC_CONFERENCE, "UC_Conference::"), mProvisioningList.mUCConferenceFeature);

        // COM Emergency information
        registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                IMS_COM_EMERGENCY, getSlotID()), mProvisioningList.mComEmergencyFeature);
    }

    private void registerGlobalOperatorListener() {

        //FakeSubscriber information  // ATT, Base, GSMA, KT, LGU , SKT
        if (!ImsGlobal.isOperator(getSlotID(), "VZW")) {
            registerPrefixSpecificChangeListener(new PrefixSpecificChangeListener(imsDB,
                    "fake_", ProviderInterface.Subscriber.TABLE_NAME_FAKE,
                    "Subscriber::fake", getSlotID()), mProvisioningList.mFakeSubscriberFeature);
        }
        else {
            removePreference(getString(R.string.ims_key_fake_subscriber));
        }

        // SMS information //ATT, GSMA, Base, KDDI, LGU, VZW,
        if (!ImsGlobal.isOperator(getSlotID(), "SKT") ) {
            registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                    IMS_SMS, getSlotID()) , mProvisioningList.mSMSFeature);
        }
        else {
            removePreference(getString(R.string.ims_key_sms));
        }

        //Ut information // ATT, GSMA, Base
        if (!ImsGlobal.isOperator(getSlotID(), "KT")
                && !ImsGlobal.isOperator(getSlotID(), "LGU")
                && !ImsGlobal.isOperator(getSlotID(), "SKT")
                && !ImsGlobal.isOperator(getSlotID(), "VZW")) {
            registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                    IMS_MMTEL_SERVER_INFO, getSlotID()),
                    mProvisioningList.mMMTELServerInfoFeature);
            registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                    IMS_MMTEL_GBA_INFO, getSlotID()),
                    mProvisioningList.mMMTELGBAInfoFeature);
            registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                    IMS_MMTEL_SERVER_CF_INFO, getSlotID()),
                    mProvisioningList.mMMTELServerInfoCFFeature);
            registerIMSCommonChangeListener(new ProvisioningChangeListener(imsDB,
                    IMS_MMTEL_SERVER_CB_INFO, getSlotID()),
                    mProvisioningList.mMMTELServerInfoCBFeature);
        }
        else {
            removePreference(getString(R.string.ims_label_mmtel_server_info));
        }

        // VoWiFi connection preference for Telstra
        if (!ImsGlobal.isOperator(getSlotID(), "TEL")) {
            removePreference(getString(R.string.ims_key_vowifi));
        }
    }

    private void registerDomesticOperatorListener() {
        // SMS SIP information  // LGU, SKT, KT
        if (ImsGlobal.isOperator(getSlotID(), "LGU")
                || ImsGlobal.isOperator(getSlotID(), "KT")) {
            registerPrefixSpecificChangeListener(new PrefixSpecificChangeListener(imsDB,
                    KEY_SMS_PREFIX, IMS_SMS_SIP, "SMS::SIP", getSlotID()),
                    mProvisioningList.mSMSSIPFeature);
        } // ATT, GSMA, Base, KDDI, VZW
        else if (!ImsGlobal.isOperator(getSlotID(), "SKT")) {
            removePreference(getString(R.string.ims_key_sms), getString(R.string.ims_key_sms_sip));
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onPause() {
        super.onPause();

        if (mAdministrativeConfigChanged) {
            //SubscriberInformationHelper.getInstance().init();
            mAdministrativeConfigChanged = false;
        }

        if (bChange == true) {
            ImsLog.e("Preferece Operator Changed. IMS Process will be killed. ");
            android.os.Process.killProcess(android.os.Process.myPid());
        }
    }

    // Private/Protected methods ---------------------------------

    protected void registerIMSCommonChangeListener(IMSCommonChangeListener listener,
            IMSProvisioningList.SettingFeature feature) {
        ImsLog.d("tableName : " + listener.getTableName());

        Cursor cursor = DBUtils.DB.getCursor(mSlotID, imsDB, listener.getTableName());
        if (cursor == null) {
            ImsLog.e("Cursor :: (" + listener.getTableName() + ") is null");
            return;
        }

        String columns[] = cursor.getColumnNames();

        SettingUtil.displayColumns(columns, getSlotID());
        // Move the cursor to the first row
        if (cursor.moveToNext() != true) {
            ImsLog.e("cursor.moveToNext() is null !!!");
            cursor.close();
            return;
        }

        if (feature.checkboxPreferenceList != null) {
            boolean isSubscriberTable = ProviderInterface.Subscriber.TABLE_NAME.equals(
                    listener.getTableName());

            if (isSubscriberTable) {
                for (String preference : feature.checkboxPreferenceList) {
                    int adminFeature = SettingUtil.getBitmaskForAdminFeatures(preference);

                    if (adminFeature == 0) {
                        setCheckboxPreference(cursor, preference, columns, listener);
                    } else {
                        setCheckboxPreferenceForAdminFeatures(cursor,
                                preference, adminFeature, columns, listener);
                    }
                }
            } else {
                for (String preference : feature.checkboxPreferenceList) {
                     setCheckboxPreference(cursor, preference, columns, listener);
                }
            }
        }

        if (feature.editPreferenceList != null) {
            for (String preference : feature.editPreferenceList) {
                setEditTextPreference(cursor, preference, columns, listener);
            }
        }
        if (feature.neditPreferenceList != null) {
            for (String preference : feature.neditPreferenceList) {
                setNumberEditTextPreference(cursor, preference, columns, listener);
            }
        }

        if (feature.listPreferenceList != null) {
            for (String preference : feature.listPreferenceList) {
                setListPreference(cursor, preference, columns, listener);
            }
        }

        // Close the cursor; release all the resource related with the cursor
        cursor.close();
    }

    private void registerPrefixSpecificChangeListener(PrefixSpecificChangeListener listener,
            IMSProvisioningList.SettingFeature feature) {
        ImsLog.d("tableName : " + listener.getTableName());

        Cursor cursor = DBUtils.DB.getCursor(mSlotID, imsDB, listener.getTableName());
        if (cursor == null) {
            ImsLog.e("Cursor :: (" + listener.getTableName() + ") is null");
            return;
        }

        String columns[] = cursor.getColumnNames();
        // Move the cursor to the first row
        SettingUtil.displayColumns(columns, getSlotID());

        // Move the cursor to the first row
        if (cursor.moveToNext() != true) {
            ImsLog.e("cursor.moveToNext() is null !!!");
            cursor.close();
            return;
        }

        String prefix = listener.getPrefix();

        if (feature.checkboxPreferenceList != null) {
            for (String preference : feature.checkboxPreferenceList) {
                setCheckboxPreference(cursor, prefix, preference, columns, listener);
            }
        }

        if (feature.editPreferenceList != null) {
            for (String preference : feature.editPreferenceList) {
                setEditTextPreference(cursor, prefix, preference, columns, listener);
            }
        }

        if (feature.editPreferenceList != null) {
            for (String preference : feature.neditPreferenceList) {
                setNumberEditTextPreference(cursor, prefix, preference, columns, listener);
            }
        }

        if (feature.editPreferenceList != null) {
            for (String preference : feature.listPreferenceList) {
                setListPreference(cursor, prefix, preference, columns, listener);
            }
        }

        // Close the cursor; release all the resource related with the cursor
        cursor.close();
    }

    private void registerUCSessionLocalFeatureChangeListener(UcSpecificChangeListener listener) {
        ImsLog.d("tableName : " + listener.getTableName());

        Cursor cursor = DBUtils.DB.getCursor(mSlotID, imsDB, listener.getTableName());
        if (cursor == null) {
            ImsLog.e("Cursor :: IMS_UCSessionChange::" + listener.getTableName() + " is null");
            return;
        }

        String columns[] = cursor.getColumnNames();

        SettingUtil.displayColumns(columns, getSlotID());

        // Move the cursor to the first row
        if (cursor.moveToNext() != true) {
            ImsLog.e("cursor.moveToNext() is null !!!");
            cursor.close();
            return;
        }

        String prefix = listener.getPrefix();

        //Process for Localfeatrue bit setting.
        int index;
        CheckBoxPreference checkbox = null;

        index = SettingUtil.getColumnIndex(columns, getString(R.string.ims_key_uc_nLocalFeature));

        checkbox = getCheckBoxPreference(prefix +
                getString(R.string.ims_key_uc_nLocalFeature_timer));
        setListener(checkbox, listener);
        setBitValue(cursor, index, checkbox, FEATURE_TIMER, prefix);

        checkbox = getCheckBoxPreference(prefix +
                getString(R.string.ims_key_uc_nLocalFeature_100rel));
        setListener(checkbox, listener);
        setBitValue(cursor, index, checkbox, FEATURE_100REL, prefix);

        checkbox = getCheckBoxPreference(prefix +
                getString(R.string.ims_key_uc_nLocalFeature_precondition));
        setListener(checkbox, listener);
        setBitValue(cursor, index, checkbox, FEATURE_PRECONDITION, prefix);

        checkbox = getCheckBoxPreference(prefix +
                getString(R.string.ims_key_uc_nLocalFeature_pem));
        setListener(checkbox, listener);
        setBitValue(cursor, index, checkbox, FEATURE_PEM, prefix);

        checkbox = getCheckBoxPreference(prefix +
                getString(R.string.ims_key_uc_nLocalFeature_update_provision));
        setListener(checkbox, listener);
        setBitValue(cursor, index, checkbox, FEATURE_UPDATE_PROVISION, prefix);

        checkbox = getCheckBoxPreference(prefix +
                getString(R.string.ims_key_uc_nLocalFeature_terminate_earlydialog));
        setListener(checkbox, listener);
        setBitValue(cursor, index, checkbox, FEATURE_TERMINATE_EARLYDIALOG, prefix);

        // Close the cursor; release all the resource related with the cursor
        cursor.close();

        // Update LocalFeature Total value
        setLocalFeatureTotalValue();
    }

    private void registerUCConferenceChangeListener(UcSpecificChangeListener listener) {
        ImsLog.d("registerUCConferenceChangeListener");
        ImsLog.d("tableName : " + listener.getTableName());

        Cursor cursor = DBUtils.DB.getCursor(mSlotID, imsDB, listener.getTableName());
        if (cursor == null) {
            ImsLog.e("Cursor :: IMS_UCConferenceChange::" + listener.getTableName() + " is null");
            return;
        }

        String columns[] = cursor.getColumnNames();

        SettingUtil.displayColumns(columns, getSlotID());

        // Move the cursor to the first row
        if (cursor.moveToNext() != true) {
            ImsLog.e("cursor.moveToNext() is null !!!");
            cursor.close();
            return;
        }

        String prefix = listener.getPrefix();

        int index;
        CheckBoxPreference checkbox = null;

        index = SettingUtil.getColumnIndex(columns, getString(R.string.ims_key_uc_features));

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bConference));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_CONFERENCE);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bConferenceSubscription));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox,
                UC_CONFERENCE_FLAG_CONFERENCE_SUBSCRIPTION);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bReferSubscription));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_REFER_SUBSCRIPTION);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bSubscriptionOutDialog));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox,
                UC_CONFERENCE_FLAG_SUBSCRIPTION_OUTDIALOG);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bReferredByHeader));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_REFERRED_BY);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bAddUserParameter));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_ADD_USER_PARAMETER);

        checkbox = getCheckBoxPreference(
                getString(R.string.ims_key_uc_bSubscriptionForParticipant));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox,
                UC_CONFERENCE_FLAG_SUBSCRIPTION_FOR_PARTICIPANT);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bPackageVersionCheck));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox,
                UC_CONFERENCE_FLAG_PACKAGE_VERSION_CHECK);

        checkbox = getCheckBoxPreference(
                getString(R.string.ims_key_uc_bSubscriptionFirstAndRefer));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_SUBSCRIPTION_FIRST);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bPaidPreferredThanFrom));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_PAID_PREFERRED);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bReuseReferToUri));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_REUSE_REFERTO_URI);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bUseReferToInvite));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_USE_REFER);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bUseDisconnectingStatus));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox,
                UC_CONFERENCE_FLAG_USE_DISCONNECTING_STATUS);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_uc_bUseReferToExHeader));
        setListener(checkbox, listener);
        setUCConferenceFeatureBit(cursor, index, checkbox, UC_CONFERENCE_FLAG_USE_REFERTO_EX_HDR);

        // Close the cursor; release all the resource related with the cursor
        cursor.close();

        // Update Total value
        setUCConferenceFeatureTotalValue();
    }

    private void setUCConferenceFeatureBit(Cursor cursor,
            int index, CheckBoxPreference checkbox, long checkBit) {
        if (index == (-1)) {
            ImsLog.e("index is (-1)");
            return;
        }

        if (cursor == null) {
            ImsLog.e("Cursor is null");
            return;
        }

        if (checkbox == null) {
            ImsLog.e("CheckBoxPreference is null");
            return;
        }

        if (checkBit == -1) {
            ImsLog.e("checkBit is -1");
            return;
        }

        String value = cursor.getString(index);

        if (value == null) {
            return;
        }

        value = value.toLowerCase();
        value = value.replaceFirst("^0x", "");

        long valueLong = 0;
        try {
            valueLong = Long.parseLong(value, 16);
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }

        ImsLog.d("mUCConferenceFeature checkBit = " + checkBit);

        if ((valueLong & checkBit) / checkBit > 0) {
            checkbox.setChecked(true);
            mUCConferenceFeature = mUCConferenceFeature | valueLong;
            checkbox.setSummary("true");
        } else {
            checkbox.setChecked(false);
            mUCConferenceFeature = mUCConferenceFeature | valueLong;
            checkbox.setSummary("false");
        }

        ImsLog.d("mUCConferenceFeature = " + mUCConferenceFeature);
    }

    private void registerTestChangeListener(TestChangeListener listener) {
        ListPreference list = null;
        EditTextPreference edit = null;
        CheckBoxPreference checkbox = null;

        list = getListPreference(getString(R.string.restart_ims_key));
        setListener(list, listener);

        list = getListPreference(getString(R.string.ims_deregistration_key));
        setListener(list, listener);

        edit = getEditTextPreference(getString(R.string.ims_key_preference_operator));

        if (edit != null) {
            setListener(edit, listener);
            String prefOperator = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_PREF_OPERATOR, "", getSlotID());

            edit.setText(prefOperator);
            edit.setSummary(prefOperator);
        } else {
            ImsLog.e("Cannot find preference. key : " +
                    getString(R.string.ims_key_preference_operator));
        }

        edit = getEditTextPreference(getString(R.string.ims_key_preference_country));
        if (edit != null) {
            setListener(edit, listener);
            String prefCountry = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_PREF_COUNTRY, "", getSlotID());

            edit.setText(prefCountry);
            edit.setSummary(prefCountry);
        } else {
            ImsLog.e("Cannot find preference. key : " +
                    getString(R.string.ims_key_preference_country));
        }

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_kr_enabler));

        if (checkbox != null) {
            setListener(checkbox, listener);

            boolean krEnabler = ImsPrivateProperties.Persistent.getBoolean(
                    ImsPrivateProperties.Persistent.KEY_PREF_KR_ENABLER, getSlotID());

            checkbox.setChecked(krEnabler);
        }

        edit = getEditTextPreference(getString(R.string.ims_key_trace_option));
        if (edit != null) {
            setListener(edit, listener);
            String strCurValue = selectImsDB(ProviderInterface.Engine.TABLE_NAME,
                    ProviderInterface.Engine.TRACE_OPTION);
            if (TextUtils.isEmpty(strCurValue) == false) {
                ImsLog.d("current tarce_option in DB : " + strCurValue);
                if (strCurValue.startsWith("0x")) {
                    strCurValue = strCurValue.substring(2);
                }

                int intCurValue = 0;
                try {
                    intCurValue = Integer.parseInt(strCurValue, 16);
                }
                catch (Exception e) {
                    e.printStackTrace();
                }

                int value = intCurValue & 0x000000FF;
                String strValue = String.format("0x%02x", value);
                edit.setText(strValue);
                edit.setSummary(strValue);
            }
        }

        edit = getEditTextPreference(getString(R.string.ims_key_trace_module));
        if (edit != null) {
            String strCurValue = "";

            setListener(edit, listener);

            strCurValue = selectImsDB(ProviderInterface.Engine.TABLE_NAME,
                    ProviderInterface.Engine.TRACE_MODULE);
            if (TextUtils.isEmpty(strCurValue) == false) {
                ImsLog.d("current tarce_module in DB : " + strCurValue);

                edit.setText(strCurValue);
                edit.setSummary(strCurValue);
            } else {
                Toast.makeText(getApplicationContext(), "Trace Module is currently empty",
                        Toast.LENGTH_LONG).show();
                edit.setSummary(strCurValue);
            }
        }

        // for VoLTE AT Command
        checkbox = getCheckBoxPreference(getString(R.string.ims_key_volte_atcmd_supported));

        if (checkbox != null) {
            setListener(checkbox, listener);

            boolean bAtCmdSupported = getAtCmdSupported();

            checkbox.setChecked(bAtCmdSupported);
            checkbox.setSummary(String.valueOf(bAtCmdSupported));
        }

        // Load Preset Config. go2071, red.kim
        // for IMS Test Bed
        list = getListPreference(getString(R.string.ims_key_ims_testbed));

        if (list != null) {
            setListener(list, listener);
        }

        // 20160421 jeongjin.lee for IMS Test Equipment
        list = getListPreference(getString(R.string.ims_key_ims_testequipment));

        if (list != null) {
            setListener(list, listener);
        }

        // for RnS Conf.
        list = getListPreference(getString(R.string.ims_key_rns_config));
        if (list != null) {
            setListener(list, listener);
        }

        // for PTCRB Conf.
        list = getListPreference(getString(R.string.ims_key_ptcrb_config));

        if (list != null) {
            list.setEnabled(true);
            setListener(list, listener);
        }

        // for Init. Config.
        list = getListPreference(getString(R.string.ims_key_init_config));
        setListener(list, listener);

        // for IMS Test Server seyoon0802.jung
        list = getListPreference(getString(R.string.ims_key_wifi_env));
        setListener(list, listener);

        // Recreate Configuration (DB initialization fully)
        list = getListPreference(getString(R.string.ims_key_recreate_config));
        setListener(list, listener);

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_vonr_enabled));

        if (checkbox != null) {
            setListener(checkbox, listener);
            checkbox.setChecked(ImsProperties.isVoNrEnabled());
        }

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_wifi_test_enabled));

        if (checkbox != null) {
            setListener(checkbox, listener);
        }

        // VoLTE Setting Menu
        checkbox = getCheckBoxPreference(getString(R.string.ims_key_volte_setting_enabled));
        if (checkbox != null) {
            setListener(checkbox, listener);
            checkbox.setChecked(SettingsUtils.isDataNetworkEnhanced4GLteMode(
                    getApplicationContext(), getSlotID()));
        }

        checkbox = getCheckBoxPreference(getString(R.string.ims_key_use_predefined_user_agent));

        if (checkbox != null) {
            setListener(checkbox, listener);
            checkbox.setChecked(getUsePredefinedUserAgent());
        }
    }

    // FOR LGU SMS
    private void updateAoSProfileForSMS(boolean onoff) {
        final String SMS_ON = "3";
        final String SMS_OFF = "2";

        String count = (onoff) ? SMS_ON : SMS_OFF;
        String AOS =
            "[Uniqueness]\n" +
            "aos_profile=2\n" +
            "\n" +
            "[aos_profile_0]\n" +
            "id=aos_app_0\n" +
            "connection=aos_connection_0\n" +
            "reg=aos_reg_0\n" +
            "service_count=" + count + "\n" +
            "app_id_0=gims.com.lgu.app.uc\n" +
            "service_id_0=gims.com.lgu.service.vt\n" +
            "app_id_1=gims.com.lgu.app.uc\n" +
            "service_id_1=gims.com.lgu.service.uc\n" +
            "app_id_2=gims.com.lgu.app.sms\n" +
            "service_id_2=gims.com.lgu.service.sms\n" +
            "\n" +
            "[aos_profile_1]\n" +
            "id=aos_app_1\n" +
            "connection=aos_connection_1\n" +
            "reg=aos_reg_1\n" +
            "service_count=0\n" +
            "\n";

        ImsLog.w("updateAoSProfileForSMS :: onoff=" + onoff);

        int affectedRows = 0;
        ContentValues cvs = new ContentValues();

        cvs.put("conf", AOS);

        imsDB.beginTransaction();

        try {
            affectedRows = imsDB.update(IMS_AOS, cvs,
                    ImsDbController.selectForSlot(mSlotID), null);
            imsDB.setTransactionSuccessful();
        } catch (SQLiteException e) {
            e.printStackTrace();
        } finally {
            DBUtils.DB.endTransaction(imsDB);
        }

        ImsLog.w("AoS :: updated row count: " + affectedRows);
    }

    private void setBitValue(Cursor cursor,
            int index, CheckBoxPreference checkbox, int checkBit, String type) {
        if (index == (-1)) {
            ImsLog.e("index is (-1)");
            return;
        }

        if (cursor == null) {
            ImsLog.e("Cursor is null");
            return;
        }

        if (checkbox == null) {
            ImsLog.e("CheckBoxPreference is null");
            return;
        }

        if (checkBit == -1) {
            ImsLog.e("checkBit is -1");
            return;
        }

        String value = cursor.getString(index);

        if (value == null) {
            return;
        }

        int valueInt = Integer.parseInt(value);

        if ((valueInt & checkBit) / checkBit > 0) {
            checkbox.setChecked(true);
            if (type.equalsIgnoreCase("voip_")) {
                localFeatureValue_voip = localFeatureValue_voip | valueInt;
            }
            else {
                localFeatureValue_vt = localFeatureValue_vt | valueInt;
            }
            checkbox.setSummary("true");
        }
        else {
            checkbox.setChecked(false);
            if (type.equalsIgnoreCase("voip_")) {
                localFeatureValue_voip = localFeatureValue_voip | valueInt;
            }
            else {
                localFeatureValue_vt = localFeatureValue_vt | valueInt;
            }
            checkbox.setSummary("false");
        }
    }

    private void setLocalFeatureTotalValue() {
        // Display Total value in menu
        EditTextPreference edit_voip = getEditTextPreference(
                getString(R.string.ims_key_uc_nLocalFeature_voip_total));
        if (edit_voip != null) {
            edit_voip.setTitle(getString(R.string.ims_label_uc_nLocalFeature_total) +
                    " : " + Integer.toString(localFeatureValue_voip));
        }

        EditTextPreference edit_vt = getEditTextPreference(
                getString(R.string.ims_key_uc_nLocalFeature_vt_total));
        if (edit_vt != null) {
            edit_vt.setTitle(getString(R.string.ims_label_uc_nLocalFeature_total) +
                    " : " + Integer.toString(localFeatureValue_vt));
        }
    }

    private void setUCConferenceFeatureTotalValue() {

        ImsLog.d("setUCConferenceFeatureTotalValue : " + Long.toHexString(mUCConferenceFeature));
        // Display Total value in menu
        EditTextPreference edit = getEditTextPreference(getString(R.string.ims_key_uc_features));

        if (edit != null) {
            edit.setTitle("Conference Call Feature" + " : " +
                    Long.toHexString(mUCConferenceFeature));
        }
    }

    private String selectImsDB(String tableName, String key) {
        ImsLog.d("getString :: tableName [ " + tableName + " ] key: " + key);

        String value = "";
        try {
            value = DBUtils.DB.getString(mSlotID, imsDB, tableName, key, "");
        } catch (SQLiteException e) {
            e.printStackTrace();
        }

        return value;
    }

    private boolean isImsDebugModeHandlingRequired() {
        return ImsGlobal.isOperator(getSlotID(), "ATT")
                || ImsGlobal.isOperator(getSlotID(), "TMO")
                || ImsGlobal.isOperator(getSlotID(), "MPCS")
                || ImsGlobal.isOperator(getSlotID(), "TRF");
    }

    private boolean isImsTestModeChangeRequiredForApp() {
        return ImsGlobal.isOperator(getSlotID(), "VZW")
                || ImsGlobal.isCountry(getSlotID(), "CN")
                || ImsGlobal.isCountry(getSlotID(), "KR");
    }

    private void notifyEvent(int event, int wParam, int lParam) {
        ISystem system = SystemInterface.getInstance().getSystem(getSlotID());

        if (system != null) {
            system.notifyEvent(event, wParam, lParam);
        }
    }

    //---------------------------------------------------------------------------------------------

    private class ProvisioningChangeListener extends IMSCommonChangeListener
    {
        // Constants--------------------------------------------------
        // Variables--------------------------------------------------
        // Static loading materials ----------------------------------
        // Public methods --------------------------------------------
        public ProvisioningChangeListener(SQLiteDatabase imsDB, String dbTable, int slotID) {
            super(imsDB, dbTable, slotID);
        }

        // Interface implementation methods --------------------------
        public boolean onPreferenceChange(Preference preference, Object newValue) {

            String key = preference.getKey();

            super.onPreferenceChange(preference, newValue);

            // Additional process after Subscriber related DB table update.
            if (mTableName.equals(ProviderInterface.Subscriber.TABLE_NAME)) {
                handleSubscriberUpdate(key, newValue);
            }

            // Additional process after SIP related DB table update.
            else if (mTableName.equals(ProviderInterface.SIP.TABLE_NAME_COM)) {
                if (ImsGlobal.isOperator(getSlotID(), "VZW")
                        || ImsGlobal.isOperator(getSlotID(), "KDDI")) {
                    handleCOMSIPUpdate(key, newValue);
                }

                if (key.equals(getString(R.string.ims_key_service_version))) {
                    ImsPrivateProperties.Persistent.set(
                            ImsPrivateProperties.Persistent.KEY_CONFIG_USER_AGENT,
                            newValue.toString(), getSlotID());
                }
            }

            // Additional process after SMS related DB table update.
            else if (mTableName.equals(IMS_SMS)) {
                handleSMSUpdate(key, newValue);
            }

            //summary change
            SettingUtil.setSummary(preference, newValue);
            return true;
        }

        // Private/Protected methods ---------------------------------
        public void handleSubscriberUpdate (String key, Object newValue) {

            if (key.equals(getString(R.string.ims_key_home_domain_name))) {
                ISystem system = SystemInterface.getInstance().getSystem(getSlotID());
                if (system == null) {
                    return;
                }
                system.notifyEvent(ImsEventDef.IMS_EVENT_CONFIG_UPDATE, 1, 0);
            }

            int adminFeature = SettingUtil.getBitmaskForAdminFeatures(key);

            if ((adminFeature > 0) || ProviderInterface.Subscriber.SERVICES.equals(key)) {
                mAdministrativeConfigChanged = true;

                boolean isDebugOn = false;

                if (adminFeature == ProviderInterface.Subscriber.AdminFeatures.DEBUG) {
                    isDebugOn = Boolean.valueOf(newValue.toString());
                    ImsLog.setDebugOn(isDebugOn);
                }

                // red.kim, In user mode, disable debug level log according to "admin_debug"
                if (isImsDebugModeHandlingRequired()) {
                    if (adminFeature == ProviderInterface.Subscriber.AdminFeatures.DEBUG) {
                        String option;

                        if (!isDebugOn) {
                            // disable privacy log
                            option = "0x0001010F";
                        } else {
                            option = "0x0001000F";
                        }

                        ImsLog.updateConfig(option);

                        notifyEvent(ImsEventDef.IMS_EVENT_CONFIG_UPDATE,
                                (ImsEventDef.IMS_CONFIG_CAT_10002 << 16), 0);
                    }
                }
                else if (OperatorInfo.isEnablerTypeForNonOperator(getSlotID())
                        && !ImsConstants.DBG) {
                    if (adminFeature == ProviderInterface.Subscriber.AdminFeatures.DEBUG) {
                        String option = selectImsDB(
                                ProviderInterface.Engine.TABLE_NAME,
                                ProviderInterface.Engine.TRACE_OPTION);
                        StringBuilder sb = new StringBuilder(option);

                        if (!isDebugOn) {
                            // disable privacy log
                            // 0x0001000X -> 0x0001010X
                            option = sb.substring(0, 7) + "1" + sb.substring(8);
                        } else {
                            // 0x0001010X -> 0x0001000X
                            option = sb.substring(0, 7) + "0" + sb.substring(8);
                        }

                        ImsLog.updateConfig(option);

                        notifyEvent(ImsEventDef.IMS_EVENT_CONFIG_UPDATE,
                                (ImsEventDef.IMS_CONFIG_CAT_10002 << 16), 0);
                    }
                }
            }

            // IMS test mode: "admin_testmode"
            if ((adminFeature == ProviderInterface.Subscriber.AdminFeatures.TESTMODE)
                    && isImsTestModeChangeRequiredForApp()) {
                int value = "true".equalsIgnoreCase(newValue.toString()) ? 1 : 0;
                SettingUtil.setTestModeForApp(getApplicationContext(), value);
            }

            // Test: carrier-configuration {
            if (key.equals(getString(R.string.ims_key_pcscf_address_0))) {
                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_CONFIG_PCSCF_ADDRESS_LIST,
                        newValue.toString(), getSlotID());
            } else if (key.equals(getString(R.string.ims_key_home_domain_name))) {
                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_CONFIG_HOME_DOMAIN_NAME,
                        newValue.toString(), getSlotID());
            } else if (key.equals(getString(R.string.ims_key_impi))) {
                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_CONFIG_IMPI,
                        newValue.toString(), getSlotID());
            } else if (key.equals(getString(R.string.ims_key_impu_0))) {
                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_CONFIG_IMPU_LIST,
                        newValue.toString(), getSlotID());
            }
            // }
        }

        public void handleCOMSIPUpdate (String key, Object newValue) {

            int nvCmd = 0;
            int updateItem = 0;

            if ( key.equals(getString(R.string.ims_key_sip_timer_t1)) ) {
                updateItem = 2;
            } else if ( key.equals(getString(R.string.ims_key_sip_timer_t2)) ) {
                updateItem = 3;
            } else if ( key.equals(getString(R.string.ims_key_sip_timer_tf)) ) {
                updateItem = 4;
            }

            if (nvCmd != 0) {
                String tv = newValue.toString();

                ImsLog.w("tv=" + nvCmd + "," + tv);

                ISystem system = SystemInterface.getInstance().getSystem(getSlotID());

                if (system != null) {
                    system.notifyEvent(ImsEventDef.IMS_EVENT_CONFIG_UPDATE, updateItem, 0);
                }
            }
            return;
        }

        public void handleSMSUpdate (String key, Object newValue) {
            ImsLog.d("");

            String smsValue = newValue.toString();

            if (smsValue == null) {
                ImsLog.e("smsValue is null");
                return;
            }

            if (key.equals(getString(R.string.ims_key_sms_format))
                    && !ImsGlobal.isOperator(getSlotID(), "KT")
                    && !ImsGlobal.isOperator(getSlotID(), "LGU")
                    && !ImsGlobal.isOperator(getSlotID(), "SKT")) {
                ISystem system = SystemInterface.getInstance().getSystem(getSlotID());

                if (system != null) {
                    system.notifyEvent(ImsEventDef.IMS_EVENT_CONFIG_UPDATE, (int)5, 0);
                }
            }
            else if (key.equals(getString(R.string.ims_key_sms_over_ip_network))
                    && !ImsGlobal.isOperator(getSlotID(), "KDDI")
                    && !ImsGlobal.isOperator(getSlotID(), "KT")
                    && !ImsGlobal.isOperator(getSlotID(), "LGU")
                    && !ImsGlobal.isOperator(getSlotID(), "SKT")) {
                ISystem system = SystemInterface.getInstance().getSystem(getSlotID());

                if (system != null) {
                    system.notifyEvent(ImsEventDef.IMS_EVENT_CONFIG_UPDATE, (int)6, 0);
                }
            }
            else if (key.equals(getString(R.string.ims_key_sms_over_ip_network))
                    && ImsGlobal.isOperator(getSlotID(), "LGU")) {
                updateAoSProfileForSMS(Boolean.valueOf(smsValue));
            }
            else if (key.equals(getString(R.string.ims_key_sms_over_ip_network))
                    && ImsGlobal.isOperator(getSlotID(), "KT")) {
                ContentValues cv = new ContentValues();

                cv.put(ProviderInterface.SMS.SMS_OVER_IP_NETWORK, smsValue);

                try {
                    int result = getContentResolver().update(ProviderInterface.SMS.CONTENT_URI,
                                    cv, ImsDbController.selectForSlot(mSlotID), null);
                    ImsLog.i("DEBUG update :: result=" + result);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private class UcSpecificChangeListener extends PrefixSpecificChangeListener
    {

        // Constants--------------------------------------------------
        // Variables--------------------------------------------------
        // Static loading materials ----------------------------------
        // Public methods --------------------------------------------
        public UcSpecificChangeListener(SQLiteDatabase imsDB,
                String keyPrefix, String tableName, String logTag) {
            super(imsDB, keyPrefix, tableName, logTag, getSlotID());
        }

        // Interface implementation methods --------------------------
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {

            String key = preference.getKey();

            ImsLog.d(mLogTag + ":: key: " + key + " -> value:" + newValue.toString());

            if (imsDB == null) {
                ImsLog.w("DB table (" + mTableName + ") is null");
                return false;
            }

            int affectedRows = 0;
            ContentValues values = new ContentValues();

            values = localFeatureValueHelper(newValue, key, values);

            // Trim the key prefix, "vt_" / "voip_" / "sms_"

            imsDB.beginTransaction();

            try {
                affectedRows = imsDB.update(mTableName, values,
                        ImsDbController.selectForSlot(mSlotID), null);
                imsDB.setTransactionSuccessful();
            } catch (SQLiteException e) {
                e.printStackTrace();
            } finally {
                DBUtils.DB.endTransaction(imsDB);
            }

            ImsLog.w(" :: updated row count: " + affectedRows);

            if (affectedRows != 1) {
                ImsLog.e("Update fails");
                return false;
            }

            setLocalFeatureTotalValue();
            setUCConferenceFeatureTotalValue();
            SettingUtil.setSummary(preference, newValue);
            return true;
        }

        // Private/Protected methods ---------------------------------
        public ContentValues localFeatureValueHelper(Object newValue, String key,
        ContentValues values) {

            if (key.contains("voip_nLocalFeature")) {
                localFeatureValue_voip = localFeatureSettingProcess(localFeatureValue_voip, key);
                values.put((getString(R.string.ims_key_uc_nLocalFeature_voip)).substring(
                        mKeyPrefix.length()), Integer.toString(localFeatureValue_voip));
            }
            else if (key.contains("vt_nLocalFeature")) {
                localFeatureValue_vt = localFeatureSettingProcess(localFeatureValue_vt, key);
                values.put((getString(R.string.ims_key_uc_nLocalFeature_vt)).substring(
                        mKeyPrefix.length()), Integer.toString(localFeatureValue_vt));
            }
            else if (key.contains("UCConference")) {
                mUCConferenceFeature = setUCConferenceCallConfiguration(mUCConferenceFeature, key);
                String value = Long.toHexString(mUCConferenceFeature);
                ImsLog.w("mUCConferenceFeature : " + value);
                values.put((getString(R.string.ims_key_uc_features)).substring(
                        mKeyPrefix.length()), "0x" + value);
            }
            else if (newValue.toString().equals("true")) {
                values.put(key.substring(mKeyPrefix.length()), "1");
            }
            else if (newValue.toString().equals("false")) {
                values.put(key.substring(mKeyPrefix.length()), "0");
            }
            else {
                values.put(key.substring(mKeyPrefix.length()), newValue.toString());
            }
            return values;
        }

        private int localFeatureSettingProcess(int localFeatureValue, String key) {
            if (key.contains("timer")) {
                return localFeatureValue ^ FEATURE_TIMER;
            }
            else if (key.contains("100rel")) {
                return localFeatureValue ^ FEATURE_100REL;
            }
            else if (key.contains("precondition")) {
                return localFeatureValue ^ FEATURE_PRECONDITION;
            }
            else if (key.contains("pem")) {
                return localFeatureValue ^ FEATURE_PEM;
            }
            else if (key.contains("update_provision")) {
                return localFeatureValue ^ FEATURE_UPDATE_PROVISION;
            }
            else if (key.contains("terminate_earlydialog")) {
                return localFeatureValue ^ FEATURE_TERMINATE_EARLYDIALOG;
            }
            // If matching string is not exist, return original localFeatureValue.
            return localFeatureValue;
        }

        // new conference call
        private long setUCConferenceCallConfiguration(long config, String key) {

            ImsLog.d("setUCConferenceCallConfiguration");

            if (key.equals(getString(R.string.ims_key_uc_bConference))) {
                return config ^ UC_CONFERENCE_FLAG_CONFERENCE;
            } else if (key.equals(getString(R.string.ims_key_uc_bConferenceSubscription))) {
                return config ^ UC_CONFERENCE_FLAG_CONFERENCE_SUBSCRIPTION;
            } else if (key.equals(getString(R.string.ims_key_uc_bReferSubscription))) {
                return config ^ UC_CONFERENCE_FLAG_REFER_SUBSCRIPTION;
            } else if (key.equals(getString(R.string.ims_key_uc_bSubscriptionOutDialog))) {
                return config ^ UC_CONFERENCE_FLAG_SUBSCRIPTION_OUTDIALOG;
            } else if (key.equals(getString(R.string.ims_key_uc_bReferredByHeader))) {
                return config ^ UC_CONFERENCE_FLAG_REFERRED_BY;
            } else if (key.equals(getString(R.string.ims_key_uc_bAddUserParameter))) {
                return config ^ UC_CONFERENCE_FLAG_ADD_USER_PARAMETER;
            } else if (key.equals(getString(R.string.ims_key_uc_bSubscriptionForParticipant))) {
                return config ^ UC_CONFERENCE_FLAG_SUBSCRIPTION_FOR_PARTICIPANT;
            } else if (key.equals(getString(R.string.ims_key_uc_bPackageVersionCheck))) {
                return config ^ UC_CONFERENCE_FLAG_PACKAGE_VERSION_CHECK;
            } else if (key.equals(getString(R.string.ims_key_uc_bReuseReferToUri))) {
                return config ^ UC_CONFERENCE_FLAG_REUSE_REFERTO_URI;
            } else if (key.equals(getString(R.string.ims_key_uc_bSubscriptionFirstAndRefer))) {
                return config ^ UC_CONFERENCE_FLAG_SUBSCRIPTION_FIRST;
            } else if (key.equals(getString(R.string.ims_key_uc_bPaidPreferredThanFrom))) {
                return config ^ UC_CONFERENCE_FLAG_PAID_PREFERRED;
            } else if (key.equals(getString(R.string.ims_key_uc_bUseReferToInvite))) {
                return config ^ UC_CONFERENCE_FLAG_USE_REFER;
            } else if (key.equals(getString(R.string.ims_key_uc_bUseDisconnectingStatus))) {
                return config ^ UC_CONFERENCE_FLAG_USE_DISCONNECTING_STATUS;
            } else if (key.equals(getString(R.string.ims_key_uc_bUseReferToExHeader))) {
                return config ^ UC_CONFERENCE_FLAG_USE_REFERTO_EX_HDR;
            }
            return config;
        }
    }

    private class TestChangeListener implements Preference.OnPreferenceChangeListener {
        // Public methods --------------------------------------------
        public TestChangeListener() {
            ImsLog.d("");
        }

        // Interface implementation methods --------------------------
        public boolean onPreferenceChange(Preference preference, Object newValue) {

            String key = preference.getKey();

            ImsLog.d("Test :: key: " + key + " -> value:" + newValue.toString());

            if (key.equals(getString(R.string.ims_key_use_predefined_user_agent))) {
                setUsePredefinedUserAgent(Boolean.valueOf(newValue.toString()));
            }
            else if ( key.equals(getString(R.string.restart_ims_key)) ) {
                handleTestRestartImsKey(newValue);
            }
            else if ( key.equals(getString(R.string.ims_deregistration_key)) ) {
                handleTestImsDeRegiKey(newValue);
            }
            else if ( key.equalsIgnoreCase(getString(R.string.ims_key_preference_operator)) ) {
                handlePreferredOperatorOrCountry(newValue,
                        ImsPrivateProperties.Persistent.KEY_PREF_OPERATOR, key);
            }
            else if ( key.equalsIgnoreCase(getString(R.string.ims_key_preference_country)) ) {
                handlePreferredOperatorOrCountry(newValue,
                        ImsPrivateProperties.Persistent.KEY_PREF_COUNTRY, key);
            }
            else if ( key.equalsIgnoreCase(getString(R.string.ims_key_kr_enabler)) ) {
                boolean oldValue = ImsPrivateProperties.Persistent.getBoolean(
                        ImsPrivateProperties.Persistent.KEY_PREF_KR_ENABLER, getSlotID());
                boolean newValueBool = Boolean.valueOf(newValue.toString());

                if (oldValue != newValueBool) {
                    ImsPrivateProperties.Persistent.setBoolean(
                            ImsPrivateProperties.Persistent.KEY_PREF_KR_ENABLER,
                            newValueBool, getSlotID());

                    bChange = true;

                    ImsLog.w("Preferred KR enabler :: " + oldValue + " >> " + newValueBool);
                }
            }
            else if ( key.equalsIgnoreCase(getString(R.string.ims_key_trace_option)) ) {
                handleTraceOption(newValue);
            }
            else if ( key.equalsIgnoreCase(getString(R.string.ims_key_trace_module)) ) {
                handleTraceModule(newValue);
            }
            else if ( key.equalsIgnoreCase(getString(R.string.ims_key_volte_atcmd_supported)) ) {
                boolean oldAtCmdSupported = getAtCmdSupported();
                boolean newAtCmdSupported = Boolean.valueOf(newValue.toString());

                if (oldAtCmdSupported != newAtCmdSupported) {
                    setAtCmdSupported(newAtCmdSupported);
                    bChange = true;
                }
            }
            else if ( key.equals(getString(R.string.ims_key_ims_testbed)) ) {
                handleImsTestbed(newValue);
            }
            // 20160421 jeongjin.lee for IMS Test Equipment
            else if ( key.equals(getString(R.string.ims_key_ims_testequipment)) ) {
                handleImsTestEquipment(newValue);
            }
            else if ( key.equals(getString(R.string.ims_key_rns_config)) ) {
                handleRnSConf(newValue);
            }
            else if ( key.equals(getString(R.string.ims_key_ptcrb_config))) {
                handlePTCRBConf(newValue);
            }
            else if ( key.equals(getString(R.string.ims_key_init_config)) ) {
                handleInitConfig(newValue);
            }
            else if ( key.equals(getString(R.string.ims_key_wifi_env)) ) {
                handleWifiEnv(newValue);
            }
            else if (key.equals(getString(R.string.ims_key_recreate_config))) {
                handleRecreateConfig(newValue);
            }
            else if (key.equals(getString(R.string.ims_key_vonr_enabled))) {
                handleVoNREnabled(newValue);
            }
            else if (key.equals(getString(R.string.ims_key_wifi_test_enabled))) {
                handleWifiTestEnabled(newValue);
            }
            else if (key.equals(getString(R.string.ims_key_volte_setting_enabled))) {
                handleVoLTESettingEnabled(newValue);
            }
            else {
                ImsLog.e("key value is not matched.");
                return false;
            }
            return true;
        }

        // Private/Protected methods ---------------------------------
        private void handleTestRestartImsKey(Object newValue) {
            if ( "Yes".equalsIgnoreCase(newValue.toString()) ) {
                ImsLog.i("Restart IMS process");
                android.os.Process.killProcess(android.os.Process.myPid());
            } else {
                ImsLog.i("Cancel to restart");
            }
        }

        private void handleTestImsDeRegiKey(Object newValue) {
            IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(getSlotID());
            if (aosInfo == null) {
                return;
            }

            if ("Yes".equalsIgnoreCase(newValue.toString())) {
                ImsLog.i("IMS De-Registration start");
                aosInfo.notifyServiceSetting(ServiceSetting.OFF, 0);
            } else {
                ImsLog.i("IMS De-Registration cancel");
                aosInfo.notifyServiceSetting(ServiceSetting.ON, 0);
            }
        }

        private void handlePreferredOperatorOrCountry(Object newValue,
                String valueKey, String menuKey) {
            String strNewValue = newValue.toString();
            if (!TextUtils.isEmpty(strNewValue)) {
                strNewValue = strNewValue.toUpperCase(Locale.ROOT);
            }

            String strOldValue = ImsPrivateProperties.Persistent.get(valueKey, "", getSlotID());

            if (!strOldValue.equalsIgnoreCase(strNewValue)) {
                EditTextPreference edit = getEditTextPreference(menuKey);
                if (edit == null) {
                    ImsLog.e("Cannot find preference");
                    ImsPrivateProperties.Persistent.set(valueKey, "", getSlotID());
                    return;
                }

                ImsPrivateProperties.Persistent.set(valueKey, strNewValue, getSlotID());
                edit.setText(strNewValue);
                edit.setSummary(strNewValue);
                bChange = true;

                ImsLog.w("Preferred operator or country changed :: key=" + valueKey
                        + ", value: " + strOldValue + " >> " + strNewValue);
            }
        }

        private void handleTraceOption(Object newValue) {
            String strOldValue = selectImsDB(ProviderInterface.Engine.TABLE_NAME,
                                            ProviderInterface.Engine.TRACE_OPTION);
            if (strOldValue.startsWith("0x")) {
                strOldValue = strOldValue.substring(2);
            }
            int intOldValue = Integer.parseInt(strOldValue, 16);

            String strNewValue = newValue.toString();
            if (strNewValue.startsWith("0x")) {
                strNewValue = strNewValue.substring(2);
            }
            int intNewValue = Integer.parseInt(strNewValue, 16);
            if (intNewValue > 0xFF || intNewValue < 0x00) {
                Toast.makeText(getApplicationContext(),
                    "Trace Option can change only with Log Level(D(0x01)/E(0x02)/I(0x04)/T(0x08))",
                    Toast.LENGTH_LONG).show();
                int updatedValue = intOldValue & 0x000000FF;
                String strUpdatedValue = String.format("0x%02x", updatedValue);
                EditTextPreference edit
                        = getEditTextPreference(getString(R.string.ims_key_trace_option));
                if (edit != null) {
                    edit.setText(strUpdatedValue);
                    edit.setSummary(strUpdatedValue);
                }
                return;
            }
            else {
                int updatedValue = (intOldValue & 0xFFFFFF00) + intNewValue;
                String strUpdatedValue = String.format("0x%08x", updatedValue);
                ImsLog.updateConfig(strUpdatedValue);

                ImsLog.w("Trace Option Changed : " + strOldValue + " -> " + strUpdatedValue);

                EditTextPreference edit
                        = getEditTextPreference(getString(R.string.ims_key_trace_option));
                strUpdatedValue = String.format("0x%02x", intNewValue);
                if (edit != null) {
                    edit.setText(strUpdatedValue);
                    edit.setSummary(strUpdatedValue);
                }

                notifyEvent(ImsEventDef.IMS_EVENT_CONFIG_UPDATE,
                        (ImsEventDef.IMS_CONFIG_CAT_10002 << 16), 0);
            }
        }

        private void handleTraceModule(Object newValue) {
            long nOldValue = 0;
            long nNewValue = 0;

            String strOldValue = selectImsDB(ProviderInterface.Engine.TABLE_NAME,
                    ProviderInterface.Engine.TRACE_MODULE);
            if (strOldValue.startsWith("0x")) {
                strOldValue = strOldValue.substring(2);
            }
            try {
                nOldValue = Long.parseLong(strOldValue, 16);
            } catch (Exception e) {
                e.printStackTrace();
            }

            String strNewValue = newValue.toString();
            if (strNewValue.startsWith("0x")) {
                strNewValue = strNewValue.substring(2);
            }
            try {
                nNewValue = Long.parseLong(strNewValue, 16);
            } catch (Exception e) {
                e.printStackTrace();
            }

            if (nNewValue > 0xFFFFFFFFL || nNewValue < 0x00000000L) {
                Toast.makeText(getApplicationContext(),
                    "Trace Module can reference IMS_TRACE_MODULE_ENTYPE 0xXXXXXXXX",
                    Toast.LENGTH_LONG).show();

                String strUpdatedValue = String.format("0x%08x", nOldValue);
                EditTextPreference edit = getEditTextPreference(
                        getString(R.string.ims_key_trace_module));
                if (edit != null) {
                    edit.setText(strUpdatedValue);
                    edit.setSummary(strUpdatedValue);
                }
                return;
            } else {
                String strUpdatedValue = String.format("0x%08x", nNewValue);
                updateImsDB(ProviderInterface.Engine.TABLE_NAME,
                        ProviderInterface.Engine.TRACE_MODULE, strUpdatedValue);
                ImsLog.w("Trace Module Changed : " + strOldValue + " -> " + strUpdatedValue);

                EditTextPreference edit = getEditTextPreference(
                        getString(R.string.ims_key_trace_module));
                if (edit != null) {
                    edit.setText(strUpdatedValue);
                    edit.setSummary(strUpdatedValue);
                }
            }
        }

        private void handleImsTestbed(Object newValue) {
            if ( "Yes".equalsIgnoreCase(newValue.toString()) ) {
                ImsLog.d("set configuration for test bed");

                if ((ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US"))
                    || (ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US"))
                    || (ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US"))
                    || (ImsGlobal.isOperator(getSlotID(), "ATT"))) {

                    //4 <note> for IMS testing in IMS Test Bed.

                    // 1. ipsec is not supported
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_1, "false");

                    // 2. isim is supported
                    ImsDbController.Subscriber.setAdminFeatures(getSlotID()
                            , ProviderInterface.Subscriber.AdminFeatures.ISIM
                            , ProviderInterface.Subscriber.AdminFeatures.USIM);

                    // 3. multiple registration is not supported
                    if ((ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US"))
                        || (ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US"))
                        || (ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US"))) {
                        updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                                ProviderInterface.SIP.COMMON_SIP_FEATURES, "0x17080013");
                    }

                    // 4. conference factory URI is  sip:confserver@one.att.com
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(R.string.ims_key_uc_tConfURI),
                            "sip:confserver@ims.testbed.com");
                    updateImsDB(IMS_UC_SESSION_VT,
                            getApplicationContext().getString(R.string.ims_key_uc_tConfURI),
                            "sip:confserver@ims.testbed.com");
                } else if (ImsGlobal.isOperator(getSlotID(), "VZW")) {
                    ImsStateStore.getMmTelState(getSlotID()).setProvisioned(
                            ImsStateStore.STATE_ACTIVE,
                            ImsStateStore.STATE_ACTIVE,
                            ImsStateStore.STATE_ACTIVE);
                    //IMS Settings > Test > Check VoPS Feature & Check VOIP Capability : OFF
                    updateImsDB(ProviderInterface.Setting.TABLE_NAME,
                            ProviderInterface.Setting.CHECK_VOPS_FEATURE, "false");
                    updateImsDB(ProviderInterface.Setting.TABLE_NAME,
                            ProviderInterface.Setting.CHECK_VOIP_CAPABILITY, "false");
                    //IMS Settings > Test > Test Mask > Ignore IMPU Validation : Check
                    //IMS Settings > Test > Test Mask > Allow Roaming Network : Check
                    //IMS Settings > Test > Test Mask > Show VoLTE Indicator : Check (optional)
                    updateImsDB(ProviderInterface.Setting.TABLE_NAME,
                            ProviderInterface.Setting.TEST_MASK, "7");
                } else if (OperatorInfo.isEnablerTypeForNonOperator(getSlotID())) {
                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(),
                            ProviderInterface.Subscriber.AdminFeatures.ISIM,
                            ProviderInterface.Subscriber.AdminFeatures.USIM);

                    // AoS
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_1, "false");

                    // Media
                    if (ImsGlobal.isOperatorCountry(getSlotID(), "TCL", "MX")) {
                        updateImsDB(ProviderInterface.Media.SubTable.CODEC_AUDIO_VOLTE,
                                "AMR_0_mode_set", "");
                    }

                    // UC
                    updateImsDB(ProviderInterface.SessionVoIP.TABLE_NAME,
                            ProviderInterface.SessionVoIP.CONF_URI,
                            "sip:confserver@ims.testbed.com");
                    updateImsDB(ProviderInterface.SessionVt.TABLE_NAME,
                            ProviderInterface.SessionVt.CONF_URI,
                            "sip:confserver@ims.testbed.com");
                    updateConferenceFeatures(UC_CONFERENCE_FLAG_CONFERENCE_SUBSCRIPTION, false);

                    // Ut
                    updateImsDB(ProviderInterface.MMTel.TABLE_NAME,
                            ProviderInterface.MMTel.MMTEL_PROVISIONING, "false");
                } else if ((ImsGlobal.isCountry(getSlotID(), "JP"))
                            || (ImsGlobal.isOperator(getSlotID(), "USC"))) {
                    // SUSBCRIBE
                    int enabledFeatures = ProviderInterface.Subscriber.AdminFeatures.ISIM;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.TESTMODE;

                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(), enabledFeatures
                            , ProviderInterface.Subscriber.AdminFeatures.USIM);

                    // AoS
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_1, "false");

                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.PS_SUPPORTED, "false");

                    // UC
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(R.string.ims_key_uc_tConfURI),
                            "sip:confserver@ims.testbed.com");
                    updateImsDB(IMS_UC_SESSION_VT,
                            getApplicationContext().getString(R.string.ims_key_uc_tConfURI),
                            "sip:confserver@ims.testbed.com");

                } else if (ImsGlobal.isOperator(getSlotID(), "SPR")) {
                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(),
                            ProviderInterface.Subscriber.AdminFeatures.TESTMODE, 0);

                    // UC
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(R.string.ims_key_uc_tConfURI),
                            "sip:confserver@ims.testbed.com");
                }

                updateImsDB(ProviderInterface.Media.SubTable.VIDEO,
                    ProviderInterface.Media.SubTable.MediaVideo.VIDEO_AVPF_ENABLE, "0");

                Toast.makeText(getApplicationContext(), "Need to be restarted",
                        Toast.LENGTH_LONG).show();
                android.os.Process.killProcess(android.os.Process.myPid());
            }
            else {
                ImsLog.d("Cancel to setTestBedConfig");
            }
        }
        // 20160421 jeongjin.lee for IMS Test Equipment
        private void handleImsTestEquipment(Object newValue) {
            if ( "Yes".equalsIgnoreCase(newValue.toString()) ) {
                ImsLog.d("set configuration for test equipment");
                if (ImsGlobal.isOperator(getSlotID(), "VZW")) {
                    //IMS Settings > Test > OMADM Provisioning > VLT & LVC : both ON
                    ImsStateStore.getMmTelState(getSlotID()).setProvisioned(
                            ImsStateStore.STATE_ACTIVE,
                            ImsStateStore.STATE_ACTIVE,
                            ImsStateStore.STATE_ACTIVE);
                } else if (OperatorInfo.isEnablerTypeForNonOperator(getSlotID())) {
                    // SUSBCRIBE
                    int enabledFeatures = ProviderInterface.Subscriber.AdminFeatures.IMS;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.ISIM;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.TESTMODE;

                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(), enabledFeatures
                            , ProviderInterface.Subscriber.AdminFeatures.USIM);

                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF, "PCO");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_ADDRESS_0, "fc01:cafe::1");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_PORT_0, "5060");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPI, "001010123456789@test.3gpp.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPU_1,
                            "sip:001010123456789@ims.mnc001.mcc001.3gppnetwork.org");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PHONE_CONTEXT, "test.3gpp.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.AUTH_ALGORITHM, "AKAv1-MD5");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.AUTH_REALM, "test.3gpp.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.SCSCF_ADDRESS, "test.3gpp.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_PORT_0, "5060");

                    // AosReg
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_SPI_3GPP_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_SPI_3GPP_1, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.GBA, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IMS_INDICATOR_0, "true");

                    // AoSConnection
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.ACCESS_POLICY_0, "0x20000004");
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.PS_SUPPORTED, "false");

                    // SIP
                    updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                            ProviderInterface.SIP.COMMON_SIP_FEATURES, "0x56060002");
                    updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                            ProviderInterface.SIP.COMMON_TCP_CRITERION_LEN, "1024");
                    updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                            ProviderInterface.SIP.PREFERRED_ID, "sip");
                } else if (ImsGlobal.isCountry(getSlotID(), "JP")) {
                    // SUSBCRIBE
                    int disabledFeatures = ProviderInterface.Subscriber.AdminFeatures.USIM;
                    disabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.DM;

                    int enabledFeatures = ProviderInterface.Subscriber.AdminFeatures.IMS;
                    enabledFeatures = ProviderInterface.Subscriber.AdminFeatures.TESTMODE;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.ISIM;

                    ImsDbController.Subscriber.setAdminFeatures(getSlotID()
                            , enabledFeatures, disabledFeatures);

                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF, "PCO");

                    // SIP
                    updateImsDB(ProviderInterface.SIP.TABLE_NAME_COM,
                            ProviderInterface.SIP.TARGET_SCHEME, "sip");

                    // AosReg
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_1, "false");

                    // AoSConnection
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.PS_SUPPORTED, "false");
                } else if (ImsGlobal.isCountry(getSlotID(), "KR")) {
                    // SUSBCRIBE
                    int enabledFeatures = ProviderInterface.Subscriber.AdminFeatures.IMS;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.ISIM;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.TESTMODE;

                    int disabledFeatures = ProviderInterface.Subscriber.AdminFeatures.USIM;
                    disabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.DM;
                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(),
                            enabledFeatures , disabledFeatures);

                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF, "CONF");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                        ProviderInterface.Subscriber.PCSCF_ADDRESS_0, "172.22.1.201");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_PORT_0, "5060");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_ADDRESS_0, "fc01:cafe::1");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_PORT_0, "5060");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPI, "001010123456789@test.3gpp.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPU_1,
                            "sip:001010123456789@ims.mnc001.mcc001.3gppnetwork.org");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PHONE_CONTEXT, "test.3gpp.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.AUTH_ALGORITHM, "AKAv1-MD5");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.AUTH_REALM, "test.3gpp.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.SCSCF_ADDRESS, "test.3gpp.com");

                    // AosReg
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_SPI_3GPP_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_SPI_3GPP_1, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IMS_INDICATOR_0, "true");

                    // AoSConnection
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.PS_SUPPORTED, "false");
                }
                if (ImsGlobal.isCountry(getSlotID(), "US")
                        || ImsGlobal.isCountry(getSlotID(), "CA")) {
                    // Subscriber
                    int enabledFeatures = ProviderInterface.Subscriber.AdminFeatures.IMS;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.TESTMODE;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.DEBUG;
                    int disabledFeatures = ProviderInterface.Subscriber.AdminFeatures.ISIM;
                    disabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.USIM;
                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(),
                            enabledFeatures, disabledFeatures);

                    // P-CSCF
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF, "PCO,CONF");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_ADDRESS_0, "fc01:cafe::1");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_PORT_0, "5060");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_ADDRESS_1, "172.22.1.201");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_PORT_1, "5060");

                    // AoSConnection
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.MPDN, "true");
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.PS_SUPPORTED, "false");
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.IP_VERSION_0, "64");

                    // AosReg
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.GBA, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_1, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IMS_INDICATOR_0, "true");

                    // VoLTE
                    Settings.Secure.putInt(getApplicationContext().getContentResolver(),
                            SettingsUtils.getKeyForDataNetworkEnhanced4GLteMode(getSlotID()), 1);
                }

                Toast.makeText(getApplicationContext(), "Need to be restarted",
                        Toast.LENGTH_LONG).show();
                android.os.Process.killProcess(android.os.Process.myPid());
            }
            else {
                ImsLog.d("Cancel to setTestEquipmentConfig");

            }
        }
        private void handleWifiEnv(Object newValue) {
            if ( "Yes".equalsIgnoreCase(newValue.toString()) ) {
                ImsLog.d("set configuration for wifi environment");

                if (ImsGlobal.isOperator(getSlotID(), "VZW")) {
                    setPersistentProperty(ImsPrivateProperties.Persistent.KEY_WIFI_TEST, "1");

                    // SUSBCRIBE
                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(), 0,
                            ProviderInterface.Subscriber.AdminFeatures.ISIM);

                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF, "CONF");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_ADDRESS_0, "192.168.0.5");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.HOME_DOMAIN_NAME, "vzims.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPI, "0000@vzims.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPU_0, "sip:0000@vzims.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPU_1, "sip:0000@vzims.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPU_2, "tel:0000");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.AUTH_PASSWORD, "0000");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.AUTH_REALM, "vzims.com");

                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PHONE_CONTEXT, "vzims.com");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.SCSCF_ADDRESS, "vzims.com");
                    // AoS
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.PROFILE_NAME_0, "wifi");
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.ACCESS_POLICY_0, "0xffffffff");
                    // UC
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(R.string.ims_key_uc_tConfURI),
                            "sip:confserver@vzims.com");
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_nMediaThresholdATime), "0");
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_nMediaThresholdHTime), "0");
                    updateImsDB(IMS_UC_SESSION_VT,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_nMediaThresholdATime), "0");
                    updateImsDB(IMS_UC_SESSION_VT,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_nMediaThresholdHTime), "0");
                    // Media
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_audio_volte_tv_rtp_inactivity), "0,0,0");
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_audio_vt_tv_rtp_inactivity), "0,0,0");
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_audio_volte_socket_pos), "ap");
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_audio_vt_socket_pos), "ap");
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_audio_volte_port_rtp_valueset),
                            "49152,49998");
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_audio_vt_port_rtp_valueset), "49152,49998");
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_text_volte_port_rtp_valueset),
                            "49152,49998");
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_text_vt_port_rtp_valueset), "49152,49998");
                }
                else {
                    // SUSBCRIBE
                    int disabledFeatures = ProviderInterface.Subscriber.AdminFeatures.ISIM;
                    disabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.USIM;

                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(), 0, disabledFeatures);

                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF, "CONF");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_ADDRESS_0, "192.168.0.100");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PCSCF_PORT_0, "5060");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.HOME_DOMAIN_NAME, "one.att.net");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPI, "0000@one.att.net");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.IMPU_0, "sip:0000@one.att.net");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.PHONE_CONTEXT, "one.att.net");
                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                            ProviderInterface.Subscriber.SCSCF_ADDRESS, "one.att.net");
                    // SIP
                    updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                            ProviderInterface.SIP.COMMON_TCP_CRITERION_LEN, "4096");
                    // AoS
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_0, "false");
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.PROFILE_NAME_0, "wifi");
                    updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                            ProviderInterface.AoSConnection.ACCESS_POLICY_0, "0xffffffff");
                    // UC
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_tConfURI), "sip:n-way_voice@one.att.net");
                    updateImsDB(IMS_UC_SESSION_COMMON,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_nUETIMER_QOS_FORCE_FAKE), "2");
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_nMediaThresholdAProtocol), "0");
                    updateImsDB(IMS_UC_SESSION_VOIP,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_nMediaThresholdHProtocol), "0");
                    updateImsDB(IMS_UC_SESSION_VT,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_tConfURI), "sip:n-way_voice@one.att.net");

                    // Media
                    updateImsDB(IMS_COM_MEDIA_AUDIO,
                            getApplicationContext().getString(
                                R.string.ims_key_media_audio_volte_socket_pos), "ap");
                }
                Toast.makeText(getApplicationContext(), "Need to be restarted!!",
                        Toast.LENGTH_LONG).show();
                android.os.Process.killProcess(android.os.Process.myPid());
            }
            else
            {
                ImsLog.d("Cancel to setWifiEnv");
            }

        }

        private void handleRnSConf(Object newValue) {
            if (!"Yes".equalsIgnoreCase(newValue.toString())) {
                ImsLog.d("Cancel to set RnS Conf");
                return;
            }
            ImsLog.d("Set configuration for Rns Conf");

            // Subscriber
            int enabledFeatures = ProviderInterface.Subscriber.AdminFeatures.IMS;
            enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.TESTMODE;
            enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.DEBUG;
            int disabledFeatures = ProviderInterface.Subscriber.AdminFeatures.ISIM;
            disabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.USIM;
            ImsDbController.Subscriber.setAdminFeatures(getSlotID(),
                    enabledFeatures, disabledFeatures);

            // P-CSCF
            updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                    ProviderInterface.Subscriber.PCSCF, "PCO,CONF");
            updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                    ProviderInterface.Subscriber.PCSCF_ADDRESS_0, "fc01:cafe::1");
            updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                    ProviderInterface.Subscriber.PCSCF_PORT_0, "5060");
            updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                    ProviderInterface.Subscriber.PCSCF_ADDRESS_1, "172.22.1.201");
            updateImsDB(ProviderInterface.Subscriber.TABLE_NAME,
                    ProviderInterface.Subscriber.PCSCF_PORT_1, "5060");

            // AoSConnection
            updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                    ProviderInterface.AoSConnection.MPDN, "true");
            updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                    ProviderInterface.AoSConnection.PS_SUPPORTED, "false");
            updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                    ProviderInterface.AoSConnection.IP_VERSION_0, "64");

            // AosReg
            updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                    ProviderInterface.AoSReg.GBA, "false");
            updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                    ProviderInterface.AoSReg.IPSEC_0, "false");
            updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                    ProviderInterface.AoSReg.IPSEC_1, "false");
            updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                    ProviderInterface.AoSReg.IMS_INDICATOR_0, "true");

            // VoLTE
            Settings.Secure.putInt(getApplicationContext().getContentResolver(),
                    SettingsUtils.getKeyForDataNetworkEnhanced4GLteMode(getSlotID()), 1);

            // SIP
            updateImsDB(ProviderInterface.SIP.TABLE_NAME_COM,
                    "header_info_target_number_format", "local");
            updateImsDB(ProviderInterface.SIP.TABLE_NAME_COM,
                    ProviderInterface.SIP.TARGET_SCHEME, "tel");

            // UC
            updateImsDB(IMS_UC_SESSION_COMMON, "bPR_Answer", "0");
            updateImsDB(IMS_UC_SESSION_COMMON, "bUse180RPR", "0");
            localFeatureValue_voip &= ~FEATURE_PRECONDITION;
            localFeatureValue_voip &= ~FEATURE_100REL;
            updateImsDB(IMS_UC_SESSION_VOIP, "nLocalFeature",
                    String.valueOf(localFeatureValue_voip));
            localFeatureValue_vt &= ~FEATURE_PRECONDITION;
            localFeatureValue_vt &= ~FEATURE_100REL;
            updateImsDB(IMS_UC_SESSION_VT, "nLocalFeature",
                    String.valueOf(localFeatureValue_vt));

            if (ImsGlobal.isOperator(getSlotID(), "VZW")) {
                // IMS Settings > Test > OMADM Provisioning > VLT & LVC : both ON
                ImsStateStore.getMmTelState(getSlotID()).setProvisioned(
                        ImsStateStore.STATE_ACTIVE,
                        ImsStateStore.STATE_ACTIVE,
                        ImsStateStore.STATE_ACTIVE);
            }

            Toast.makeText(getApplicationContext(), "Need to be restarted",
                    Toast.LENGTH_LONG).show();
            android.os.Process.killProcess(android.os.Process.myPid());
        }

        private void handlePTCRBConf(Object newValue) {
            if ( "Yes".equalsIgnoreCase(newValue.toString()) ) {
                if ((ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US"))
                        || (ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US"))
                        || (ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US"))
                        || (ImsGlobal.isOperator(getSlotID(), "ATT"))
                        || (ImsGlobal.isOperator(getSlotID(), "DCM"))
                        || (ImsGlobal.isOperator(getSlotID(), "KDDI"))
                        || (ImsGlobal.isOperator(getSlotID(), "SBM"))
                        || (ImsGlobal.isCountry(getSlotID(), "CA"))
                        || (ImsGlobal.isOperator(getSlotID(), "USC"))) {
                    ImsLog.d("set configuration for PTCRB Conf");

                    // SUSBCRIBE
                    int enabledFeatures = ProviderInterface.Subscriber.AdminFeatures.IMS;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.ISIM;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.TESTMODE_GCF;
                    enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.DEBUG;

                    if (ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US")) {
                        enabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.TESTMODE;
                    }

                    int disabledFeatures = 0;

                    if (ImsGlobal.isOperator(getSlotID(), "DCM")
                        || ImsGlobal.isOperator(getSlotID(), "SBM")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US")
                        || ImsGlobal.isOperator(getSlotID(), "ATT")) {
                        disabledFeatures |= ProviderInterface.Subscriber.AdminFeatures.USIM;
                    }

                    ImsDbController.Subscriber.setAdminFeatures(getSlotID(),
                            enabledFeatures, disabledFeatures);

                    updateImsDB(ProviderInterface.Subscriber.TABLE_NAME_FAKE,
                            ProviderInterface.Subscriber.IMPU_0,
                            "\"Anonymous\"<sip:anonymous@anonymous.invalid>");

                    // AOS
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_SPI_3GPP_0, "false");
                    updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                            ProviderInterface.AoSReg.IPSEC_SPI_3GPP_1, "false");
                    if ((ImsGlobal.isOperator(getSlotID(), "DCM"))
                        || (ImsGlobal.isOperator(getSlotID(), "KDDI"))) {
                        updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                                ProviderInterface.AoSReg.IPSEC_0_ALGS, "0x00070003");
                        updateImsDB(ProviderInterface.AoSReg.TABLE_NAME,
                                ProviderInterface.AoSReg.IPSEC_1_ALGS, "0x00070003");
                    }

                    // UC
                    updateImsDB(IMS_UC_EMERGENCY,
                            getApplicationContext().getString(
                                R.string.ims_key_uc_bESCheckSOS), "1");
                    if (ImsGlobal.isOperator(getSlotID(), "ATT")) {
                        updateImsDB(ProviderInterface.MediaAudio.TABLE_NAME,
                                ProviderInterface.MediaAudio.RTP_INACTIVITY_0, "0,0,0");
                    }

                    // SIP
                    // Remove "Route" header
                    if ((ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US"))
                        || (ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US"))
                        || (ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US"))) {
                        updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                                ProviderInterface.SIP.COMMON_SIP_FEATURES, "0x170C0013");
                    } else if ((ImsGlobal.isOperator(getSlotID(), "DCM"))
                        || (ImsGlobal.isOperator(getSlotID(), "KDDI"))
                        || (ImsGlobal.isOperator(getSlotID(), "SBM"))
                        || (ImsGlobal.isOperator(getSlotID(), "USC"))) {
                        updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                                ProviderInterface.SIP.COMMON_SIP_FEATURES, "0x56020002");
                    }

                    updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                            ProviderInterface.SIP.REG_EXPIRATION, "600000");
                    updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                            ProviderInterface.SIP.REG_SUB_EXPIRATION, "600000");
                    updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                            ProviderInterface.SIP.COMMON_TCP_CRITERION_LEN, "1024");

                    // add audio media feature
                    int nFeatureTagAudio = 0x00000100;
                    updateFeatureTag(nFeatureTagAudio);

                    // SIP
                    // Change preferred identity to sip
                    if (ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US")
                        || ImsGlobal.isOperator(getSlotID(), "ATT")) {
                        updateImsDB(ProviderInterface.SIP.TABLE_NAME_COM,
                                ProviderInterface.SIP.PREFERRED_ID, "sip");
                    }

                    // AoS Connection
                    if (ImsGlobal.isOperator(getSlotID(), "ATT")) {
                        // it's a change to help ptcrb protocol test cases.
                        // it is asked not to connect pdn when ue is in 2g/3g network.
                        // it connects pdn and makes registration only when in lte.
                        // it maintains pdn and registration when it move from lte to 2g/3g.
                        // if pdn is disconnected in 2g/3g, then don't try to connect pdn again.
                        updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                                ProviderInterface.AoSConnection.ACCESS_POLICY_0, "0x20000004");
                    } else if ((ImsGlobal.isOperator(getSlotID(), "DCM"))
                        || (ImsGlobal.isOperator(getSlotID(), "KDDI"))
                        || (ImsGlobal.isOperator(getSlotID(), "SBM"))) {
                        updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                                ProviderInterface.AoSConnection.PS_SUPPORTED, "false");
                    } else if (ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US")) {
                        // adding 0x00200000 (3g) to the access policy to maintain ims pdn
                        // after srvcc.
                        updateImsDB(ProviderInterface.AoSConnection.TABLE_NAME,
                                ProviderInterface.AoSConnection.ACCESS_POLICY_0, "0x21200004");
                        updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                                getApplicationContext().getString(
                                    R.string.ims_key_sip_tcp_keepalive_timer), "-2");
                    }

                    // MMTEL
                    if (ImsGlobal.isCountry(getSlotID(), "CA")) {
                        updateImsDB(ProviderInterface.MMTel.TABLE_NAME,
                            ProviderInterface.MMTel.MMTEL_SUPPORT_SERVICE, "0x0000001E");
                        updateImsDB(ProviderInterface.MMTel.TABLE_NAME,
                            ProviderInterface.MMTel.MMTEL_SRV_RECORD, "false");
                    } else if (ImsGlobal.isOperatorCountry(getSlotID(), "TMO", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "MPCS", "US")
                        || ImsGlobal.isOperatorCountry(getSlotID(), "TRF", "US")) {
                        updateImsDB(ProviderInterface.MMTel.TABLE_NAME,
                                ProviderInterface.MMTel.MMTEL_TLS, "false");
                        updateImsDB(ProviderInterface.MMTel.TABLE_NAME,
                                ProviderInterface.MMTel.MMTEL_RULEID_FROM_RESPONSE, "false");
                        updateImsDB(ProviderInterface.GBA.TABLE_NAME,
                                ProviderInterface.GBA.GBA_TYPE, "gba_me");
                        updateImsDB(ProviderInterface.GBA.TABLE_NAME,
                                ProviderInterface.GBA.GBA_BSF_PORT, "80");
                        updateImsDB(ProviderInterface.GBA.TABLE_NAME,
                                ProviderInterface.GBA.GBA_SCHEMA, "HTTP");
                    }

                    //precondition
                    if (ImsGlobal.isOperator(getSlotID(), "USC")) {
                        updateImsDB(ProviderInterface.FEATURE.TABLE_NAME,
                            ProviderInterface.FEATURE.FEATURE_SDP_PRECONDITION, "1");
                        updateImsDB(IMS_UC_SESSION_VOIP,
                            "nLocalFeature", "31");
                    }

                    // Media
                    updateImsDB(ProviderInterface.Media.TABLE_NAME,
                            ProviderInterface.Media.SESSION_LEVEL_BW, "true");
                    for (int i = 0; i < 6; i++) {
                        updateImsDB(ProviderInterface.Media.SubTable.CODEC_AUDIO_VOLTE,
                                "AMR_" + i + "_max_red", "0");
                    }

                    if (ImsGlobal.isCountry(getSlotID(), "CA")) {
                        updateImsDB(ProviderInterface.SIP.TABLE_NAME,
                                getApplicationContext().getString(
                                    R.string.ims_key_sip_tcp_keepalive_timer), "-2");
                        updateImsDB(IMS_UC_SESSION_COMMON,
                                getApplicationContext().getString(
                                    R.string.ims_key_uc_nUETIMER_QOS_FORCE_FAKE), "1");
                        updateImsDB(ProviderInterface.MediaAudio.TABLE_NAME,
                                ProviderInterface.MediaAudio.RTP_INACTIVITY_0, "0,0,0");
                    }

                    Toast.makeText(getApplicationContext(), "Need to be restarted",
                            Toast.LENGTH_LONG).show();
                    android.os.Process.killProcess(android.os.Process.myPid());
                } else if (OperatorInfo.isEnablerTypeForNonOperator(getSlotID())) {
                    ImsLog.d("Set Operator for Global GCF Test");
                    ImsPrivateProperties.Persistent.set(
                            ImsPrivateProperties.Persistent.KEY_PREF_OPERATOR,
                            "GCF", getSlotID());
                    ImsPrivateProperties.Persistent.set(
                            ImsPrivateProperties.Persistent.KEY_PREF_COUNTRY,
                            "GLOBAL", getSlotID());

                    ImsLog.w("Preferred Operator : " + ImsPrivateProperties.Persistent.get(
                                ImsPrivateProperties.Persistent.KEY_PREF_OPERATOR, "", getSlotID())
                            + ", Preferred Country : " + ImsPrivateProperties.Persistent.get(
                                ImsPrivateProperties.Persistent.KEY_PREF_COUNTRY, "", getSlotID())
                            );

                    Toast.makeText(getApplicationContext(), "Need to be restarted",
                            Toast.LENGTH_LONG).show();
                    android.os.Process.killProcess(android.os.Process.myPid());
                }
            }
            else {
                ImsLog.d("Cancel to setTestBedConfig");
            }
        }

        private void handleInitConfig(Object newValue) {
            if ( "Yes".equalsIgnoreCase(newValue.toString()) ) {
                ImsLog.d("restore IMS DB");

                ImsPrivateProperties.Persistent.removeTestProperties(getSlotID());

                // DB restore: gims_db
                ProviderDBUpdateHelper.restoreConfigFromXml(getApplicationContext(), getSlotID());

                Toast.makeText(getApplicationContext(), "Need to be restarted!!",
                        Toast.LENGTH_LONG).show();
                android.os.Process.killProcess(android.os.Process.myPid());
            }
            else {
                ImsLog.d("Cancel to initConfig");
            }
        }

        private void handleRecreateConfig(Object newValue) {
            if ("Yes".equalsIgnoreCase(newValue.toString())) {
                ImsLog.d("Recreate-Config");

                if (getUsePredefinedUserAgent()) {
                    setUsePredefinedUserAgent(false);
                }

                String[] gimsDb = new String[] {
                        ProviderInterface.DBFP,
                        // Shared Memory file
                        ProviderInterface.DBFP + "-shm",
                        // Write-Ahead Log file
                        ProviderInterface.DBFP + "-wal",
                        // Journal file
                        ProviderInterface.DBFP + "-journal"
                    };

                for (int i = 0; i < gimsDb.length; i++) {
                    File file = new File(gimsDb[i]);

                    try {
                        if (file.exists()) {
                            file.delete();
                        }
                    } catch (Throwable t) {
                        t.printStackTrace();
                    }
                }

                Toast.makeText(getApplicationContext(),
                        "IMS restarted...", Toast.LENGTH_LONG).show();
                android.os.Process.killProcess(android.os.Process.myPid());
            } else {
                ImsLog.d("Recreate-Config :: Cancelled");
            }
        }

        private void handleVoNREnabled(Object newValue) {
            boolean vonrEnabled = "true".equalsIgnoreCase(newValue.toString());
            ImsLog.d("VoNR-Enabled=" + vonrEnabled);
            // CarrierConfigManager#KEY_VONR_ENABLED_BOOL
        }

        private void handleWifiTestEnabled(Object newValue) {
            boolean testEnabled = "true".equalsIgnoreCase(newValue.toString());
            ImsLog.d("WifiTest-Enabled=" + testEnabled);

            if (testEnabled) {
                ImsPrivateProperties.Persistent.set(ImsPrivateProperties.Persistent.KEY_WIFI_TEST,
                        "1", mSlotID);
            } else {
                ImsPrivateProperties.Persistent.set(ImsPrivateProperties.Persistent.KEY_WIFI_TEST,
                        "0", mSlotID);
            }
        }

        private void handleVoLTESettingEnabled(Object newValue) {
            boolean volteEnabled = "true".equalsIgnoreCase(newValue.toString());
            ImsLog.d("VoLTE-Enabled=" + volteEnabled);

            if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
                Toast.makeText(getApplicationContext(),
                        "Not support VoLTE Setting menu", Toast.LENGTH_LONG).show();
            } else {
                if (volteEnabled) {
                    Settings.Secure.putInt(getApplicationContext().getContentResolver(),
                        SettingsUtils.getKeyForDataNetworkEnhanced4GLteMode(getSlotID()), 1);
                } else {
                    Settings.Secure.putInt(getApplicationContext().getContentResolver(),
                        SettingsUtils.getKeyForDataNetworkEnhanced4GLteMode(getSlotID()), 0);
                }
            }
        }

        private void updateImsDB(String tableName, String key, String value) {
            ImsLog.d(" :: tableName [ " + tableName + " ] key: " + key + " -> value:" + value);

            int affectedRows = 0;
            boolean bUpdateOtherSlot = false;
            ContentValues cvs = new ContentValues();

            cvs.put(key, value);

            if (MSimUtils.isMultiSimEnabled() &&
                (key.equals(ProviderInterface.Engine.TRACE_OPTION)) ||
                (key.equals(ProviderInterface.Engine.TRACE_MODULE))) {
                    //Trace option / Trace module update for other slot as well.
                    bUpdateOtherSlot = true;
            }

            imsDB.beginTransaction();

            try {
                affectedRows = imsDB.update(tableName, cvs,
                        ImsDbController.selectForSlot(mSlotID), null);

                if (bUpdateOtherSlot) {
                    affectedRows += imsDB.update(tableName, cvs,
                            ImsDbController.selectForSlot((mSlotID == 0)? 1 : 0), null);
                    ImsLog.d("Updated other slot as well with new value.");
                }

                imsDB.setTransactionSuccessful();
            } catch (SQLiteException e) {
                e.printStackTrace();
            } finally {
                DBUtils.DB.endTransaction(imsDB);
            }

            ImsLog.w(" :: updated row count: " + affectedRows);
        }

        private void updateFeatureTag(int nUpdatedFeatureTag) {
            String strFeatureTag;
            strFeatureTag = selectImsDB(ProviderInterface.SIP.TABLE_NAME_COM,
                                    ProviderInterface.SIP.FEATURE_TAGS);

            int featureTag = convertStrHexToInt(strFeatureTag);
            strFeatureTag  = String.format("0x%08X", (featureTag | nUpdatedFeatureTag));

            updateImsDB(ProviderInterface.SIP.TABLE_NAME_COM,
                        ProviderInterface.SIP.FEATURE_TAGS, strFeatureTag);
        }

        private void updateConferenceFeatures(long feature, boolean isOn) {
            String value = selectImsDB(ProviderInterface.UCConference.TABLE_NAME,
                    ProviderInterface.UCConference.CONF_FEATURES);

            long features = convertStrHexToLong(value);

            value = String.format("0x%08X",
                    (isOn) ? (features | feature) : (features & (~feature)));

            updateImsDB(ProviderInterface.UCConference.TABLE_NAME,
                    ProviderInterface.UCConference.CONF_FEATURES, value);
        }

        private int convertStrHexToInt(String hex) {
            int value = -1;

            if (hex == null) {
                return value;
            }

            if (hex.startsWith("0x")) {
                hex = hex.substring(2);
            }

            if (hex.isEmpty()) {
                return value;
            }

            try {
                value = Integer.parseInt(hex, 16);
            } catch (NumberFormatException e) {
                ImsLog.e(e.toString());
                e.printStackTrace();
            }

            return value;
        }

        private long convertStrHexToLong(String hex) {
            long value = -1;

            if (hex == null) {
                return value;
            }

            if (hex.startsWith("0x")) {
                hex = hex.substring(2);
            }

            if (hex.isEmpty()) {
                return value;
            }

            try {
                value = Long.parseLong(hex, 16);
            } catch (NumberFormatException e) {
                ImsLog.e(e.toString());
                e.printStackTrace();
            }

            return value;
        }

    }

    private class IMSProvisionSlotListener extends SettingSlot.SlotListener {
        @Override
        public void onSlot(int slotID) {
            initProvisioning(slotID);
        }
    }
}
