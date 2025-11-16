#include "pch.h"


#include "cfmu804.h"

#include <numeric>



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

    z_status status=send_command(0xf,0,0,2000,
                                 &return_code,data,133, &len);


    if(status)
        return NULL;

    U8 ant=data[0];
    U8 epc_len=data[2];

    U8* epc=(U8*)(data+3);
    U8 rssi=data[epc_len+3];
    ZOUT("rssi=%d epclen=%d",rssi,epc_len);
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
    z_status status=send_command(0x91, (U8*)&param,5,2000,0,&loss, 1);
    if (status)
        return 0;
    return loss;
}
z_status Cfmu804::badcmd()
{
    return send_command(0x99, 0,0,2000,0,0, 0);;

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

/*
cfmu804>info_get
starting rx_thread
response:11 00 21 00 01 14 75 02 31 80 1e 00 01 01 00 01 d5 e0
data:01 14 75 02 31 80 1e 00 01 01 00 01
Version=1401
Type=75
Tr_Type=2
dmaxfre=31
dminfre=80
Power=30
CheckAnt=1
Ant=1
Scntm=0
US Band #1: Min=902.750 Max=927.250
*/

z_status Cfmu804::_info_get(reader_info_t* p_info)
{
    z_status status=send_command(0x21, 0,0,2000,0,p_info, sizeof(reader_info_t));
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
    if (send_command(0x04, (U8*)&buff,epc.get_len()+5,2000, &code)==zs_ok) {

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
        ZOUT("Version=%x\n", info.Version);
        ZOUT("Type=%x\n", info.Type);
        ZOUT("Tr_Type=%x\n", info.Tr_Type);
        ZOUT("dmaxfre=%x\n", info.dmaxfre);
        ZOUT("dminfre=%x\n", info.dminfre);
        ZOUT("Power=%d\n", info.Power);
        ZOUT("CheckAnt=%d\n", info.CheckAnt);
        ZOUT("Ant=%d\n", info.Ant);
        ZOUT("Scntm=%d\n", info.Scntm);

        if (((info.dmaxfre&0xc0) ==0 )&&((info.dminfre&0xc0) ==0x80 )) {
            ZOUT("US Band #1: Min=%3.3lf Max=%3.3lf\n",
            getFreqBand(FREQ_BAND_US1,info.dminfre&0x3f),
            getFreqBand(FREQ_BAND_US1,info.dmaxfre&0x3f)
                );

        }
        else
        if (((info.dmaxfre&0xc0) ==0xc0 )&&((info.dminfre&0xc0) ==0x0 )) {
            ZOUT("US Band #3: Min=%3.3lf Max=%3.3lf\n",
            getFreqBand(FREQ_BAND_US3,info.dminfre&0x3f),
            getFreqBand(FREQ_BAND_US3,info.dmaxfre&0x3f)
                );
        }else {
            ZOUT("ERROR! Unknown band");
        }

        return zs_ok;

    }
    printf("error reading info\n");

    return zs_ok;

}
z_status Cfmu804::_readmode_set(working_mode_t& mode)
{

    return  send_command(0x75, ((U8*)&mode)+1,5,2000,0, 0);

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

    z_status status=send_command(0x77, 0,0,2000,0,&mode, sizeof(working_mode_t));




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
struct inv_params2_t
{
    U8 QValue;
    U8 Session;
    U8 target;
    U8 ant;
    U8 scan;

}  __attribute__((__packed__)) ;
z_status  Cfmu804::inventory()
{
    inv_params_t params=
            {
                0x85,0,1,0,0,0,0,0,0x80,30
            };
    inv_params2_t params2=
    {
        4,0xfd,0,0x80,0x32
    };
    //09 00 01 04 fd 00 80 32 4d 9b
    char p2[]={ 0x05,00};
    const int data_len=100;
    char data[data_len];
    int retlen=0;

    //return send_command(1, (U8*)&params,sizeof(params),0,&data,data_len,&retlen);
    z_status status= send_command(1, (U8*)&params2,sizeof(params2),2000,0,&data,data_len, &retlen);
    zout<<"STATUS:"<<status<<"\n";

    //if (status)
    return status;

}
z_status  Cfmu804::inv(int session,int target)
{
    inv_params2_t params2=
    {
        4,(U8)session,(U8)target,0x80,10
    };
    //09 00 01 04 fd 00 80 32 4d 9b

    //return send_command(1, (U8*)&params,sizeof(params),0,&data,data_len,&retlen);
    z_status status= send_command(1, (U8*)&params2,sizeof(params2),3000);
    zout<<"STATUS:"<<status<<"\n";

    //if (status)
    return status;

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

    return send_command(2, (U8*)&params,sizeof(params),2000,0,&data,data_len, &retlen);

}
int  Cfmu804::get_temperature_cmd()
{
    struct {
        U8 sign=0;
        U8 temp=0;
    } data;
    z_status status=send_command(0x92, 0,0,2000,0,&data, sizeof(data));
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
