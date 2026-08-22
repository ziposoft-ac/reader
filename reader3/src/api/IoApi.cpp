#include "IoApi.h"

#include "api/MqClient.h"


ZMETA_DEF(IoApiClient);



z_status ioBeep(U16 duty, std::initializer_list<Tone> const tones) {
    RemoteBeep_t remote;
    memset(&remote, 0, sizeof(remote));


    int i = 0;
    for (auto t: tones) {
        remote.notes[i] = {t, duty};
        i++;
        if (i == RemoteBeepMaxLength)
            break;
    }
    remote.count = i;
    return mq_send_msg_t<RemoteBeep_t>(ioServiceName, "beepMq", remote);
}

z_status ioLedSet(LedSet_t set) {
    return mq_send_msg_t<LedSet_t>(ioServiceName, "setLed", set);
}

z_status ioLedFlash(LedFlash_t flash) {
    return mq_send_msg_t<LedFlash_t>(ioServiceName, "ledFlash", flash);
}

z_status IoApiClient::buzz(int duty, int f0, int d0, int f1, int d1, int f2, int d2) {

    RemoteBeep_t remote={};
    remote.notes[0]={f0,d0,duty};
    remote.notes[1]={f1,d1,duty};
    remote.notes[2]={f2,d2,duty};
    remote.count=1;
    if (d2) remote.count=3;
    else
        if (d1) remote.count=2;


    return mq_send_msg_t<RemoteBeep_t>(ioServiceName, "beepMq", remote);



}


