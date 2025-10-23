//
// Created by ac on 11/12/20.
//
#include "processRunner.h"
#include "root.h"
#include <filesystem>
//#include <openssl/ossl_typ.h>

ZMETA_DEF(ProcessRunner);


ProcessRunner::ProcessRunner()
{


}
int  ProcessRunner::timer_callback(void*)
{
    std::unique_lock<std::mutex> mlock(_mutex);
    auto now=z_time::get_now();

    for (auto i: _tags) {
        auto tag=i.second;
        if (tag->_last_time_seen ==0)
            continue;

        U64 missing_time=now.get_t() - tag->_last_time_seen;
        if (tag->_counted) {
            if (missing_time > _min_split_time*1000) {
                tag->_last_time_seen=0;
                tag->_counted=false;
                zout << "removing old:" << i.first <<"\n";

            }
            continue;

        }
        else {
            if (missing_time>_missing_count_time) {
                tag->_counted=true;
                zout << "COUNTED because it was missing:" << i.first <<":"<<missing_time<<"\n";
            root.gpio.buzzer.pushBeeps({{1400,100}});

            }
        }


    }

    return 100;
}

z_status ProcessRunner::start()
{

    ZLOG("ProcessRunner start:\n");

    if(_open)
        return zs_ok;


    root.getReader().register_consumer(this);

    if(!_timer) {
        _timer=root.timerService.create_timer_t(this,&ProcessRunner::timer_callback,0,100    );

    }

    _open=true;
    ZLOG("ProcessRunner opened\n");

    return zs_ok;
}
z_status ProcessRunner::stop()
{
    _timer->stop();

    root.getReader().remove_consumer(this);

    if(!_open)
        return zs_ok;
    _open=false;
    return zs_ok;
}



bool ProcessRunner::callbackQueueEmpty()
{
    return true;
}
bool ProcessRunner::callbackRead(RfidRead* read)
{

    std::unique_lock<std::mutex> mlock(_mutex);

    U64 last_notify=0;
    try {

        z_string epc;
        read->getEpcString(epc);
        if(_print_reads)
        {
            zout << read->get_ms_epoch() << ":" << read->_antNum << ":" << read->_rssi << ":" << epc <<"\n";
            zout.flush();
        }
        U64 timestamp = read->get_ms_epoch();

        do {
            RfidTag *pTag = _tags.getobj(epc);

            if (!pTag) {
                pTag = z_new RfidTag();
                _tags.add(epc, pTag);

            }
            if (pTag->_last_time_seen==0) {
                pTag->_last_rssi=read->_rssi;
                pTag->_last_time_seen = timestamp;
                break;

            }


            if (pTag->_counted) {
                //ZLOG("Ignoring counted tag %s\n",epc.c_str());
                pTag->_last_time_seen = timestamp;

                break; //ignore

            }
            U64 missing_time_ms = timestamp - pTag->_last_time_seen;

            if (missing_time_ms>_missing_count_time) {
                //let timer handle this
                //ZLOG("Ignoring old tag %s %d\n",epc.c_str(),missing_time_ms);

                break;

            }
            if ( pTag->_last_rssi< read->_rssi) {
                // getting closer
                //ZLOG("tag  getting closer %s\n",epc.c_str());

                pTag->_last_rssi=read->_rssi;
                pTag->_last_time_seen = timestamp;
                break;
            }
            pTag->_counted=true;
            //root.gpio.buzzer.beepDiminishing({2000,500});
            root.gpio.buzzer.pushBeeps({{1400,100}});

            zout << "COUNTED moving away:" << epc <<"\n";
            zout.flush();
        }while (0);




    }
    catch(...)
    {
        Z_THROW_MSG(zs_internal_error,"Exception writing to read log file");
    }



    return true;
}


