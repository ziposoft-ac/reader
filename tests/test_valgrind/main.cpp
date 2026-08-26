


#include <thread>
#include <limits>
#include <vector>
#include <deque>
#include <string>
#include <cstring>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <iostream>


class X {
    int a=1;
};


X x;


int main() {
    srand(time(NULL));
    tzset();

    char* buff=new char[1024];

    delete [] buff;

    return 0;
}