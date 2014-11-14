
#ifndef __MB_UTIL_H__
#define __MB_UTIL_H__
#include <naut_types.h>
#include <paging.h>


uint_t multiboot_get_size(ulong_t mbd);
addr_t multiboot_get_phys_mem(ulong_t mbd);
struct multiboot_info * multiboot_parse(ulong_t mbd, ulong_t magic);
void multiboot_rsv_mem_regions(struct nk_mem_info * mem, ulong_t mbd);

struct multiboot_info {
    char * boot_loader;
    void * sec_hdr_start;
};




#endif
