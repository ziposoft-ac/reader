
#include "pch.h"

#include "zipolib/zipolib.h"
#include "zipolib/csv_files.h"
#include <iostream>
#include <fstream>      // std::ifstream
using namespace std;

/*________________________________________________________________________

z_parse_csv

________________________________________________________________________*/
#define LIMIT_COL 10
#define LIMIT_ROW 1000
z_status z_csv_encode_string(z_string& output)
{
	//Z_ASSERT(0);//THIS doesnt work
	size_t dq = 0;
	if (output.find_first_of(",\"\n") == z_string::npos)
		return zs_success;

	output.insert(0, 1, '\"');
	while ((dq = output.find_first_of("\"", dq)) != z_string::npos)
	{
		output.insert(dq, 1, '\"');

	}
	output.append(1, '\"');

	return zs_success;
}
z_parse_csv::z_parse_csv()
{
	_column_idx = 0;
	_row_idx = 0;
	_buff = 0;
	_i = 0;
	_buffSize = 0;
	_num_columns = 0;
	_delemiter = ',';
}
z_parse_csv::~z_parse_csv()
{
}
bool z_parse_csv::Inc()
{
	if (_bBufferEnd) return true;
	_i++;
	if (_i == _buffSize) _bBufferEnd = true;
	return _bBufferEnd;
}
#if 0
z_status z_parse_csv::ParseBuffer(ctext buff, size_t size)
{
	_column_idx = 0;
	_row_idx = 0;
	_i = 0;
	_bBufferEnd = false;
	_buff = buff;
	_buffSize = size;
	if (size == 0)
		return zs_bad_parameter;
	while (_bBufferEnd == false)
	{
		ZT("%20.20s", _buff + _i);
		if (!NewRowCallback())
		{
			ZT("!NewRowCallback()");
			return zs_unparsed_data;
		}
		if (!ParseLine())
		{
			ZT("ParseLine failed\n");
			return zs_unparsed_data;
		}
		if (!EndRowCallback())
		{
			ZT("!EndRowCallback()");
			return zs_unparsed_data;
		}
		_row_idx++;
	}
	ZT("ParseBuffer() done");
	return zs_ok;
}

z_status z_parse_csv::ParseLine()
{
	_column_idx = 0;
	_bLineEnd = false;


	while (_bLineEnd == false)
	{
		if (!ParseValue())
			return false;
		if (_bLineEnd && (_column_idx == 0) && (_value == ""))
		{
			//If it is a blank svg:line, skip it.
			_bLineEnd = false;
			continue;
		}
		if (_value.length() == 0)
			return false;
		if (!NewValueCallback(_value)) return false;
		_column_idx++;
		if (_bBufferEnd) break;
	}
	return zs_ok;

}
#endif

z_status z_parse_csv::ParseLine(const z_string & line, z_strlist& row)
{
	_buff = line.c_str();
	_buffSize = line.size();
	_column_idx = 0;
	_bLineEnd = false;
	_bBufferEnd = false;
	_i = 0;


	while (_bLineEnd == false)
	{
		z_status status = ParseValue();
		if (status)
			return status;
		// Why was this here? 0 length data is valid.
		if (_bLineEnd && (_column_idx == 0) && (_value == ""))
		{
			//If it is a blank svg:line, skip it.
			_bLineEnd = false;
			continue;
		}
		//if (_value.length() == 0) 			return false;
		row.push_back(_value);
		_column_idx++;
		if (_bBufferEnd) break;
	}
	return zs_ok;

}
//TODO this is terrible rewrite
z_status z_parse_csv::ParseValue()
{
	//	bool bValueEnd=false;
	_value = "";
	_bInsideString = false;

	while (_i<_buffSize)
	{
		if (_bBufferEnd) return zs_ok;
		U8 c = _buff[_i];
		U8 c2 = 0;
		if ((_i + 1)<_buffSize)
			c2 = _buff[_i + 1];
		Inc();
		if (_bInsideString)
		{
			if (c == '\"')
			{
				if (c2 == '\"')
					Inc();
				else
				{
					_bInsideString = false;
					continue;
				}
			}
		}
		else
		{
			if (c == '\"')
			{
				_bInsideString = true;
				continue;
			}
			if (c == _delemiter) return zs_ok;

			if (
				((c == 0xD) && (c2 != 0xA)) //Crapintosh
				|| (c == 0xA) //Unix
				)
			{
				_bLineEnd = true;
				return zs_ok;
			}
			if (c == 0xD) //DOS
			{
				continue; //next byte is the newline
			}
			if ((c != '\t') && (c<' '))
			{
				c = '~';
			}
			else
			{
				//continue; //skip white space?
			}

		}
		_value += c;
	}
	_bBufferEnd = true;
	return zs_ok;
}
z_status z_parse_csv::DetectDelimiter(ctext filename, char& del)
{
#if 0
	ifstream ifs(filename, std::ifstream::in);
	if (!ifs.is_open())
		return Z_ERROR(zs_could_not_open_file);


	auto y = [&](auto first, auto second)
	{
		r.e = 3;
		if (!ifs.is_open())
			return Z_ERROR(zs_could_not_open_file);
		return first + second;
	};

	_delemiter = ',';

	z_string line;
	_column_idx = 0;
	_row_idx = 0;
	z_status status;
	z_strlist row;
	while (!ifs.eof())
	{
		row.clear();
		getline(ifs, line);
		if (line.size() == 0)
			continue;
		if (status = ParseLine(line, row))
		{
			ZT("ParseLine failed\n");
			return status;
		}
		_row_idx++;
	}
#endif
	return zs_ok;
}



z_status z_parse_csv::ParseFile(ctext filename,  char delemiter,int max_rows)
{
	_delemiter = delemiter;
	ifstream ifs(filename, std::ifstream::in);
	if (!ifs.is_open())
		return Z_ERROR(zs_could_not_open_file);
	z_string line;
	_column_idx = 0;
	_row_idx = 0;
	z_status status;
	z_strlist row;
    StartCallback();
	while (!ifs.eof())
	{
		
		row.clear();
		getline(ifs, line);
		if (line.size() == 0)
			continue;
		//ZT("%20.20s", line.c_str());
		if (status=ParseLine(line, row))
		{
			ZT("ParseLine failed\n");
			return status;
		}
		if (status=RowCallback( row))
		{
			ZT("!RowCallback()");
			return status;
		}
		_row_idx++;
		if (max_rows)
			if (max_rows == _row_idx)
				break;
	}

	return zs_ok;
}

