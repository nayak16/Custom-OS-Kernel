/** @file page_directory.h
 *  @brief Specifies interface for page directory
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

#ifndef _PAGE_DIRECTORY_H_
#define _PAGE_DIRECTORY_H_

#define PRESENT_FLAG_BIT 0

#define RW_FLAG_BIT 1

#define MODE_FLAG_BIT 2

#define WRITE_THROUGH_FLAG_BIT 3

#define CACHING_FLAG_BIT 4


typedef struct page_directory {
    int directory[1024];
} page_directory_t;

int pd_init(page_directory_t *pd){
    pd->directory
}

int pd_map(void *virt_addr, void *phys_addr){

}

int pd_initialize_kernel(){
    int i; // the ith page table entry
    // 1024 entries per table -> 4 * 1024 ptes
    for (i = 0; i < ; )
}



#endif /* _PAGE_DIRECTORY_H_ */


