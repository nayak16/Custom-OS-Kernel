/** @file frame_manager.c
 *  @brief implementation for a vm frame manager
 *
 *  @author Aatish Nayak (aatishn)
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs.
 */

typedef struct frame{
    int foo;
} frame_t;

typedef struct frame_manager{
    /** @brief Array of frames */
    frame_t *frames;
    /** @brief Number of frames in frames */
    int frame_count;
} frame_manager_t;


int frame_manager_init(frame_manager_t *fm){
    if (fm == NULL) return -1;
    int n = machine_phys_frames();
    fm->frame_count = n;
    fm->frames = malloc(n * sizeof(frame_t));    
    return 0;
}