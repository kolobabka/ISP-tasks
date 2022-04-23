#include "test.hpp"

unsigned char *source (int size)
{
    return (unsigned char *)calloc (size, sizeof (unsigned char));
}

void sink (unsigned char b) {}

char globalBuf[10];

int testSink ()
{
    unsigned int tmp;
    unsigned char localBuf[10];
    unsigned char *buf = source (10);
    // case 1
    sink (buf[0]);       // Leak
    sink (localBuf[0]);  // Ok
    // case 2
    localBuf[1] = buf[1];
    sink (localBuf[1]);  // Leak

    // case 3
    localBuf[2] = buf[2];
    localBuf[3] = localBuf[2];
    localBuf[2] = 'a';
    sink (localBuf[3]);  // Leak
    sink (localBuf[2]);  // Ok

    // case 4
    globalBuf[0] = buf[3];
    sink (globalBuf[0]);  // Leak

    // case 5
    globalBuf[1] = buf[4];
    localBuf[0] = globalBuf[1];
    tmp = localBuf[0];
    sink (globalBuf[2]);  // Ok
    sink (tmp & 0xff);    // Leak

#if 1
    sink (tmp >> 1 & 0xff);  // ????
#endif

    free (buf);
    return 0;
}