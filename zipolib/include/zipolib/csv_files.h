#ifndef CSV_FILES_H
#define CSV_FILES_H
#include <zipolib/zipolib.h>
//#include <zipolib/z_map.h>

z_status z_csv_encode_string(z_string& output);

//class z_parse_csv_row : public std::vector<z_string>{};

class z_parse_csv
{
	int _column_idx;
	int _num_columns;
	size_t _buffSize;
	ctext _buff;
	size_t _i;
	z_string _value;

	char _delemiter;
protected:
	int _row_idx;

public:
	z_parse_csv();
	virtual ~z_parse_csv();


	z_status DetectDelimiter(ctext filename, char& del);
	virtual z_status ParseFile(ctext filename,char delemiter=',',int max_rows=0);
#if 0
	virtual z_status ParseBuffer(ctext buff, size_t size);
	virtual z_status ParseLine();
#endif
	virtual z_status ParseLine(const z_string & line, z_strlist &row);
	virtual z_status ParseValue();
	bool _bInsideString;
	bool _bLineEnd;
	bool _bBufferEnd;
	bool Inc();
	virtual z_status RowCallback(z_strlist& row)
	{
		//z_printf("\n");
		return zs_ok;
	}
    virtual z_status StartCallback()
    {
        return zs_ok;
    }
	virtual z_status NewRowCallback()
	{
		//z_printf("\n");
		return zs_ok;
	}
	virtual z_status EndRowCallback()
	{
		//z_printf("\n");
		return zs_ok;
	}
	virtual z_status NewValueCallback(const z_string & value)
	{
		//z_printf("%s ",value.c_str());
		return zs_ok;
	}
};
class z_parse_csv_file : public z_parse_csv
{
public:
    std::vector<z_strlist>* pdata=0;
    virtual z_status ParseFileData(ctext filename,std::vector<z_strlist>& data,char delemiter=',',int max_rows=0)
    {
        pdata=&data;
        return ParseFile(filename,delemiter,max_rows);
    }
    virtual z_status RowCallback(z_strlist& row)
    {
        pdata->push_back(row);
        return zs_ok;
    }
};
/*

template <class CSV_OBJ> class z_csv_map : public z_parse_csv, public z_obj_map<CSV_OBJ>
{
public:
	


};
*/
#endif
