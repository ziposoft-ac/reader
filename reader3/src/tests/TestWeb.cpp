//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"
#include "web/WebServer.h"

#include "global.h"





class TestWeb : public  Service,public CommandHandler{
public:
    WebServer ws;
    MqServer mq;

    TestWeb(){}
    virtual ~TestWeb() {}
    U64 _counter=0;
    U64 _print_stat_interval=10000;
    U64 _max_count=100000;
    U64 _us_start;


    z_status print_stat_count(int count ) {
        U64 us_end= z_time_get_ticks_us();
        U64 diff=us_end-_us_start;
        double calls_per_sec=count/((double)diff/1000000.0);
        printf("test_post  %d iters in %lu us, %lf per call, %lf calls per sec \n",count,diff,(double)diff/(double)count,calls_per_sec);
        return zs_ok;


    }
    z_status test_post(z_json_obj& jin,z_json_stream& jout) {


        int count=jin.get_int("count",0);
        if (count==0) {

            _us_start= z_time_get_ticks_us();
        }

        count++;
        //printf("count=%d\n",count);
        jout.keyval_int("count",count);;
        if (count %_print_stat_interval == 0) print_stat_count(count);
        return zs_ok;
    }
    z_status test_post2(z_json_obj& jin,z_json_stream& jout) {
        int count=jin.get_int("count",0);

        printf("count=%d\n",count);
        jout.keyval_int("count",count);;
        jout.keyval("hey2","what2?");
        return zs_ok;
    }
    z_status initialize() override{

        U64 t1=z_time_get_ticks_us();
        U64 ns1=z_time_get_ticks_ns();
        z_sleep_us(1);

        U64 diff_us=z_time_get_ticks_us()-t1;
        U64 diff_ns=z_time_get_ticks_ns()-ns1;
        printf("time us=%lld\n",diff_us);
        printf("diff_ns=%lld\n",diff_ns);


        reg_fn("test",&TestWeb::test_post);
        reg_fn("@test",&TestWeb::test_post_reply);
        reg_fn("test2",&TestWeb::test_post2);
        ws.register_consumer(this);
        mq.register_consumer(this);
        ws.start();
        mq.run("/test");
        return zs_ok;
    };
    z_status shutdown() override{
        ws.stop();
        mq.stop();
        ws.remove_consumer(this);
        mq.remove_consumer(this);

        return zs_ok;
    };

    z_status test_post_reply(z_json_obj& jin,z_json_stream& jout) {
        I64 count_rx=jin.get_int("count");
        if (count_rx!= (_counter+1)) {
            jin.print() ;
            Z_ERROR_LOG("count does not match %lld != %ld\n", count_rx, _counter+1);
            return zs_data_error;
        }
        _counter=count_rx;
        if (_counter%_print_stat_interval==0l)  print_stat_count(_counter);

        if (_counter<_max_count) {
            sendCount(_counter);
        }
        else
        {
            printf("complete\n");
        }
        return zs_ok;

    }
    z_status sendCount(int count) {

        if (count==0) {
            _counter=0;
            _us_start= z_time_get_ticks_us();
        }

        z_string s;
        z_json_stream js(s);
        js.obj_start();
        js.keyval_int("count",count);

        js.obj_end();
        return mq_send_msg_with_reply("/test","/test","test",count,s,s.length());

    }
    z_status runCounter(int max) {
       mq_send_msg("/test","test");

        return zs_ok;
    }
};
ZMETA(TestWeb) {
    ZBASE(Service);
    ZOBJ(ws);
    ZOBJ(mq   );
    ZPROP(_max_count   );
    ZPROP(_print_stat_interval   );
    ZCMD(sendCount, ZFF_CMD_DEF, "sendCount",
         ZPRM(int, count, 0, "count", ZFF_PARAM)
         );
    ZCMD(runCounter, ZFF_CMD_DEF, "runCounter",
         ZPRM(int, count, 10, "count", ZFF_PARAM)
         );
};

ROOT_SERVICE(TestWeb);
