#include "pch.h"


#include "cfmu804.h"

#include <numeric>

unsigned int uiCrc16Cal(unsigned char const* pucY, unsigned char ucX);

void Response::dump()
{
    U8 status = _frame->buff.frame.status;
    if (status>1)
    {
        const cfmu_error_t* err=get_cfmu_error(status);
        zout << err->error << '\n';
        zout << err->desc << '\n';
    }
    else {
        int i;


        printf("response:");
        for (i = 0; i < _frame->_len; i++)
            printf("%02x ", _frame->buff.raw[i]);
        printf("\n");
        int data_len= _frame->_len - 6;
        if(data_len)
        {
            printf("data:");
            for (i = 0; i < _frame->_len - 6; i++)
                printf("%02x ", _frame->buff.frame.data[i]);
            printf("\n");
        }

    }


}


void Response::dump_read()
{
    int i;
    ResponseFrame::framebuff_t &buff=_frame->buff;
    U8 ant=buff.frame.data[0];
    U8 epclen=buff.frame.data[1];
    U8* epc=(U8*)(buff.frame.data+2);
    U8 rssi=buff.frame.data[epclen+2];
    printf("READ:  ANT#%d RSSI=%d EPC=",ant,rssi);
    for (i = 0; i < epclen; i++)
        printf("%02x", buff.frame.data[i+2]);
    printf("\n");
}
int ResponseFrame::read(serial::Serial& port,bool debug)
{
    size_t bytes_read=0;
    size_t bytes_available = port.available();
    if (!bytes_available)
    {
        zout << "No serial data available?.\n";
        return 0;
    }

    while (bytes_available)
    {
        //change this to check bytes available here!
        //what the fuck is happening with timer3?!
        //count bytes read1
        int bytes_to_read = get_msg_len()-_len;
        if (bytes_to_read < 1)
            Z_THROW("frame is longer than expected?");
        if (bytes_to_read > bytes_available)
            bytes_to_read = bytes_available;

        int len_read = port.read(get_read_ptr(), bytes_to_read);
        bytes_read+=len_read;
        if(debug)
        {
            zout <<"\nRX:";
            int i;
            for(i=0;i<len_read;i++)
            {
                U8* buff=get_read_ptr();
                zout.format_append(" %02x",buff[i]);

            }
            zout<<"\n";
        }
        //bytes_available -= len_read;
        _len += len_read;
        if (is_complete()) {
            break;

        }
        bytes_available = port.available();
    }
    return bytes_read;
}

void ResponseFrame::dump()
{
    if (!is_complete())
        return;
    U8 status = buff.frame.status;
    int i;
    if (status>1)
    {
        const cfmu_error_t* err=get_cfmu_error(status);
        zout << err->error << '\n';
        zout << err->desc << '\n';
    }
    else
    {
        printf("response:");
        for (i = 0; i < _len ; i++)
            printf("%02x ", buff.raw[i]);
        printf("\n");
        printf("data:");
        for (i = 0; i < _len - 6; i++)
            printf("%02x ", buff.frame.data[i]);

    }
    printf("\n");
    zout.flush();
}

z_status Cfmu804::_hw_close()
{
    if(isReading())
       _read_stop();

    _rx_thread_stop();
    _port.close();
    return zs_ok;
}

z_status Cfmu804::send_command(U8 code,U8* tx_data,int tx_len,cfmu_status_code *p_return_code,void* rx_data,int max_rx_len,int* var_len)
{
    z_status status=zs_timeout;
    std::unique_lock<std::mutex> mlock(_mutex_command);
    if(open()!=zs_ok)
        return zs_not_open;
    auto prev=_responses.pop(code); // remove previous response if there
    delete prev;

    __cmd_tx(code,tx_data,tx_len);
    Response* resp= _responses.get_wait_for(code,2,true);
    cfmu_status_code return_code=0xF0;//Quit waiting for response
    if(resp)
    {
        return_code=resp->get_status();
        if(return_code<2)
        {
            status=zs_io_error;
            int len=resp->get_data_len();
            if(var_len)
            {
                *var_len=len;
            }
            if(rx_data)
            {
                if(max_rx_len>=len)
                {
                    memcpy(rx_data,resp->get_data(),len);
                }
                else {
                    return_code= 0xF1;//Return length error

                }
            }
        }
        delete resp;
    }
    if(return_code>1)
    {
        z_string err_msg;
        print_cfmu_error(err_msg,return_code);
        status=zs_failed_on_device;

        //Z_THROW_MSG(zs_io_error,err_msg);
    }
    else {
        status=zs_ok;

    }
    if (p_return_code)
        *p_return_code=return_code;

    return status;

}
z_status Cfmu804::_info_get(reader_info_t* p_info)
{
    z_status status=send_command(0x21, 0,0,0,p_info,sizeof(reader_info_t));
    if (status==zs_ok)
        _power=p_info->Power;




    return status;

}
struct write_epc_t
{
    U8 len;
    U32 pwd;
    U8 epc[Epc::_max_len];
}__attribute__((__packed__)) ;

cfmu_status_code Cfmu804::program_epc(Epc& epc)
{
    open();

    write_epc_t buff;
    buff.pwd=0;
    memcpy(buff.epc,epc.get_data(),epc.get_len());
    buff.len=epc.get_len()/2; // Length is in 16bit WORDS. Dont ask.

    cfmu_status_code code;
    if (send_command(0x04, (U8*)&buff,epc.get_len()+5,&code)==zs_ok) {

    }
    else {
        Z_ERROR_MSG(zs_io_error,"error writing epc");
    }




    return code;
}
z_status Cfmu804::write_bcd(int number)
{
    Epc epc;
    epc.set_bcd_from_int(number);

    cfmu_status_code code=program_epc(epc);
    if(code)
        return zs_io_error;
    return zs_ok;
}

#define FREQ_BAND_US1 902.75
#define FREQ_BAND_US3 902
double getFreqBand(double base,int n) {
    return base+n*0.5;
}

z_status Cfmu804::info_get()
{
    reader_info_t info;
    if (_info_get(&info)==zs_ok) {
        ZLOG("Version=%x\n", info.Version);
        ZLOG("Type=%x\n", info.Type);
        ZLOG("Tr_Type=%x\n", info.Tr_Type);
        ZLOG("dmaxfre=%x\n", info.dmaxfre);
        ZLOG("dminfre=%x\n", info.dminfre);
        ZLOG("Power=%d\n", info.Power);
        ZLOG("CheckAnt=%d\n", info.CheckAnt);
        ZLOG("Ant=%d\n", info.Ant);
        ZLOG("Scntm=%d\n", info.Scntm);

        if (((info.dmaxfre&0xc0) ==0 )&&((info.dminfre&0xc0) ==0x80 )) {
            ZLOG("US Band #1: Min=%3.3lf Max=%3.3lf\n",
            getFreqBand(FREQ_BAND_US1,info.dminfre&0x3f),
            getFreqBand(FREQ_BAND_US1,info.dmaxfre&0x3f)
                );

        }
        else
        if (((info.dmaxfre&0xc0) ==0xc0 )&&((info.dminfre&0xc0) ==0x0 )) {
            ZLOG("US Band #3: Min=%3.3lf Max=%3.3lf\n",
            getFreqBand(FREQ_BAND_US3,info.dminfre&0x3f),
            getFreqBand(FREQ_BAND_US3,info.dmaxfre&0x3f)
                );
        }else {
            ZLOG("ERROR! Unknown band");
        }

        return zs_ok;

    }
    printf("error reading info\n");

    return zs_ok;

}
z_status Cfmu804::_readmode_set(working_mode_t& mode)
{

    return  send_command(0x75, ((U8*)&mode)+1,5,0,0);

}
z_status Cfmu804::readmode_set()
{
    working_mode_t mode;
    z_status status=_readmode_get(mode);
    if(status)
        return status;
    mode.Session=_session;
    mode.QValue=_qvalue;
    mode.FilterTime=_filter_time;
    mode.MaskMem=_maskMem;
    mode.TagProtocol=_tagProtocol;
    mode.ReadPauseTime=_pause_read_time;


    status=_readmode_set(mode);
    if(status)
        return status;
    printf("set status=%x\n", status);

    printf("TagProtocol=%x\n", mode.TagProtocol);
    printf("ReadPauseTime=%x\n", mode.ReadPauseTime);
    printf("FilterTime=%x\n", mode.FilterTime);
    printf("QValue=%x\n", mode.QValue);
    printf("Session=%d\n", mode.Session);
    printf("MaskMem=%d\n", mode.MaskMem);
    printf("MaskAdr=%d\n", mode.MaskAdr);
    printf("MaskLen=%d\n", mode.MaskLen);

    return zs_ok;

}
z_status Cfmu804::_readmode_get(working_mode_t& mode)
{

    z_status status=send_command(0x77, 0,0,0,&mode,sizeof(working_mode_t));




    return status;

}
z_status Cfmu804::readmode_get()
{
    working_mode_t mode;
    z_status status=_readmode_get(mode);
    if(status)
    {
        return status;
    }
    _session=mode.Session;
    _qvalue=mode.QValue;
    _filter_time=mode.FilterTime;
    _maskMem=mode.MaskMem;
    _tagProtocol=mode.TagProtocol;
    _pause_read_time=mode.ReadPauseTime;

    printf("ReadMode=%x\n", mode.ReadMode);
    printf("TagProtocol=%x\n", mode.TagProtocol);
    printf("ReadPauseTime=%x\n", mode.ReadPauseTime);
    printf("FilterTime=%x\n", mode.FilterTime);
    printf("QValue=%x\n", mode.QValue);
    printf("Session=%d\n", mode.Session);
    printf("MaskMem=%d\n", mode.MaskMem);
    printf("MaskAdr=%d\n", mode.MaskAdr);
    printf("MaskLen=%d\n", mode.MaskLen);
    return zs_ok;
}

struct inv_params_t
{
    U8 QValue;
    U8 Session;
    U8 MaskMem;
    U16 MaskAdr;
    U8 MaskLen;
    U8 AdrTID;
    U8 LenTID;
    U8 target;
    U8 ant;
    U8 scan;

}  __attribute__((__packed__)) ;

z_status Cfmu804::config_read() {

    readmode_get();
    info_get();
    write_power_get();
    antCheck();
    return zs_ok;

}

z_status  Cfmu804::inventory()
{
    inv_params_t params=
            {
                0x85,0,1,0,0,0,0,0,0x80,30
            };
    const int data_len=100;
    char data[data_len];
    int retlen=0;

    return send_command(1, (U8*)&params,sizeof(params),0,&data,data_len,&retlen);

}
struct params_read_chip_t
{
    U8 epc_len;
    U8 epc[16];
    U8 mem;
    U8 wordptr;
    U8 num;
    U32 pwd;



}  __attribute__((__packed__)) ;
z_status  Cfmu804::exp_data_read()
{
//    params_read_chip_t params={2,0,0,0,4,3,0,20,0   };
    params_read_chip_t params={8,
                               0x98,0x46 ,0xa2 ,0x66 ,0x53 ,0xc1 ,0x02 ,0xee ,0x9f ,0x78 ,0xb9 ,0x98 ,0x9f ,0xb4 ,0xcc ,0x11,
                               3,0,6,0   };


    //98 46 a2 66 53 c1 02 ee 9f 78 b9 98 9f b4 cc 11


    const int data_len=100;
    char data[data_len];
    int retlen=0;

    return send_command(2, (U8*)&params,sizeof(params),0,&data,data_len,&retlen);

}
int  Cfmu804::get_temperature_cmd()
{
    struct {
        U8 sign=0;
        U8 temp=0;
    } data;
    z_status status=send_command(0x92, 0,0,0,&data,sizeof(data));
    if (status==zs_ok) {
        int temperature=data.temp;
        if(data.sign==0)
            temperature=-temperature;
        return temperature;

    }


    return -1;
}
z_status  Cfmu804::inventory_single()
{
    open();
    RfidRead* r=read_single();
    if(!r)
        return zs_io_error;
    z_string s;
    r->getJson(s);
    zout<<s<<'\n';
    r->_epc.getHexString(s);
    zout<<"hex:"<<s<<"\n";
    ;
    zout<<"bib:"<<r->_epc.get_bib_number()<<"\n";
    delete r;
    return zs_ok;

}
z_status  Cfmu804::program(int number,bool overwrite)
{
    bool programmed=false;
    RfidRead* r=program_bcd(number,overwrite,programmed);
    if(!r)
    {
        zout<<"read failed\n";
        return zs_io_error;

    }
    int num=r->_epc.get_bib_number();
    z_string s;
    r->_epc.getHexString(s);
    zout<<"epc:" << s<< "\n";
    zout<<"read:"<<num<<" programmed:"<< programmed<<'\n';

    delete r;
    return zs_ok;

}


RfidRead* Cfmu804::program_bcd(int number,bool overwrite,bool& programmed)
{
    open();
    RfidRead* r=read_single();
    if(!r)
    {
        Z_ERROR_MSG(zs_not_found,"no bib read");
        return 0;
    }
    z_string epc_string;
    r->_epc.getHexString(epc_string);
    programmed=false;
    if(r->_epc.get_bib_number() == number)
    {
        Z_ERROR_MSG(zs_already_exists,"bib already programmed with same number= %s",epc_string.c_str());
        return r;
    }

    int len=r->_epc.get_len();
    if((len<5) && !overwrite)
    {
        Z_ERROR_MSG(zs_already_exists,"bib already programmed with different number= %s,%d",epc_string.c_str(),len);
        return r;
    }

    z_status  status=write_bcd(number);
    if(status)
    {
        Z_ERROR_MSG(zs_io_error,"error writing epc");
        return r;
    }

    delete r;
    r=read_single();
    if(r->_epc.get_bib_number() == number)
    {
        programmed=true;
    }
    return r;
}



RfidRead*  Cfmu804::read_single()
{
    open();
    if(isReading())
    {
        Z_ERROR_MSG(zs_already_open,"already reading");
        return NULL;
    }
    if(!_open)    {
        Z_ERROR_MSG(zs_not_open,"not open");
        return NULL;
    }
    U8 data[134];
    int len=0;
    cfmu_status_code return_code=0xF0;//Quit waiting for response

    z_status status=send_command(0xf,0,0,&return_code,
        data,133,&len);


    if(status)
        return NULL;

    U8 ant=data[0];
    U8 epc_len=data[2];

    U8* epc=(U8*)(data+3);
    U8 rssi=data[epc_len+3];
    ZLOG("rssi=%d epclen=%d",rssi,epc_len);
    RfidRead* r=new RfidRead(1,data[0],rssi,epc,epc_len,z_time::get_now());


    return r;
}
z_status Cfmu804::freq_set(U8 low,U8 hi)
{
    struct param_t{
        U8 max;
        U8 min;
    } p;
    if ((hi>3)||(hi<low))
        return Z_ERROR_MSG(zs_bad_parameter,"freq quad out of range: low=%d hi=%d",low,hi);
    const U8 range=50;

    p.min=range/4*low+0x80;
    p.max=range/4*(hi+1)-1;





    return send_command(0x22,(U8*)&p,2);
}
U8 Cfmu804::get_ant_loss(int antnum)
{
    struct param_t{
        U32 test_freq;
        U8 ant;
    } param;
    param.test_freq = bswap_32(915000);
    param.ant = antnum;
    U8 loss;
    z_status status=send_command(0x91, (U8*)&param,5,0,&loss,1);
    if (status)
        return 0;
    return loss;
}
z_status Cfmu804::badcmd()
{
    return send_command(0x99, 0,0,0,0,0);;

}
z_status Cfmu804::measure_ant(int antnum)
{
    U8 loss=get_ant_loss(antnum);
    zout.format_append("ANT#%d loss=%d",antnum,loss);
    return zs_ok;

}
z_status Cfmu804::antCheck()
{
    z_status status=open();
    if(status)
        return status;

    int i;
    U8 loss;
    _antenna_detected=0;
    for(i=0;i<4;i++)
    {
        zout<< "ANT#" << i+1;
        loss=get_ant_loss(i);
        if(loss>3)
        {
            _antenna_detected|=(1<<i);
            zout<< "Connected:"<<loss<<"\n";
        } else
            zout<< "Not Connected:"<<loss<<"\n";

    }
    zout<< "_antenna_detected:"<<_antenna_detected<<"\n";

    return zs_ok;
}
z_status Cfmu804::config_write(        )
    {
    z_status status=open();
    if(status)
        return status;


    working_mode_t mode;
    _readmode_get(mode);
    if(mode.ReadMode)
    {
        _read_stop();
    }
    power_set(_power);
    mode.ReadPauseTime=_pause_read_time;
    mode.QValue=_qvalue;
    mode.Session=_session;
    mode.FilterTime=_filter_time;
    _readmode_set(mode);

    antCheck();
    _antenna_config=_antenna_mask&_antenna_detected;
    zout<< "_antenna_config:"<<_antenna_config<<"\n";

    ant_cfg_set(_antenna_config);
    if(_antenna_config)
    {
        reader_info_t info;
        if (_info_get(&info) == zs_ok) {
            _antenna_config=info.Ant;
        }
    }

    return zs_ok;
}

z_status Cfmu804::_hw_open()
{
    try
    {
        if (_port.isOpen())
            return zs_ok;
        _port.setPort(_port_name);
        _port.setBaudrate(_port_speed);
        _port.setFlowcontrol(serial::flowcontrol_none);
        _port.setParity(serial::parity_none);
        _port.setStopbits(serial::stopbits_one);
        _port.open();
        _port.flushInput();
        serial::Timeout to = serial::Timeout::simpleTimeout(100);
        _port.setTimeout(to);

        _rx_thread_start();



    }
    //TODO check if we are currently reading!!
    catch (std::system_error & e) {
        zout << "System error: " << e.what() << '\n';
        return zs_could_not_open_file;
    }
    catch (std::exception & e) {
        zout << "Unhandled Exception: " << e.what() << '\n';
        return zs_could_not_open_file;
    }
    return zs_ok;
}
bool  Cfmu804::_rx_frame(ResponseFrame* frame )
{
    try
    {
        size_t bytes_read = 0;
        int resp_idx = 0;
        z_time t;
        while (!_rx_quit)
        {
            t.set_now();
            if (!_port.waitReadable()) {
                //ZDBG("serial timeout: %d\n",t.get_elapsed_ms());
                continue;

            }
            size_t bytes_available = _port.available();
            if (!bytes_available)
            {
                zout << "No serial data available?.\n";
                continue;
            }
            //ZDBG("serial data available: %d ms, %d bytes\n",t.get_elapsed_ms(),bytes_available);


            _total_bytes_read+=frame->read(_port,_debug_rx);
            if (frame->is_complete())
            {
                return true;
            }
        }
    }
    catch (std::exception & e) {
        zout << "Unhandled Exception: " << e.what() << '\n';
        return false;
    }

    return false;

}
int clz(int N) {
    return N ? 32 - __builtin_clz(N) : 0;
}
void  Cfmu804::rx_thread()
{

    zout << "starting rx_thread\n";

    ResponseFrame *p_frame = new ResponseFrame();
    try
    {
        do {
            if (!_rx_frame(p_frame))
            {
                break; // Probably are quiting
            }
            Response* cmdResponse=0;
            ResponseFrame::framebuff_t &buff=p_frame->buff;
            U8 cmd_code=p_frame->get_cmd();
            switch(cmd_code)
            {
                case 0xee:
                {
                    U8 ant=buff.frame.data[0];
                    U8 epclen=buff.frame.data[1];
                    U8* epc=(U8*)(buff.frame.data+2);
                    U8 rssi=buff.frame.data[epclen+2];

                    queueRead(ant,rssi,epc,epclen,z_time::get_now());
                    p_frame->reset();
                    break;

                }
                default:
                {
                    cmdResponse = new Response(p_frame);
                    p_frame = new ResponseFrame();
                    break;
                }
            }
            if(cmdResponse)
            {
                if(_debug_rx)
                    cmdResponse->dump();
                _responses.replace(cmd_code,cmdResponse);

               // _cmd_responses.push_back(cmdResponse);
            }
        } while(!_rx_quit);
    }
    catch (std::exception & e) {
        zout << "Unhandled Exception: " << e.what() << '\n';

    }
    zout << "exiting rx_thread\n";
    if (p_frame)
        delete p_frame;
    return;

}
z_status Cfmu804::_rx_thread_start()
{
    if (_rx_started)
        return zs_already_open;
    _rx_quit = false;

    _rx_thread_handle = std::thread(&Cfmu804::rx_thread, this);
    _rx_started = true;

    return zs_ok;

}
z_status Cfmu804::_rx_thread_stop()
{

    _rx_quit = true;
    if (_rx_thread_handle.joinable())
        _rx_thread_handle.join();
    _rx_started = false;


    return zs_ok;

}
z_status Cfmu804::send_cmd_byte(U8 cmd_code,U8 data)
{
    return send_command(cmd_code,&data,1);

}
z_status Cfmu804::cmd_single_byte_return(U8 cmd_code,U8* pdata)
{
    z_status status=send_command(cmd_code, 0,0,0,pdata,1);
    return status;

}
z_status Cfmu804::__cmd_tx(U8 cmd_code,U8* data,U8 data_len)
{

    open();
    //rx_start();


    int total_len = data_len + 5;
    _buff_tx[0] = data_len + 4;
    _buff_tx[1] = 0xff;
    _buff_tx[2] = cmd_code;
    memcpy(&_buff_tx[3], data, data_len);

    int crc = uiCrc16Cal(_buff_tx, data_len+3);
    _buff_tx[3+ data_len] = crc & 0xff;
    _buff_tx[4+ data_len] = crc >> 8;
    int i;

    if(_debug_tx)
    {
        printf("sending:");
        for (i = 0; i< total_len ; i++)
            printf("%02x ", _buff_tx[i]);
        printf("\n");

    }

    try
    {
        _port.write(_buff_tx, total_len);
        _port.flushOutput();


    }
    catch (std::exception & e) {
        zout << "Unhandled Exception: " << e.what() << '\n';
    }
    _rx_quit = false;


    return zs_ok;

}

ZMETA_DEF(Cfmu804);



