#include "test.hpp"
//----------------------------------------
//----------------------------------------

vert* source () {return (vert*) malloc (sizeof (vert));}
void destructor (vert* ptr) { free (ptr); }
void testSource () {

    vert* x = source ();  //create vertex 

    vert* y = source ();  //create vertex
    vert* z = source ();  //create vertex
    vert* w = source ();  //create vertex
    vert* u = source ();  //create vertex

    y->l_ = w;            // create edge y --> w
    x->r_ = z;            // create edge x --> z
    x->l_ = u;            // remove edge x --> z and create x --> u
    x->l_ = x;            // remove edge x --> u and create x --> x

    vert* tmp = z;        // do nothing 
    y->r_ = tmp;          // create edge y --> z
    z->l_ = y->l_;        // create edge z --> y
    u->l_ = w;            // create edge u --> w 
    destructor (w);       // remove w and all incidental edges 
    tmp->l_ = u;          // create edge z --> u
    destructor (tmp->l_); // remove u and all incidental edges 
    destructor (tmp);     // remove z and all incidental edges
    destructor (y);       // remove z and all incidental edges
    destructor (x);       // remove x
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
