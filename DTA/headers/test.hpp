#ifndef __TEST_HPP__
//----------------------------------------
//----------------------------------------
#include <stdlib.h>
//----------------------------------------
//----------------------------------------
struct vert {

    vert* l_;
    vert* r_;
};
//----------------------------------------
//----------------------------------------
vert* source ();
void destructor (vert* ptr);
void testSource ();
#if 0
unsigned char* source(int size);
void sink(unsigned char b);
int testSink ();
#endif
//----------------------------------------
//----------------------------------------
#endif