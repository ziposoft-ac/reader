#include "pch.h"

#include "zipolib/cgi.h"
#include "zipolib/z_filesystem.h"
#include "zipolib/z_factory.h"
#include "zipolib/z_file.h"
#ifdef WIN32
#include <io.h>
#include <fcntl.h>

#endif
/*

/7823502@N04/
156936854@N02
27014743397


dummyz::dummyz()
{
	x = "fred";
}
*/
ZMETA(cgi_input)
{
	ZBASE(z_factory_controller);
	ZACT_JSON(get_obj_features_json);
	ZPROP(_dumpenv);

};


Z_FC_COMMANDS(cgi_input)
{
	Z_FC_BASE(z_factory_controller);
	ZFC_ACT_JSON(get_obj_features_json);

}

void cgi_input::init_commands()
{
	Z_FC_INIT(cgi_input);

}



const char * CONTENT_MULTIPART = "multipart/form-data;";
const char * CONTENT_URL = "application/x-www-form-urlencoded";


z_status cgi_input::get_obj_features_json(z_json_stream& s, z_json_obj& params)
{
	zf_node& n = get_temporary_evaluation_node();
	
	//n.get_factory()->json_data_fact(s, n.get_obj(), ZFF_JSON_STRUCT);

	n.get_factory()->json_data_fact(s, n.get_obj(), ZFF_JSON_STRUCT);

	return zs_ok;


}


#if 0
cgi_input::cgi_input()
{

	_bSimulate = false;
	_bSaveContext = true;
	_contextFile = "cgi_context.txt";

}
#endif
cgi_input::~cgi_input()
{


}
#define ENVAR(X) { #X,  _##X }
z_status cgi_input::env_load()
{
	struct EnvVar
	{
		ctext _name;
		z_string& _memVar;
	};
	const EnvVar vars[] =
	{
		ENVAR(REQUEST_METHOD),
		ENVAR(REMOTE_ADDR),
		ENVAR(QUERY_STRING),
		ENVAR(HTTP_COOKIE),
		ENVAR(HTTP_REFERER),
		ENVAR(SCRIPT_NAME),
		ENVAR(SCRIPT_ROOT),
		ENVAR(HTTP_USER_AGENT),
		ENVAR(REMOTE_HOST),
		ENVAR(REQUEST_URI),
		ENVAR(PATH_INFO),
		ENVAR(PATH_TRANSLATED),
		ENVAR(CONTENT_TYPE),
		ENVAR(CONTENT_LENGTH),
		ENVAR(REMOTE_IDENT),
		ENVAR(REMOTE_USER),
	};
	const uint varCount = sizeof(vars) / sizeof(EnvVar);

	uint i;

	if (_bSimulate)
	{
		if (z_file_exists(_contextFile))
			return zs_not_found;
		std::ifstream file(_contextFile);
		while (true)
		{
			std::string name, val;
			std::getline(file, name, '=');
			if (file.eof())
				break;
			std::getline(file, val);
			if (name == "")
			{
				return Z_ERROR_MSG(zs_parse_error, "error reading file: %s", _contextFile);
			}
			for (i = 0; i < varCount; i++)
			{
				if (name == vars[i]._name)
				{
					vars[i]._memVar = val;
					break;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < varCount; i++)
		{
			ctext val = getenv(vars[i]._name);
			if (!val)
				val = "";
			vars[i]._memVar = val;
		}


		if (_bSaveContext)
		{
			std::ofstream file(_contextFile);
			for (i = 0; i < varCount; i++)
				file << vars[i]._name << "=" << vars[i]._memVar << std::endl;
		}

	}
	if(_dumpenv)
		for (i = 0; i < varCount; i++)
		{
			ZDBG("%s: %s \n", vars[i]._name, vars[i]._memVar.c_str());
		}


	return zs_ok;

}

bool cgi_input::is_cgi_mode(bool debugReplay)
{
	z_status status = zs_ok;
	_bSimulate = debugReplay;
	status = env_load();
	if (!_REQUEST_METHOD)
	{
		// We are not in CGI mode 
		// So just bail out
		return false;
	}
	return true;


}

z_status cgi_input::parse_input()
{
	z_status status;
	int content_len = _CONTENT_LENGTH.get_int_val();


	if(content_len)
		parse_content();


	if(_QUERY_STRING)
		parseGetStr(_QUERY_STRING);

	return zs_ok;

}
z_status cgi_input::parse_content()
{
	z_status result = zs_internal_error;
	ctext FILE_ASCII = "post.txt";
	ctext FILE_BIN = "post.bin";
	ctext fileName = FILE_ASCII;
	char* post_url_buff = 0;
	int content_len = _CONTENT_LENGTH.get_int_val();
	bool bContentMulti = _CONTENT_TYPE.contains(CONTENT_MULTIPART);
	std::ifstream* p_file = 0;

	try
	{
		if (bContentMulti)
		{
			fileName = FILE_BIN;
			if (!_CONTENT_TYPE.contains(CONTENT_URL))
			{
				Z_THROW_MSG(zs_operation_not_supported, "Unknown content type");
			}
		}
		std::istream* p_stream = &(std::cin);
		
		if ((_bSaveContext) && (!_bSimulate))
		{
			ZT("saving stdin to file %s %d", fileName, content_len);
			z_file_save_from_stdin(fileName, content_len);
		}
		bool bReadFromFile = _bSaveContext || _bSimulate;
		if (bReadFromFile)
		{
			p_file = z_new std::ifstream(fileName);
			if (!p_file->is_open())
			{
				Z_THROW_MSG(zs_could_not_open_file, "cant open POST file");
			}
			p_stream = p_file;
		}

		if (bContentMulti)
		{
			result = parsePostMultiPart(*p_stream);
		}
		else
		{
			post_url_buff = z_new char[content_len + 1];
			p_stream->read(post_url_buff, content_len);

			result = parseGetStr(post_url_buff, content_len);
		}

	}
	catch (z_except& ex)
	{
		if (p_file)
			delete p_file;

		if (post_url_buff)
			delete post_url_buff;
		Z_RETHROW("parsing POST failed");
	}
	if (p_file)
		delete p_file;

	if (post_url_buff)
		delete post_url_buff;


	return result;
}
z_status cgi_input::parsePostMultiPart(std::istream& is)
{
	z_string line;
	z_string boundary;
	size_t index = _CONTENT_TYPE.find('=');
	boundary.assign(_CONTENT_TYPE, index + 1, _CONTENT_TYPE.size() - index);
	return zs_ok;

#if 0
		while (1)
		{
			getline(is, line);
			if(line!=)
			if (!ReadLine()) break; //Read boundary svg:line ---------3132230333176
			if (strstr(_lineStart, _boundary.c_str()) == 0) break; //if it does not contain boundary, quit
			if (!ReadLine()) break; // Read "Content-Disposition" svg:line
			//Get the variable name 
			char* var = strchr(_lineStart, '\"');
			if (!var) break;
			var++;
			char* end = strchr(var, '\"');
			if (!end) break;
			char* uploadfilename = strstr(end, "filename");//Check for file name
			*end = 0;

			//Check for file name
			if (uploadfilename)
			{
				uploadfilename = strchr(uploadfilename, '\"');
				if (!uploadfilename) break;
				uploadfilename++;
				end = strchr(uploadfilename, '\"');
				if (!end) break;
				char* start = strrchr(uploadfilename, '\\');
				if (start) uploadfilename = start + 1;
				*end = 0;
			}
			/*

			*/
			if (!ReadLine()) break;//Blank svg:line
			if (_lineStart == _lineEnd) //if this was a blank svg:line
			{
				_lineStart = _nextLine;
				_lineEnd = FindEndOfData(); //Get the variable value
				if (!_lineEnd) break;
				*_lineEnd = 0;
				_nextLine = _lineEnd + 2;
				//AddVariable(var,_lineStart,&_context_in);
				_query[var] = _lineStart;
			}
			else //Binary FILE
			{
				ZDBG("found uploaded file");

				if (!ReadLine()) break;
				char* binend = FindEndOfData();
				ZDBG("binend=%x", binend);
				if (binend == 0) break;
				if (z_directory_change("upload", 1)) break; //TODO !! GOD THIS IS AWFUL!
				z_file_out ofile(uploadfilename);
				ZDBG("_nextLine=%x binend-_nextLine=%d", _nextLine, binend - _nextLine);
				ofile.write(_nextLine, binend - _nextLine);
				ZDBG("out of ofile.write");
				ofile.close();
				if (z_directory_change("..", 0)) break;
				_nextLine = binend;
				_query[var] = uploadfilename;
				//AddVariable(var,uploadfilename,&_context_in);
				ZDBG("got here?");
				if (!ReadLine()) break;
			}

		}


		delete _pPostBuffer;
		delete _pLineBuffer;

		return;

	}
#endif

}


void cgi_input::dumpQuery()
{

	for (auto i : _query)
	{
		gz_stdout << i.first << " = " << i.second << "\n";
	}

}
z_status cgi_input::ExecQuery(z_json_stream&output)
{

	zf_command_line_parser parser(this);
	z_status status = zs_not_implemented;

	z_time time;
	//if (_show_duration)		time.set_now();
	//if (_show_duration)		out << "\n" << time.get_elapsed_ms() << "ms\n";

	z_string cmd;
	if (!_query.get("command", cmd))
	{
		Z_THROW_MSG(zs_not_found, "No command specified");

	}

	zf_node_path path(get_root());
	z_string feature_str;
	try
	{

		status = parser.evaluate_path(cmd, path, feature_str);
		if (zs_ok == status)
		{
			zf_node node = path.get_selected();
			//this is horrible. fix this
			callback_set_temporary_evaluation_node(node);
			z_factory* fact = node.get_factory();
			if (!fact)
				return zs_wrong_object_type;
			z_void_obj* vobj = node.get_obj();
			if (!vobj)
				return zs_wrong_object_type;


			zf_mfunc_json* act = fact->get_feature_t< zf_mfunc_json>(feature_str);
			if (!act)
			{
				act = dynamic_cast<zf_mfunc_json*>(get_command(feature_str));
				if (act)
				{
					node = get_self();

				}

			}

			if (!act)
			{
				Z_THROW_MSG(zs_not_found, "'%s' is not a feature of '%s'",feature_str.c_str(),fact->get_name());
				return zs_not_found;
			}


			zf_param_list* act_params = act->get_param_list();
			if (act_params)
			{
			    for(auto param : *act_params)
                {
					z_string qval;
					if (!param)
						return Z_ERROR(zs_internal_error);
					ctext name = param->get_name();
					if (_query.get(name, qval))
                        param->set_from_string(qval, vobj);
				}
			}
			zf_cc_cgi cc(this);
			cc.set_cc(node, ZFF_EXE, output);
			act->execute(cc);
		}
		else
		{
			z_string error = "Error parsing request\n";
			parser.print_context(error);
			Z_THROW_MSG(zs_parse_error, error);
		}
	}
	catch (z_except& ex)
	{
		throw;

	}
	if (status)
	{

	}

	return status;

}
z_status cgi_input::parseGetStr(z_string& s)
{
	return parseGetStr(s.c_str(),s.length());
}

z_status cgi_input::parseGetStr(ctext str, size_t length)

{
	z_status status = zs_ok;
	char *pVarStart = z_new char[length + 1];
	ctext iInput = str;
	char * iNew = pVarStart;
	ctext end = iInput + length;
	char *pValStart = 0;
	bool add_var = false;
	try
	{
		while (iInput < end)
		{
			char x = *iInput;
			iInput++;
			switch (x)
			{
			case '+':
				x = ' ';
				break;
			case '%':
			{
				if ((iInput + 2) > end)
				{
					Z_THROW_MSG(zs_parse_error, "bad escape value");
					break;
				}

				int val;
				if (sscanf(iInput, "%02x", &val) != 1)
				{
					Z_THROW_MSG(zs_parse_error, "bad escape value");

					break;
				}
				iInput += 2;
				// Skip linefeed hmmm. why is this here?
				if (val == 10) continue;

				x = val;
				break;
			}

			case '=':
				pValStart = iNew + 1;
				x = 0;
				break;
			case 0:
			case '&':
				add_var = true;
				x = 0;

				break;

			default:
				break;
			}
			if (status)
				break;
			*iNew = x;
			iNew++;
			if (iInput == end)
			{
				*iNew = 0;
				add_var = true;
			}
			if (add_var)
			{
				ctext val = (pValStart ? pValStart : "");
				_query[pVarStart] = val;

				_parser.set_source(pVarStart);
				_parser.parse_url_param(_json_params, pVarStart, val);

				ZDBG("%s=%s\n", pVarStart, val);
				pValStart = 0;
				iNew = pVarStart;
				add_var = false;
			}
		}

	}
	catch (z_except& ex)
	{
		delete pVarStart;
		Z_RETHROW("Error parsing GET request");
	}
	delete pVarStart;



	return status;
}

zf_cc_cgi::zf_cc_cgi(cgi_input * fc)
	: zf_command_context( fc)
{

}
