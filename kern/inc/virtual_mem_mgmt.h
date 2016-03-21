#include <page_directory.h>
#include <mem_section.h>

int vmm_map_sections(page_directory_t *pd, mem_section_t *secs, int num_secs);

int vmm_deep_copy(page_directory_t *pd_dest, page_directory_t *pd_src);
