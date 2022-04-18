package com.android.imsstack.enabler.aos;

import java.util.Set;
import android.net.Uri;

public class AosRegistrationListener implements IAosRegistrationListener {
    @Override
    public void notifyRegistered(int networkType, int featureTagBits,
            Set<String> featureTags) {}

    @Override
    public void notifyRegistering(int networkType, int featureTagBits,
            Set<String> featureTags) {}

    @Override
    public void notifyDeregistered(int reason) {}

    @Override
    public void notifyTechnologyChangeFailed(int networkType, int causeCode) {}

    @Override
    public void notifyAssociatedUriChanged(Uri[] uris) {}

    @Override
    public void notifyCapabilitiesUpdateFailed(int capabilities, int networkType,
            int reason) {}
}