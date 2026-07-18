//
// Created by ac on 7/24/21.
//

#ifndef ZIPOSOFT_Z_JSON_H
#define ZIPOSOFT_Z_JSON_H
#include "pch.h"

#include <variant>
#include <type_traits>
#include <utility>
#include <array>
#include <memory>

class json_obj ;
class json_arr ;


class json_val
{
    typedef std::variant<std::monostate,int,bool, double,std::string,json_obj*,json_arr*> _vT;
    _vT _v;
    public:
    json_val() {}
    json_val(const int val) {_v=val; };
    json_val & operator = (const int inval);
    json_val & operator = (ctext val);
    json_val & operator = (bool val);
    operator int() const ;
    operator bool() const ;
    operator double() const ;


};
struct keyval
{
    std::string key;
    json_val val;
};



class json_obj : public std::map<std::string,z_json_val> {
    json_obj ( const json_obj & ) = delete;
public:
    json_val& operator[](ctext key);
    json_obj(keyval list[]);
    json_obj(){};
    json_val & operator = (keyval* list);
};
class json : public json_obj
{
    json(keyval list[]);

};

class json_arr {
    z_obj_vector<json_val> _array;

public:

};


#endif //ZIPOSOFT_Z_JSON_H
