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
    if (debug) {
        ZDBG("\nRX:");
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
            int i;
            for(i=0;i<len_read;i++)
            {
                U8* buff=get_read_ptr();
                ZDBG(" %02x",buff[i]);

            }
        }
        //bytes_available -= len_read;
        _len += len_read;
        if (is_complete()) {
            break;

        }
        bytes_available = port.available();
    }
    if (debug) {
        ZDBG("\n");
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

z_status Cfmu804::send_command(U8 code, U8* tx_data,
                               int tx_len,
                               int wait_time_ms,
                               cfmu_status_code *p_return_code,
                               void* rx_data,
                               int max_rx_len, int* ret_len)
{
    z_status status=zs_timeout;
    if(open()!=zs_ok)
        return zs_not_open;
    std::unique_lock<std::mutex> mlock(_mutex_command);

    auto prev=_responses.pop(code); // remove previous response if there
    delete prev;

    __cmd_tx(code,tx_data,tx_len);
    Response* resp= _responses.get_wait_for(code,1,true);
    if (!resp) {
        if (p_return_code)
            *p_return_code=0xF0;
        return Z_ERROR_MSG(zs_timeout,"command timeout waiting for reponse");

    }
    cfmu_status_code return_code=0xF0;//Quit waiting for response
    return_code=resp->get_status();
    if(return_code<2)
    {
        status=zs_io_error;
        int len=resp->get_data_len();
        if(ret_len)
        {
            *ret_len=len;
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
    if(return_code>3)
    {
        //z_string err_msg;
        print_cfmu_error(get_error_logger(),return_code);
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

z_status Cfmu804::_hw_init() {


    //IS IT CURRENTLY READING??? STOP IT?
    //GET HW VERSION/CONTEXT_PORT_NO_LISTEN_SERVER


    //TODO check if we are currently reading!!
    _read_stop();
    reader_info_t info;
    if (_info_get(&info)==zs_ok) {
        _model=(Model)info.Type;
        switch (_model) {
            case model_e714:
                _num_ports=4;
                ZLOG("E714 4 port reader \n");

                break;
            case model_e718:
                _num_ports=8;
                ZLOG("E718 8 port reader \n");

                break;
            default:
                _num_ports=4;
                Z_ERROR_LOG("Unknown reader type=%x", _model);

                break;

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

        if (_rx_thread_start()) {


            return zs_io_error;

        }
        //DO NOT CALL ANY COMMANDS HERE
        //will cause a recursive open


    }

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


            _total_bytes_read+=frame->read(_port,_debug_rx_bytes);
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
            #ifdef  ENABLE_PHASE
            U16 phase1=0;
            U16 phase2=0;
            #endif
            ResponseFrame::framebuff_t &buff=p_frame->buff;
            U8 cmd_code=p_frame->get_cmd();
            U8 status=p_frame->get_status();
            if (_debug_rx_frames)
                p_frame->dump();
            switch(cmd_code)
            {
                case 0xee:
                {

                    U8 ant=buff.frame.data[0];

                    U8 epclen=buff.frame.data[1];
                    if (epclen&0x80) {
                        //_ERROR_MSG(zs_internal_error,"DO");
                    }
                    bool phase=epclen&0x40;

                    epclen&=0x3f;
                    U8* epc=(U8*)(buff.frame.data+2);
                    U8 rssi=buff.frame.data[epclen+2];
#ifdef ENABLE_PHASE
                    if (phase) {
                        phase1=*(U16*)(buff.frame.data+epclen+3);
                        phase2=*(U16*)(buff.frame.data+epclen+5);
                        //ZDBG_HEX(epc,epclen);
                       // ZDBGS << rssi << '\t' << phase1 << '\t' << phase2 <<'\n';
                       // ZDBGS.format_append("%d %d %*s\n",phase1,phase2,epclen,epc);

                        //ignore phase for now
                    }
#endif
                    queueRead(ant,rssi,epc,epclen,z_time::get_now()
                    #ifdef  ENABLE_PHASE
                        ,phase1,phase2
                    #endif
                        );
                    p_frame->reset();
                    break;

                }
                case 0x01:
                {
                    if (status==0x03) {
                        //  01 01 04 00 00 00 16 4c 21 a8
                        // ant 01, num 01, epc size 04, epc 00000016, rssi 4c, crc
                        U8 ant=buff.frame.data[0];
                        U8 num_epc=buff.frame.data[1];
                        if (num_epc!=1)
                            Z_ERROR_MSG(zs_internal_error,"Inventory #EPC>1!");
                        U8 epclen=buff.frame.data[2];
                        U8* epc=(U8*)(buff.frame.data+3);
                        U8 rssi=buff.frame.data[epclen+3];

                        queueRead(ant,rssi,epc,epclen,z_time::get_now()
                        #ifdef  ENABLE_PHASE
                        ,phase1,phase2
                        #endif
                        );
                        p_frame->reset();
                        break;
                    }
                    // FALL THROUGH TO DEFAULT BELOW


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
                if(_debug_response)
                    cmdResponse->dump();
                //ZDBG("adding response: %x\n",cmd_code);
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
    z_status status=send_command(cmd_code, 0,0,2000,0,pdata, 1);
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



