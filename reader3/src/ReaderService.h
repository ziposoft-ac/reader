//
// Created by ac on 6/29/26.
//

#ifndef ZIPOSOFT_READERSERVICE_H
#define ZIPOSOFT_READERSERVICE_H
#include "zipolib/z_log.h"

class ReaderService {
public:
    ReaderService() {}
    bool _running=false;
    z_status run();
};

ZMETA_DECL(ReaderService) {

    ZACT(run);

}



#endif //ZIPOSOFT_READERSERVICE_H
