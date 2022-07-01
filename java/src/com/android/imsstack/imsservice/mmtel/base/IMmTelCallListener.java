package com.android.imsstack.imsservice.mmtel.base;

import android.telephony.ims.ImsExternalCallState;
import android.telephony.ims.stub.ImsCallSessionImplBase;

import java.util.List;

/**
 * Listener for MMTel call events.
 */
public interface IMmTelCallListener {
    /**
     * This is invoked when incoming call is received.
     */
    void onIncomingCallReceived(ImsCallSessionImplBase session);

    /**
     * This is invoked when IMS dialog state is changed.
     */
    void onImsDialogStateChanged(List<ImsExternalCallState> externalCallDialogs);
}
