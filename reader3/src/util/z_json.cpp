//
// Created by ac on 7/24/21.
//

#include "z_json.h"

#include <experimental/array>

json_val::operator bool() const
{
    return true;
}
json_val & json_val::operator = (ctext val)
{
    _v=val;
    return *this;

}

json_val & json_val::operator = (const int val)
{
    _v=val;
    return *this;

}
json_val & json_val::operator = (const bool val)
{
    _v=val;
    return *this;
}
#if 0
int test() {
    json_val x;
    x=3;
    json_obj obj;

    obj["x"]=3;
    obj["p"]="larry";

    keyval lst[]={ { "key",3} };
    json_obj o=lst;
    json_obj k;

    //k={ { "key",3} };
    //json w={ { "key",3} };


    return 0;
}
#endif