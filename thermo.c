#include "thermo.h"

const unsigned int freq_tbl [] = {
  218,219, //?????????
};

int ftot(unsigned int freq){
    int i;

    if(freq < freq_tbl[0]){
        return -32768;
    }
    for(
            i = 0;
            i < 1601 &&
            freq > freq_tbl[i];
            i++
            );
    return i - 500;
}
