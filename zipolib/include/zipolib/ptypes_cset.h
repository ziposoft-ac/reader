/*
 *
 *  C++ Portable Types Library (PTypes)
 *  Version 2.0.2  Released 17-May-2004
 *
 *  Copyright (C) 2001-2004 Hovik Melikyan
 *
 *  http://www.melikyan.com/ptypes/
 *
 *
 *   cset class extracted out of ptypes lib
 *
 */
#ifndef ptypes_cset_h
#define ptypes_cset_h
#include "zipolib/zipo.h"


// -------------------------------------------------------------------- //
// --- character set -------------------------------------------------- //
// -------------------------------------------------------------------- //


const int  _csetbits = 256;
const int  _csetbytes = _csetbits / 8;
const int  _csetwords = _csetbytes / sizeof(int);
const char _csetesc = '~';


class cset 
{
protected:
    char data[_csetbytes];

    void assign(const cset& s)                  { memcpy(data, s.data, _csetbytes); }
    void assign(const char* setinit);
    void clear()                                { memset(data, 0, _csetbytes); }
    void fill()                                 { memset(data, -1, _csetbytes); }
    void include(char b)                        { data[U8(b) / 8] |= U8(1 << (U8(b) % 8)); }
    void include(char min, char max);
    void exclude(char b)                        { data[U8(b) / 8] &= U8(~(1 << (U8(b) % 8))); }
    void unite(const cset& s);
    void subtract(const cset& s);
    void intersect(const cset& s);
    void invert();
    bool contains(char b) const                 { return (data[U8(b) / 8] & (1 << (U8(b) % 8))) != 0; }
    bool eq(const cset& s) const                { return memcmp(data, s.data, _csetbytes) == 0; }
    bool le(const cset& s) const;

public:
    cset();
    cset(const cset& s);
    cset(const char* setinit);

    cset& operator=  (const cset& s)            { assign(s); return *this; }
    cset& operator+= (const cset& s)            { unite(s); return *this; }
    cset& operator+= (char b)                   { include(b); return *this; }
    cset  operator+  (const cset& s) const      { cset t = *this; return t += s; }
    cset  operator+  (char b) const             { cset t = *this; return t += b; }
    cset& operator-= (const cset& s)            { subtract(s); return *this; }
    cset& operator-= (char b)                   { exclude(b); return *this; }
    cset  operator-  (const cset& s) const      { cset t = *this; return t -= s; }
    cset  operator-  (char b) const             { cset t = *this; return t -= b; }
    cset& operator*= (const cset& s)            { intersect(s); return *this; }
    cset  operator*  (const cset& s) const      { cset t = *this; return t *= s; }
    cset  operator!  () const                   { cset t = *this; t.invert(); return t; }
    bool  operator== (const cset& s) const      { return eq(s); }
    bool  operator!= (const cset& s) const      { return !eq(s); }
    bool  operator<= (const cset& s) const      { return le(s); }
    bool  operator>= (const cset& s) const      { return s.le(*this); }

    friend cset operator+ (char b, const cset& s);
    friend bool operator& (char b, const cset& s);
    friend void assign(cset& s, const char* setinit);
    friend void clear(cset& s);
    friend void fill(cset& s);
    friend void include(cset& s, char b);
    friend void include(cset& s, char min, char max);
    friend void exclude(cset& s, char b);

};
inline bool operator& (char b, const cset& s)     { return s.contains(b); }
inline cset operator+ (char b, const cset& s)     { return s + b; }

/*
inline void assign(cset& s, const char* setinit)  { s.assign(setinit); }
inline void clear(cset& s)                        { s.clear(); }
inline void fill(cset& s)                         { s.fill(); }
inline void include(cset& s, char b)              { s.include(b); }
inline void include(cset& s, char min, char max)  { s.include(min, max); }
inline void exclude(cset& s, char b)              { s.exclude(b); }
*/


#endif

