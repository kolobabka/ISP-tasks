#include "test.hpp"
#include <iostream>
//----------------------------------------
//----------------------------------------

vert* source () {return (vert*) malloc (sizeof (vert));}

void testSource () {

    vert* x = source (); //create vertex 
    vert* y = source (); //create vertex
    vert* z = source (); //create vertex
    vert* w = source (); //create vertex
    vert* u = source (); //create vertex


    // vert* z = source ();
    x->l_ = y; // create edge between x and y
    x->r_ = z;
    // y->r_ = x->r_;
        // x->l_ = y->r_;
    // y->l_ = x->l_;
}


#if 0
unsigned char* source(int size) {
    
    return (unsigned char*) calloc (size, sizeof (unsigned char));
}

void sink(unsigned char b) {}

int testSink () {

    unsigned int tmp;
    unsigned char localBuf[10];
    unsigned char *buf = source(10);
    
    // case 1
    sink(buf[0]); // Leak
    sink(localBuf[0]); // Ok

    free (buf); 
    return 0;
}
#endif 
