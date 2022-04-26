package com.android.imsstack.imsservice;

import android.annotation.Nullable;
import android.telephony.ims.aidl.IImsServiceController;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.stub.ImsFeatureConfiguration;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.SipTransportImplBase;

import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.util.Log;

/**
 * Implements ImsService to provide VoLTE/Emergency/RCS features.
 */
public class ImsService extends android.telephony.ims.ImsService {

    @Override
    public void onCreate() {
        super.onCreate();

        logi("ImsService created");

        ImsServiceController.create(getApplicationContext());
    }

    @Override
    public void onDestroy() {
        logi("ImsService destroy...");

        ImsServiceController.destroy(getApplicationContext());

        super.onDestroy();
    }

    @Override
    public ImsFeatureConfiguration querySupportedImsFeatures() {
        if (!ImsServiceController.isReady()) {
            logi("querySupportedImsFeatures :: not-ready");
            return super.querySupportedImsFeatures();
        }

        logi("querySupportedImsFeatures");

        // It will return the supported features by this ImsService.
        // Generally, the features are the same as defined in AndroidManifest.xml.
        ImsFeatureConfiguration.Builder fcBuilder = new ImsFeatureConfiguration.Builder();
        int simCount = ImsServiceController.getSimCount(getApplicationContext());

        for (int i = 0; i < simCount; i++) {
            fcBuilder.addFeature(i, ImsFeature.FEATURE_MMTEL);
            fcBuilder.addFeature(i, ImsFeature.FEATURE_EMERGENCY_MMTEL);
            fcBuilder.addFeature(i, ImsFeature.FEATURE_RCS);
        }

        return fcBuilder.build();
    }

    @Override
    public @ImsServiceCapability long getImsServiceCapabilities() {
        //TODO uncomment below statements for SIP Delegate Support.
        //Currently SingleRegistration is not supported so we can comment it.
        //return super.CAPABILITY_SIP_DELEGATE_CREATION;

        logi("getImsServiceCapabilities: CAPABILITY_TERMINAL_BASED_CALL_WAITING");
        return (CAPABILITY_TERMINAL_BASED_CALL_WAITING);
    }

    @Override
    public void readyForFeatureCreation() {
        logi("readyForFeatureCreation");
    }

    @Override
    public void enableIms(int slotId) {
        if (!ImsServiceController.isReady()) {
            logi("enableIms :: not-ready, slotId=" + slotId);
            return;
        }

        logi("enableIms :: slotId=" + slotId);

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr != null) {
            isr.enableIms();
        }
    }

    @Override
    public void disableIms(int slotId) {
        if (!ImsServiceController.isReady()) {
            logi("disableIms :: not-ready, slotId=" + slotId);
            return;
        }

        logi("disableIms :: slotId=" + slotId);

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr != null) {
            isr.disableIms();
        }
    }

    @Override
    public MmTelFeature createMmTelFeature(int slotId) {
        logi("createMmTelFeature :: slotId=" + slotId);

        ImsServiceController isc = ImsServiceController.getInstance();

        if (isc == null) {
            logi("No ImsServiceController");
            return null;
        }

        return isc.getMmTelService(slotId);
    }

    @Override
    public RcsFeature createRcsFeature(int slotId) {
        logi("createRcsFeature for slot : " + slotId);

        ImsServiceController isc = ImsServiceController.getInstance();

        if (isc == null) {
            logi("No ImsServiceController");
            return null;
        }

        return isc.getRcsFeature(slotId);
    }

    @Override
    public ImsConfigImplBase getConfig(int slotId) {
        if (!ImsServiceController.isReady()) {
            logi("getConfig :: not-ready, slotId=" + slotId);
            return null;
        }

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr == null) {
            logi("getConfig :: Service is down for phone" + slotId);
            return null;
        }

        return isr.getConfig();
    }

    @Override
    public ImsRegistrationImplBase getRegistration(int slotId) {
        if (!ImsServiceController.isReady()) {
            logi("getRegistration :: not-ready, slotId=" + slotId);
            return null;
        }

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr == null) {
            logi("getRegistration :: Service is down for phone" + slotId);
            return null;
        }

        return isr.getRegistration();
    }

    @Override
    public @Nullable SipTransportImplBase getSipTransport(int slotId) {
        logi("getSipTransport :: slotId=" + slotId);

        if (!ImsServiceController.isReady()) {
            logi("getSipTransport :: not-ready, slotId=" + slotId);
            return null;
        }

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr == null) {
            logi("getSipTransport :: Service is down for phone" + slotId);
            return null;
        }
        return isr.getSipTransport();
    }

    private static void logi(String s) {
        Log.i(Log.TAG, "[GII-IMPL] " + s);
    }
}
