// cfmu804.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#ifndef CFMU_H
#define CFMU_H
#include "pch.h"
#include "serial.h"
#include "rfid.h"


#ifdef WINDOWS
#define bswap_32  _byteswap_ulong

#else
#include <byteswap.h>

#endif




unsigned int uiCrc16Cal(unsigned char const* pucY, unsigned char ucX);



typedef U8 cfmu_status_code ;

//#pragma pack(1)
struct cfmu_cmd_t
{
    U8 code;
    ctext name;
    ctext desc;

};
struct cfmu_error_t
{
	U8 code;
	ctext error;
	ctext desc;

};
extern const cfmu_error_t cfmu_error_table[];
const cfmu_error_t* get_cfmu_error(cfmu_status_code code);
void print_cfmu_error(z_stream& stream,cfmu_status_code status);

struct tag_read_data
{
    U8 ant;
    U8 len;

};
class CommandFrame
{
public:
	const static int max_len = 255;
	union buff
	{
		struct frame
		{
			U8 len;
			U8 addr;
			U8 cmd;
			U8 data[1];
			
		};
		U8 raw[max_len + 1];
	};
	void response_callback()
	{}
};


class ResponseFrame
{
public:
	ResponseFrame() {}
	virtual ~ResponseFrame() {
	};

	const static int max_len = 255;
	int _len = 0;
	void reset() {
		_len = 0;
	}
	int get_msg_len()
	{
		if(_len)
			return buff.frame.len + 1;
		return 1;
	}
	U8 get_cmd() { return buff.frame.cmd; }
	U8 get_status() { return buff.frame.status; }
	U8* get_data_ptr()
	{
		return &(buff.frame.data[0]);
	}
	union framebuff_t
	{
		struct 
		{
			U8 len;
			U8 addr;
			U8 cmd;
			U8 status;
			U8 data[1];
		} frame;
		U8 raw[max_len + 1];
	} buff;

	bool is_complete()
	{
		if (!_len)
			return false;
		return (_len == get_msg_len());
	}
	int read(serial::Serial& port,bool debug);
	void dump();
	U8* get_read_ptr()
	{
		return &(buff.raw[_len]);
	}
};
class Response
{
public:
    ResponseFrame* _frame=0;
    Response(ResponseFrame* frame) { _frame=frame;  };
	virtual ~Response() {

		if (_frame) {
			delete _frame;

		}
	};
    virtual void dump();
    virtual void dump_read();

    U8* get_data()  { return _frame->buff.frame.data;  }
    cfmu_status_code get_status()  { return _frame->buff.frame.status;  }
    U8 get_data_len()  { return _frame->buff.frame.len-5;  }
};





struct reader_info_t
{
    U16 Version;
    U8 Type;
    U8 Tr_Type;
    U8 dmaxfre;
    U8 dminfre;
    U8 Power;
    U8 Scntm;
    U8 Ant;
    U8 rsvd1;
    U8 rsvd2;
    U8 CheckAnt;//Antenna check configuration 0: antenna check off;	1: antenna check on.
} ;
struct working_mode_t
        {
    U8 ReadMode;
    U8 TagProtocol;
    U8 ReadPauseTime;
    U8 FilterTime;
    U8 QValue;
    U8 Session;
    U8 MaskMem;
    U16 MaskAdr;
    U8 MaskLen;
    U8 MaskData[32];
    U8 AdrTID;
    U8 LenTID;
        }  __attribute__((__packed__)) ;


class Cfmu804 : public RfidReader
{
    friend z_factory_t<Cfmu804>;
    std::mutex _mutex_command;

    z_status __cmd_tx(U8 cmd_code,U8* data=0,U8 data_len=0);
    int _maskMem=0;
    int _tagProtocol=0;
protected:
    z_status _rx_thread_start();
    z_status _rx_thread_stop();
    z_status _hw_open();
    z_status _hw_close();
public:

    void  rx_thread();
    bool  _rx_frame(ResponseFrame* frame);

   // z_obj_vector<Response> _cmd_responses;

    //int port_speed = 9600;
    //int _port_speed = 57600;


    int _port_speed = 115200;
    static const int _buff_size = 300;
    U8 _buff_tx[_buff_size];


    bool _rx_quit = false;
    bool _rx_started = false;
    z_safe_map<U8,Response,true> _responses;

    std::thread _rx_thread_handle;

    serial::Serial _port;

    bool _debug_rx_frames=false;
    bool _debug_rx_bytes=false;
    bool _debug_response=false;
    bool _debug_tx=false;

    z_status _readmode_get(working_mode_t& mode);
    z_status _readmode_set(working_mode_t& mode);
	z_status _info_get(reader_info_t* p_info);

public:


    z_string _port_name =
#ifdef ARM
    "/dev/ttyAMA0";
#else
    "/dev/ttyUSB0";

#endif
    virtual ~Cfmu804()
    {
    }

	z_status send_cmd_byte(U8 cmd_code,U8 data);
    z_status cmd_single_byte_return(U8 cmd_code,U8 *data);



/*
    z_status set_freq()
    {
        U8 data[2] = { 0xF1,0 };
        cmd_tx(0x22, (U8*)&data, 2);
        return zs_ok;

    }
*/
    z_status readtime(int time_sec)
    {
        send_cmd_byte(0x76,1);
        z_sleep_ms(time_sec*1000);
        send_cmd_byte(0x76,0);

        return zs_ok;

    }
    z_status fast_read(int time_sec)
    {
        send_cmd_byte(0x50,0);
        z_sleep_ms(time_sec*1000);
        cmd(0x51);

        return zs_ok;

    }
	z_status baud_rate_set()
    {
    	send_cmd_byte(0x28,6);
    	return zs_ok;
    }
    z_status _read_stop()
    {
        send_cmd_byte(0x76,0);
        //TODO wait for reading to actually stop
        return zs_ok;
    }
    z_status antCheck();
    z_status _read_start()
    {
        send_cmd_byte(0x76,1);

        return zs_ok;
    }
    z_status send_command(U8 code, U8* tx_data, int tx_len, int wait_time_ms = 2000, cfmu_status_code *p_status=0,
                          void* rx_data=0, int max_rx_len=0, int* var_len=0);
    z_status badcmd();
    z_status measure_ant(int antnum);
    z_status temperature_get()
    {
        zout << get_temperature_cmd() << "\n";
        return zs_ok;

    }
    U8 get_ant_loss(int antnum);

    int get_temperature_cmd();
    virtual z_status config_write();
	virtual z_status config_read();
    virtual z_status ant_dump();

    virtual z_status readmode_get() override;
    virtual z_status readmode_set();
    virtual z_status info_dump() override;
    virtual z_status inventory() ;
    virtual z_status inv(int session,int target) ;
    virtual z_status inventory_single() ;
    virtual RfidRead* read_single() ;
    virtual z_status exp_data_read() ;

	z_status profile_set_get(U8 &profile_return,U8 profile_set=0);
	z_status profile_set(int val);
	z_status profile_dump();
    z_status set_return_loss(int val)
    {
        U8 data = val;
        return send_command(0x6e,&data,1);
    }
	z_status beep_off()
    {

    	U8 data = 0;

    	return send_command(0x40, &data, 1);
    }
	z_status beep_on()
    {

        U8 data = 1;

    	return send_command(0x40, &data, 1);
    }
	z_status write_power_set(int val)
    {
    	U8 data = val;
    	return send_command(0x79, &data, 1);
    }
	z_status write_power_get()
    {
    	U8 data = 0;
    	if (cmd_single_byte_return(0x7a, &data)==zs_ok) {
    		_write_power = data;
    		ZDBG("Write power set to %d\n",_write_power);
    		return zs_ok;
    	}
    	return zs_io_error;
    }
	z_status power_set(int val)
    {
    	U8 data = val;

    	z_status status= send_command(0x2f, &data, 1);
		if (status==zs_ok)
		{
			_power=val;
			ZLOG("Power set to %d\n",_power);
		}
		return status;
    }
	/*
	 *Scantime: inventory time. Reader will modify the maximum response time according to user defined value (0*100ms ~ 255*100ms),
	 *and reader will apply this new setting for future inventories.
	 *Default setting of Scantime is 0x14 (corresponding to 20*100ms).
	 *Valid setting of Scantime is 0x00 ~ 0xff (corresponding to 3*100ms ~ 255*100ms).
	 */
	z_status scan_time_set(int val)
    {
    	U8 data = val;
    	return send_command(0x25, &data, 1);
    }
    z_status ant_check_set(int val)
    {
        U8 data = val;
        return send_command(0x66, &data, 1);
    }
    z_status ant_cfg_set(int val)
    {
        U8 data = val;

        return send_command(0x3f, &data, 1);

    }
	/*
	 * Set freq quadrants, 0-3
	 * Full range: low=0 hi=3
	 * Bottom half: low=0 hi=1
	 * Top half : low=2 hi=3
	 */
	z_status freq_set(U8 low,U8 max);
    z_status cmd(int cmd)
    {
        return send_command(cmd,0,0);
    }

    z_status write_bcd(int number);
    z_status program(int number,bool overwrite);
    cfmu_status_code program_epc(Epc& epc);
    RfidRead* program_bcd(int number,bool overwrite,bool& programmed);



};
ZMETA_DECL(Cfmu804)
{
    ZBASE(RfidReader);
    ZPROP(_port_speed);
    ZPROP(_debug_tx);
    ZPROP(_debug_rx_bytes);
    ZPROP(_debug_rx_frames);
    ZPROP(_debug_response);

    ZPROP(_maskMem);
    ZPROP(_tagProtocol);

    ZPROP(_port_name);
    ZACT(badcmd);
    ZACT(inventory_single);
    ZACT(ant_dump);

    ZACT(inventory);

    ZACT(exp_data_read);
    ZACT(info_dump);
    ZACT(beep_on);
    ZACT(beep_off);
    ZACT(readmode_get);
    ZACT(write_power_get);
    //ZACT(set_freq);
    ZCMD(measure_ant, ZFF_CMD_DEF, "measure_ant",
         ZPRM(int, ant, 0, "ant", ZFF_PARAM)
    );
	ZCMD(write_power_set, ZFF_CMD_DEF, "write_power_set",
		 ZPRM(int, number, 10, "number", ZFF_PARAM)
	);
	ZCMD(profile_set, ZFF_CMD_DEF, "profile_set",
		 ZPRM(int, number, 13, "number", ZFF_PARAM)
	);
    ZACT(profile_dump);


	ZCMD(inv, ZFF_CMD_DEF, "inv",
		 ZPRM(int, session, 0, "session", ZFF_PARAM),
		 ZPRM(int, target, 0, "target", ZFF_PARAM)
	);

    ZCMD(write_bcd, ZFF_CMD_DEF, "write_bcd",
         ZPRM(int, number, 0, "number", ZFF_PARAM)
    );
    ZCMD(program, ZFF_CMD_DEF, "program",
         ZPRM(int, number, 0, "number", ZFF_PARAM),
         ZPRM(bool, overwrite,false, "overwrite", ZFF_PARAM)
    );

    ZACT(baud_rate_set);
    ZACT(temperature_get);
    ZCMD(readtime, ZFF_CMD_DEF, "readtime",
         ZPRM(int, time_sec, 0, "time_sec", ZFF_PARAM)
    );
    ZCMD(fast_read, ZFF_CMD_DEF, "fast_read",
     ZPRM(int, time_sec, 0, "time_sec", ZFF_PARAM)
    );
    ZCMD(cmd, ZFF_CMD_DEF, "cmd",
         ZPRM(int, cmd, 0, "code", ZFF_PARAM)
    );
    ZCMD(send_cmd_byte, ZFF_CMD_DEF, "cmd2",
         ZPRM(int, cmd, 0, "code", ZFF_PARAM),
         ZPRM(int, data, 0, "data", ZFF_PARAM)
    );
	ZCMD(freq_set, ZFF_CMD_DEF, "freq_set",
	 ZPRM(int, low, 0, "low quad", ZFF_PARAM),
	 ZPRM(int, hi, 3, "hi quad", ZFF_PARAM)
);
    ZCMD(set_return_loss, ZFF_CMD_DEF, "set_return_loss", ZPRM(int, val, 0, "val", ZFF_PARAM));
    ZCMD(ant_check_set, ZFF_CMD_DEF, "ant_check_set", ZPRM(int, val, 0, "val", ZFF_PARAM));
    ZCMD(ant_cfg_set, ZFF_CMD_DEF, "ant_cfg_set", ZPRM(int, val, 0, "val", ZFF_PARAM));
	/*
 *Scantime: inventory time. Reader will modify the maximum response time according to user defined value (0*100ms ~ 255*100ms),
 *and reader will apply this new setting for future inventories.
 *Default setting of Scantime is 0x14 (corresponding to 20*100ms).
 *Valid setting of Scantime is 0x00 ~ 0xff (corresponding to 3*100ms ~ 255*100ms).
 */
    ZCMD(scan_time_set, ZFF_CMD_DEF, "scan_time_set", ZPRM(int, val, 0x14, "val", ZFF_PARAM));
};



#endif