package com.android.imsstack.enabler.mtc;

import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.ImsLog;

public class IncomingRejectedMtcCall extends IncomingMtcCall implements Parcelable {

    //------------------------------------------------------------------------------------------//
    public IncomingRejectedMtcCall(Parcel source) {
        super(source);
    }

    @Override
    public boolean isAutoRejectedCall() {
        return true;
    }

    @Override
    public void readFromParcel(Parcel source) {

        callKey = 0;

        int callType = source.readInt();

        OIPType = source.readInt();
        int cnap = source.readInt(); // same as OIPType
        calleePartyNum = "";
        callerPartyNum = source.readString();

        callInfo = new CallInfo(IUMtcCall.SERVICETYPE_NORMAL, callType);

        int videoQuality = MediaInfo.VIDEO_QUALITY_NONE;
        int videoDirection = MediaInfo.DIRECTION_INVALID;
        if (MtcCallUtils.hasVideo(callType)) {
            videoQuality = MediaInfo.VIDEO_QUALITY_QVGA_LS;
            videoDirection = MediaInfo.DIRECTION_SEND_RECEIVE;
        }
        mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_WB, videoQuality,
                MediaInfo.DIRECTION_SEND_RECEIVE, videoDirection, MediaInfo.DIRECTION_INVALID,
                MediaInfo.GTTMODE_INVALID);

        suppInfo = new SuppInfo();
        String cna = source.readString();
        if (callerPartyNum != null && callerPartyNum.equals(cna) == false
                && "".equals(cna) == false) {
            suppInfo.addService_str(SuppInfo.TYPE_CNAP, cna);
        }

        rejectedReason = source.readInt();

        logTag = "";

        ImsLog.d("callKey : " + callKey
                + " OIPType : " + OIPType
                + " callerPartyNum : " + callerPartyNum
                + " cna : " + cna
                + " ArcReason : " + rejectedReason
                 );
    }
}
