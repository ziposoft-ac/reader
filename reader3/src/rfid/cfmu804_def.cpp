#include "cfmu804.h"




const cfmu_cmd_t cfmu_cmd_table[] = {
        {0x21, "Obtain reader information", "Obtain reader information"},
        {0x76, "Modify working mode", "Modify working mode"},

};



#if 0
0x01	Tag Inventory
0x02	Read data command
0x03	Write data command
0x04	Write EPC number
0x05	Kill tag
0x06	Set memory read / write protection for specific memory
0x07	Block erase
0x08	Read protection configuration(according to EPC number)
0x09	Read protection configuration(Without EPC number)
0x0a	Unlock read protection
0x0b	Read protection status check
0x0c	EAS configuration
0x0d	EAS alert detection
0x0f	Single tag inventory
0x10	Write blocks
0x11	Obtain Monza4QT working parameters
0x12	Modify Monza4QT working parameters
0x15	Extended data reading with assigned mask
0x16	Extended data writing with assigned mask
0x18	Inventory with memory buffer
0x19	Mix inventory
0x1a	Inventory with EPC number
0x1b	QT inventory
		
0x50	Inventory command for SINGLE tag.This command will only inquire one tag in every cycle, with NO inventory condition.
0x51	Inventory command for MULTIPLE tags.Inquire tags according predefined conditionand only return UIDs of the corresponding tags.
0x52	Read data command.Read data from tag, maximum 32 bytes in every command cycle.
0x53	Write data command.Write data to tag, maximum 32 bytes in every command cycle.
0x54	Obtain lock status command.Check the lock status of a particular memory unit.
0x55	Byte locking command.Lock a particular(unlocked) byte in a tag.
			

0x21	Obtain reader information
0x22	Modify working frequency
0x24	Modify reader address
0x25	Modify reader inventory time
0x28	Modify serial baud rate
0x2f	Modify RF power
0x33	LED/Buzzer control
0x3f	Setup antenna multiplexing
0x40	Enable/disable beepPwm
0x46	GPIO control
0x47	Obtain GPIO state
0x4c	Obtain the reader unique serial number
0x3a	Modify tag customised function
0x66	Enable antenna check
0x6a	Modify communication interface
0x6e	Modify or load Antenna return loss threshold configuration
0x70	Modify maximum EPC/TID length configuration for memory buffer
0x71	Load the maximum EPC/TID length configuration
0x72	Obtain data from memory buffer
0x73	Clear memory buffer
0x74	Obtain the total tag amount from memory buffer
0x75	Modify parameters of real time inventory mode
0x76	Modify working mode
0x77	Load real time inventory mode parameters
0x78	Load/modify heartbeat packet time break of real time inventory
0x79	Modify RF power configuration separately for write operations
0x7a	Load the RF power configuration of write operations
0x7b	Modify or load maximum write retry time configuration
0x7d	Modify password of tag customised functions
0x7e	Obtain password of tag customised functions
0x7f	Load/modify reader profile
0x85	Synchronise EM4325 timestamp
0x86	Obtain EM4325 temperature data
0x87	Obtain external data via EM4325 SPI
0x88	Reset EM4325 alert
0x90	Modify or load DRM configuration
0x91	Measure antenna return loss
0x92	Measure current reader temperature


ResponseReaderInfo

 Version
2
Firmware version, the high byte states the main version number and the low byte states the subversion number.
Type
1
Reader model code, 0x20 is UHFReader288MP
Tr_Type
1
Supported prototypes.
bit1 = 1: supports 18000-6C.
bit0 = 1: supports 18000-6B.
All other bits are reserved.
dmaxfre
1
bit7 ~ bit6: frequency band configuration;
bit5 ~ bit0: maximum frequency point.
dminfre
1
bit7 ~ bit6: frequency band configuration;
bit5 ~ bit0: minimum frequency point.
Power
1
Output RF power, range from 0 to 30.
Scntm
1
Inventory time.
Reader will respond the inventory command delivered from host within this specific inventory time.
Ant
1
Antenna configuration.
Reserved
1
Reserved.
Reserved
1
Reserved.
CheckAnt
1
Antenna check configuration
0: antenna check off;
1: antenna check on.



#endif

#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL  0x8408
unsigned int uiCrc16Cal(unsigned char const* pucY, unsigned char ucX)
{
    unsigned char ucI, ucJ;
    unsigned short int  uiCrcValue = PRESET_VALUE;
    for (ucI = 0; ucI < ucX; ucI++)
    {
        uiCrcValue = uiCrcValue ^ *(pucY + ucI);
        for (ucJ = 0; ucJ < 8; ucJ++)
        {
            if (uiCrcValue & 0x0001)
            {
                uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
            }
            else
            {
                uiCrcValue = (uiCrcValue >> 1);
            }
        }
    }
    return uiCrcValue;
}






const cfmu_error_t cfmu_error_table[] = {
	{0x00, "Operation succeed", "Command is successfully executed, all the requested data is included in the Data[] field of response frame."},
	{0x01, "Inventory succeed", "Host delivered G2 tag inventory command, tag inventory was completed successfully and reader is able to deliver data response within the predefined inventory time."},
		{0x02, "Inventory timeout", "Host delivered G2 tag inventory command, reader fails to complete the inventory within the predefined inventory time."},
		{0x03, "Further data is waiting to be delivered", "Host delivered G2 tag inventory command, reader is not able to response all the data in a single frame, further data will be transmitted in the following frames. "},
		{0x04, "Reader memory is full ", "Host delivered G2 tag inventory command, reader has completed parts of the inventory and run out of memory space due to the amount of tags. Reader will response the inquired EPC number."},
		{0x05, "Access password error", "Host delivered an incorrect access password to reader."},
		{0x09, "Tag killing failed ", "Host delivered a G2 tag killing command, but the kill operation is failed due to incorrect tag killing password or poor communication between reader and tag."},
		{0x0a, "All-zero tag killing password", "It is not possible to kill a Tag with all- zero tag killing password."},
		{0x0b, "Command is not support by the tag", "Some optional commands stated in G2 protocol and some manufacturers' customised command may not be supported by the tags.   "},
		{0x0c, "All-zero access password", "For NXP UCODE EPC G2X tag, it is not possible to enable reader protection or enable EAS alert if the tag access password is all-zero."},
		{0x0d, "Fail to enable read protection ", "The read protection is enabled for the target NXP UCODE EPC G2X tag. It is not possible to lock a protection enabled tag."},
		{0x0e, "Fail to unlock the tag", "The target NXP UCODE EPC G2X tag is already unlocked or the tag locking function is not support by the target tag."},
		{0x10, "Fail to perform write operation ", "Some bytes stored in the target 6B tag are locked."},
		{0x11, "Fail to perform lock operation ", "Fail to perform lock operation on a 6B tag."},
		{0x12, "Fail to perform lock operation ", "The target 6B tag is locked."},
		{0x13, "Fail to stored the parameter value", "Fail to store the value of some preserved parameters. Configuration will still valid before reader shut down"},
		{0x14, "Modification failed", "Fail to adjust the RF power."},
		{0X15, "response within the predefined inventory time", "Host delivered 6B tag inventory command, tag inventory was completed successfully and reader is able to deliver data response within the predefined inventory time."},
		{0x16, "Inventory timeout", "Host delivered 6B tag inventory command, reader fails to complete the inventory within the predefined inventory time."},
		{0x17, "Further data is waiting to be delivered", "Host delivered 6B tag inventory command, reader is not able to response all the data in a single frame, further data will be transmitted in the following frames."},
		{0x18, "Reader memory is full", "Host delivered 6B tag inventory command, reader has completed parts of the inventory and run out of memory space due to the amount of tags. Reader will response the inquired EPC number."},
		{0x19, "All-zero access password or function is not supported", "Fail to enable EAS alert. It may caused by an all-zero tag access password or the EAS alert is not supported by the target tag."},
		{0x1A, "Fail to execute tag customised command", "Some special tag functions are enabled, but fail to execute some commands."},
		{0x26, "This frame contains statistic data packet", "Host delivered G2 tag inventory command, tag inventory was completed successfully and reader is now delivering statistic data packet."},
		{0x28, "This frame contains heartbeat packet", "In real time inventory mode, no tag is detected within the heartbeat packet time break, reader will upload heartbeat data."},
		{0xF0, "Quit waiting for response", "The process recieved a quit_notify before response was received."},
        {0xF1, "Return length error", "Data return length does not match expected length."},
        {0xF2, "Not Open", "Could not open"},
		{0xF8, "Antenna connection check failure", "Antenna connection error detected before tag operation."},
		{0xF9, "Fail to execute command", "Command execution error."},
		{0xFA, "Operation failed", "Tags are detected within the effective field, but failed to complete the operation due to poor communication between reader and tags."},
		{0xFB, "No operatable tags", "No operatable tag is detected in the effective range."},
		{0xFC, "Error code returned from tags", "RFID tag reported an error to reader, reader will present this code in the ""Err_code"" field in the response."},
		{0xFD, "Command length error", "Host delivered a command frame with incorrect frame length"},
		{0xFE, "Illegal command", "Host delivered a incorrect  command frame, e.g. unrecognised command code or a command frame with CRC error (failed to pass CRC16 check)"},
		{0xFF, "Parameter error", "Host delivered unrecognised parameters in a command frame."}
	
};
const cfmu_error_t cfmu_error_unknown = { 0, "Unkown error", "Error Code is not in error table" };

const cfmu_error_t* get_cfmu_error(cfmu_status_code code)
{
	int i;
	for (i = 0; i < sizeof(cfmu_error_table) / sizeof(cfmu_error_t); i++)
	{
		if (cfmu_error_table[i].code == code)
			return &cfmu_error_table[i];
	}
	return &cfmu_error_unknown;

}
void print_cfmu_error(z_stream& stream,cfmu_status_code status)
{
    const cfmu_error_t* err=get_cfmu_error(status);
    stream.format_append("CFMU Error %02x\n",status);
    stream << err->error << '\n';
    stream << err->desc << '\n';

}
