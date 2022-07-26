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
