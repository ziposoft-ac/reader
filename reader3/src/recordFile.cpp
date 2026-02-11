//
// Created by ac on 2/2/26.
//

#include "recordFile.h"
#include <filesystem>

z_status RecordFile::writeRfidRead(RfidRead* read,ctext epc) {
    if(!_file.is_open())
        return zs_not_open;
    _file << read->_index << ','<<   read->get_ms_epoch() <<
        ','<< read->_antNum << ','
        <<  read->_rssi << ','  << epc<< '\n';
    return zs_ok;

}






z_status RecordFile::open_new(ctext path,ctext type,z_time ts) {

    if (is_open()) {
        return Z_ERROR_MSG(zs_already_open,"Record file already open");

    }
    if (!path) {
        return Z_ERROR_MSG(zs_bad_parameter,"Record path not set");

    }
    _path=path;
    _type=type;
    std::error_code ec;
    std::filesystem::create_directory(_path.c_str(),ec);
    if (ec.value()) {
        Z_ERROR_MSG(zs_bad_parameter,"Could not create directory record file!");
    }

    _time_opened = ts;

    _time_opened.string_format(_time_opened_str, "%Y_%m_%d_%H_%M_%S",true);
    _name = "live-"+_time_opened_str+ "-"+  std::to_string(_time_opened.get_t())+"-live-" +_type  + "-.txt";
    _fullpath=_path+"/"+_name;

    z_status s = _file.open(_fullpath, "ab");
    if (s)
    {
        Z_ERROR_MSG(s,"Could not open record file: \"%s\" : %s ",_fullpath.c_str(),zs_get_status_text(s));
    }

    return s;
}

z_status RecordFile::close_copy() {
    if (!_file.is_open()) {
        //return Z_ERROR_MSG(zs_not_open,"Record file not open");
        return zs_not_open;
    }
    _file.close();
    z_time time_closed = z_time::get_now();



    z_string new_name =_path+"/reads-"+_time_opened_str +"-" +  std::to_string(_time_opened.get_t()) +"-" + std::to_string(time_closed.get_t())+"-"  +_type  + "-.txt";
    try {
        // Copy the file
        std::error_code ec;
        std::filesystem::rename(_fullpath.c_str(), new_name.c_str(),ec);
        if (ec.value()) {
            ZT("Error moving file %s",ec.message().c_str());
        }

    } catch (const std::filesystem::filesystem_error& e) {
            ZT("Error moving file %s",ec.message().c_str());

    }
    return zs_ok;
}