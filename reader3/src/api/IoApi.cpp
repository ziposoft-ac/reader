#include "IoApi.h"

#include "api/MqClient.h"

ZMETA(IoApiTest) {
    ZACT(testBeep);
    ZCMD(setLed, ZFF_CMD_DEF, "setLed",
        ZPRM(int, color, 1, "color", ZFF_PARAM),
        ZPRM(int, on, 1, "onoff", ZFF_PARAM)
     );

    ZCMD(ledFlash, ZFF_CMD_DEF, "ledFlash",
        ZPRM(int, color, 1, "color", ZFF_PARAM),
        ZPRM(int, on, 1, "onoff", ZFF_PARAM)
 );
};
#ifdef NOGPIO
z_status ioLedSet(LedSet_t set) {
    return zs_ok;
}

z_status ioLedFlash(LedFlash_t flash) {
    return zs_ok;
}
z_status ioBeep(U16 duty, std::initializer_list<Tone> const tones) {

    return zs_ok;
}

#else
z_status ioBeep(U16 duty, std::initializer_list<Tone> const tones) {

    RemoteBeep_t remote;
    memset(&remote,0,sizeof(remote));


    int i=0;
    for (auto t : tones) {
        remote.notes[i]={t,duty};
        i++;
        if (i==RemoteBeepMaxLength)
            break;
    }
    remote.count=i;
    return mq_send_msg_t<RemoteBeep_t>(ioServiceName,"beepMq",remote );

}

z_status ioLedSet(LedSet_t set) {
    return mq_send_msg_t<LedSet_t>(ioServiceName,"setLed",set );
}

z_status ioLedFlash(LedFlash_t flash) {
    return mq_send_msg_t<LedFlash_t>(ioServiceName,"ledFlash",flash );
}


#endif

