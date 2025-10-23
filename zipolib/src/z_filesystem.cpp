#include "pch.h"
#include "zipolib/z_filesystem.h"

const char unix_path_slash='/';
const char win_path_slash='\\';
z_status z_filesys_get_path_parts(z_string& fullpath,z_string* path_out,z_string* name_out,z_string* ext_out)
{

	//TODO make this UTF8 compatible
	size_t len=fullpath.length();
	size_t i;
	for(i=0;i<len;i++)
		if(fullpath[i]=='\\')
			fullpath[i]='/';


	size_t forslash=fullpath.rfind(unix_path_slash);
	if(forslash==z_string::npos)  forslash=0;

	size_t backslash=fullpath.rfind(win_path_slash);
	if(backslash==z_string::npos)  backslash=0;
	
	size_t slash=(backslash>forslash?backslash:forslash);

	if(slash)
		slash++;


	if(path_out)
		path_out->assign(fullpath,0,slash);


	size_t dot=fullpath.rfind(	'.');

	if((dot==-1)||(dot<slash))
	{
		//there is no extension
		dot=fullpath.length();
		if(ext_out)
			*ext_out="";
	
	}
	else
	{
		if(ext_out)
			ext_out->assign(fullpath,dot+1,fullpath.length()-dot);
	}
	

	if(name_out)
		name_out->assign(fullpath,slash,dot-slash);
	if(path_out)
	path_out->assign(fullpath,0,slash);


	return zs_ok;
}

z_status z_filesys_path_append(z_string& path, z_string append)
{
	path << "/" << append;
	return zs_ok;

}

z_status z_filesys_getcwd(z_string& path)
{
	char* current_dir=z_new char[1024];
	if(z_filesys_get_current_dir(current_dir, 1024))
	{
		return Z_ERROR_MSG(zs_unknown_error,"Could not get current directory");
	}
	else
		path= current_dir;
	delete []current_dir;

	return zs_ok;
}
z_status z_filesys_setcwd(z_string& path)
{
	int s= z_directory_change(path.c_str(),false);
	if(s)
		return zs_could_not_open_dir;
	return zs_ok;
}
ctext z_directory::get_path()
{
	return _path;

}


z_status z_directory::set_to_cwdir()
{
	return z_filesys_getcwd(_path);

}
z_status z_directory::open(ctext path,bool create)
{
	z_status status;
	if(_hDir!=0)
			return Z_ERROR_MSG(zs_already_open,"z_directory already open");

	status=z_dir_open(path,&_hDir); //current dir
	if(status==zs_success)
	{
		_path=path;
		return status;
	}

	if(create)
	{
		status=z_dir_create(path,0); //current dir
		if(status!=zs_success)
			return Z_ERROR_MSG(zs_could_not_open_dir,"Could not create directory \"%s\"",path);
		status=z_dir_open(path,&_hDir); //current dir
		if(status==zs_success)
			return status;
		_path=path;
	}
	return Z_ERROR_MSG(zs_could_not_open_dir,"Could not open directory \"%s\"",path);


}
void z_directory::close()
{
	z_dir_close(_hDir);
	_hDir=0;

}
z_directory::z_directory()
{
	_hDir=0;

}
z_directory::~z_directory()
{
	if(_hDir)
		z_dir_close(_hDir);

}
z_status z_directory::traverse_tree()
{
	z_status status;
	if(_hDir==0)
	{
		if(open(".")) 
			return Z_ERROR_MSG(zs_could_not_open_dir,"Could not open local directory");
	}
	status=traverse_tree_recurse(_path,_hDir);
	return status;

}
z_status z_directory::callback_file(ctext name,ctext fullpath)
{

	zout << name <<'\n';
	return zs_ok;
};
z_status z_directory::callback_dir(ctext name,ctext fullpath)
{
	zout << name <<'\n';

	return zs_ok;

};
z_status z_directory::traverse_tree_recurse(z_string &path, z_directory_h dir)
{
	int type;
	z_string filename;
	ctext pname;
	z_status status;
	while(z_dir_get_next(dir,&pname,Z_DIR_TYPE_FILE|Z_DIR_TYPE_DIR,&type)==zs_ok)
	{
		z_string subpath = path;
		subpath <<'/' << pname;

		if(type== Z_DIR_TYPE_DIR)
		{
			status = callback_dir(pname, subpath);
			if (status == zs_skipped)
				continue;
			z_directory_h subdir;
			status=z_dir_open(subpath,&subdir);
			if (status == zs_ok)
			{
				status=traverse_tree_recurse(subpath,subdir);
				z_dir_close(subdir);
			}
			if (status == zs_quit)
				return status;
			//Just skip it if we cant open it
			//else return status;

		}
		else
		{
			status = callback_file(pname, subpath);
			if (status == zs_quit)
				return zs_quit;
		}
	}
	
	return zs_success;

}


z_status z_directory::get_files_by_extension(ctext ext,z_strlist &list,  int type_mask)
{
	
	if(_hDir==0)
	{

		if(open(".")) 
			return Z_ERROR_MSG(zs_could_not_open_dir,"Could not open local directory");

	}
	z_string filename;
	ctext pname;
	list.clear();
	int type;
	while(z_dir_get_next(_hDir,&pname, type_mask,&type)==0)
	{
		filename=pname;
		size_t found=filename.find_last_of('.');
		if(found==z_string::npos)
			continue;

		if (filename.substr(found+1)==ext)
		{
			list << filename;
		}
	}
	close();
	return zs_success;

}

