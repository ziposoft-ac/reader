//
// Created by ac on 7/17/26.
//
#include "RfidService.h"
#include "../api/IoApi.h"


struct Counter {
    U64 count;

};


z_status RfidService::initialize() {
    if (!globalLock())
        return zs_already_open;

    if (_simulate) {
        ZDBG("Simulate is on\n");
        _reader= &simulator;

    }
    else
        _reader= &cfmu804;

    ioLedSet({LedRed,true  });
    ioLedSet({LedGreen,false  });
    ws.start();
    mq.run("/rfidservice");
    _reader->open();
    _visits.initialize();
    //ZDBGS.add_stdout();

    reg_func("stopstart_raw",&RfidService::post_start_stop_raw);
    reg_func("stopstart_visits",&RfidService::post_start_stop_visits);
    reg_func("status",&RfidService::get_status);
    _getRawReadsCd=reg_func("reads_raw",&RfidService::getRawReads);



    getRfidReader().register_cb_queue_empty(this,[this]() {
        //ZDBG("rfid queue empty\n");
        if (_getRawReadsCd)
            _getRawReadsCd->complete_req_all();
        return true;
    });


    ws.register_consumer(this);
    mq.register_consumer(this);
    return zs_ok;
};

ZMETA(RfidService) {
    ZBASE(Service);
    ZOBJ_X(cfmu804,"rfid",ZFF_PROP_DEF,"cf-804 module");
    ZOBJ(ws);
    ZOBJ(simulator);
    ZPROP(_simulate);
    ZOBJ(_visits);
    ZOBJ(apiTest);

    ZACT(simulate_on);
    ZACT(simulate_off);


};
ROOT_SERVICE(RfidService);

RfidReader& getRfidReader() {
    return gRfidService.getRfidReader();
}

