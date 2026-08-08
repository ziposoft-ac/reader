//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"

#include "global.h"





class TestBlankService : public  Service{
public:
    TestBlankService(){}
    virtual ~TestBlankService() {}
    z_status initialize() override{
        return zs_ok;
    };
    z_status shutdown() override{
        return zs_ok;
    };

};
ZMETA(TestBlankService) {
    ZBASE(Service);

};

ROOT_SERVICE(TestBlankService);
