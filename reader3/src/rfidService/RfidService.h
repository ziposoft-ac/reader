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
#include "../api/IoApi.h"

#include "global.h"
#include "rfid/simRace.h"
#include "rfid/VisitProcess.h"
#define WAIT_FOR_NEW_READS_TIMEOUT 29000

class RfidService : public  Service,public CommandHandler{


    RfidReader* _reader;
public:
    bool _simulate=true;
    Cfmu804 cfmu804;
    WebServer ws;
    //MqServer mq;
    //MqFeed mq;
    //MqFeed feed;

    VisitProcess _visits;
    IoApiClient apiTest;
    RfidSimulator simulator;
    RfidSimRace simRace;
    RfidReader& getRfidReader() {
        return *_reader;
    }

    z_status simulate_on() {
        _reader->close();

        _reader=&simRace;
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
    CommandDelayed* _getGetVisitsCd;


    z_status initialize() override;

    z_status shutdown() override{
        //command_handler_stop();
        _visits.stop();
        _visits.shutdown();
        ws.remove_consumer(this);

        ws.stop();

        /*
        mq.remove_consumer(this);
        mq.stop();
        mq.shutdown();
        */
        simulator.close();
        simRace.close();
        cfmu804.close();
        Service::shutdown();

        return zs_ok;
    };
    int getRawReads(http_request req,z_string_map &vars,z_json_obj &jin, z_json_stream &jout);
    int getVisits(http_request req,z_string_map &vars,z_json_obj &jin, z_json_stream &jout);

    int post_start_stop_raw(z_json_obj& o,z_json_stream& jout);
    int post_start_stop_visits(z_json_obj& o,z_json_stream& jout);
    int json_status(z_json_stream& jout);
    int get_status(z_string_map& params,z_json_stream& jout);
    int get_raw_reads(z_string_map& params,z_json_stream& jout);
    int post_config(z_json_obj& o,z_json_stream& jout);
    void callbackVisitNotify();
    U64 _counter=0;

};

// todo - get rid of this
extern RfidService gRfidService;
#endif //ZIPOSOFT_RFIDSERVICE_H
