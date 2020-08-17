package com.lzm.player.listener;

public interface MOnPcmInfoListener {
    void onPcmInfo(byte[] retPcmBuffer, int retPcmBufferSize);
    void onPcmRate(int samplerate,int bit, int channels);
}
