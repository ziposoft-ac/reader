//
// Created by ac on 7/30/21.
//
#include "pch.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/eventfd.h>




void testWrite() {
    int _fPipe=0;

    mkfifo("/tmp/debugview",0666);
    _fPipe = ::open("/tmp/debugview",(  O_WRONLY | O_NONBLOCK));
    if (_fPipe<0) {
        if (errno==6) {
            printf("debugview app is not open\n");
            return ;
        }
        printf("could not open debugview pipe err=%d %s\n", errno,strerror(errno));
        return;
    }
    char buf[1024];
    int index=0;
    while (1) {
        int i;
        index++;
        sprintf(buf,"%04d:",index);
        ::write(_fPipe, buf, strlen(buf));
        for (i=0; i<10; i++) {
            sprintf(buf,"%02d-",i);
            int to_write = strlen(buf)+1;
            int written= ::write(_fPipe, buf, to_write);
            if (written!=to_write) {
                printf("\n\ndebugview write:%d\n\n",written);
            }
        }
        ::write(_fPipe, "\n",1);
        fsync(_fPipe);

        usleep(10000);
        //sleep(1);
    }
}


int main(int argc, char* argv[])
{

    if (argc>1) {
        testWrite();
        return 0;
    }
    int _fPipe=0;
    int _fQuitEvent=0;
    z_string _name="/tmp/debugview";
    mkfifo(_name,0666);
    _fPipe = open(_name,( O_RDWR));
    _fQuitEvent=eventfd(0,EFD_CLOEXEC);
    zout<< "DebugView\n\n";

    pollfd fds[]={{_fPipe,POLLIN,0},{_fQuitEvent,POLLIN,0}};
    bool running=true;
    char buf[801];
    size_t n=0;
    do {

        int event=::poll(fds, 2, 10000);
        if(event==1)
        {
            //do {
                n= read(_fPipe,buf,800);
                //printf("%d:",n);
                fwrite(buf,n,1,stdout);

            //}while (n>0);



        }
        if(event==0)
        {
        }
        //zout.flush();

    }while(running);
    return zs_ok;
    return 0;
}