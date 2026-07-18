//
// Created by ac on 8/2/21.
//

#ifndef IPC_REQ_H
#define IPC_REQ_H
#include "pch.h"

enum IpcCode
{
    unprocessed,
    not_connected,
    write_error,
    unknown_error,
    timeout,
    connection_error,
    parse_error,
    aborted,
    busy,
    pending,
    success,

};




/*

export class IpcRequest<T extends object=object> {
    command:string;
    data: T;
    timeout_ms=1000;
}
export class IpcResponse<T extends object=object> {
    code=IpcCode.unprocessed;
    data: T|null = null;
    error: Error|null=null;
    constructor(init?: Partial<IpcResponse<T>>) {
        Object.assign(this, init);
    }
}


*/

#endif //IPC_REQ_H
