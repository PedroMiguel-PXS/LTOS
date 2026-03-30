// this runs .bin fle format

#include <stdint.h>

// LTOS Binary format runner
// yes, we execute .bin on LTOS

typedef void(*bin_func_t)(void);

void run_bin(void* address) {
    if (address == 0) return;
    bin_func_t func = (bin_func_t)address;
    func();
}

// on future we will add to verify headers or... module support
// BUT ON FUTURE!
// not now maybe :)
    