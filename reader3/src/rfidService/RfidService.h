//
// Created by ac on 7/25/26.
//

#ifndef ZIPOSOFT_RFIDSERVICE_H
#define ZIPOSOFT_RFIDSERVICE_H

#include "pch.h"
#include "main/Service.h"
#include "rfid/rfid.h"
#include "rfid/cfmu804.h"
#include "rfid/simulator.h"
#include "web/WebServer.h"

#include "global.h"
#include "rfid/VisitProcess.h"
#define WAIT_FOR_NEW_READS_TIMEOUT 29000

class RfidService : public  Service,public CommandHandler{


    RfidReader* _reader;
public:
    bool _simulate=true;
    z_status simulate_on() {
        _reader->close();

        _reader=&simulator;
        _simulate=true;
        _reader->open();
        return zs_ok;
    }
    z_status simulate_off() {
        _reader->close();
        _reader=&cfmu804;
        _simulate=false;
        _reader->open();

        return zs_ok;
    }
    RfidService(){}
    virtual ~RfidService() {}

    CommandDelayed* _getRawReadsCd;


    z_status initialize() override;
    z_status run() override {

        return zs_ok;

    }
    z_status shutdown() override{

        ws.remove_consumer(this);
        mq.remove_consumer(this);
        _visits.shutdown();

        ws.stop();
        mq.stop();

        _reader->close();

        return zs_ok;
    };
    int getRawReads(http_request req,z_string_map &vars,z_json_obj &jin, z_json_stream &jout);

    int post_start_stop_raw(z_json_obj& o,z_json_stream& jout);
    int get_status(z_string_map& params,z_json_stream& jout);
    int get_raw_reads(z_string_map& params,z_json_stream& jout);
    int post_config(z_json_obj& o,z_json_stream& jout);

    U64 _counter=0;
    Cfmu804 cfmu804;
    WebServer ws;
    MqServer mq;
    VisitProcess _visits;
    RfidSimulator simulator;
    RfidReader& getRfidReader() {
        return *_reader;
    }
    z_status setLed(int color, int onoff);
};

#endif //ZIPOSOFT_RFIDSERVICE_H
