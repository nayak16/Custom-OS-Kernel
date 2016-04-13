/** @file cleanup.c
 *  @brief Implements a cleanup routine manager
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug Never tested and never used
 */

#include <cleanup.h>
#include <stdlib.h>

typedef struct cleanup_task{
    void (*routine)(void *);
    void *arg;
} cleanup_task_t;

int cleanup_init(cleanup_t *clu){
    if (clu == NULL) return -1;
    return stack_init(clu->stk);
}

int cleanup_push(cleanup_t *clu, void (*routine)(void *), void *arg){
    if (clu == NULL || routine == NULL) return -1;
    cleanup_task_t *clut = malloc(sizeof(cleanup_task_t));
    if (clut == NULL) return -2;
    clut->routine = routine;
    clut->arg = arg;
    return stack_push(clu->stk, (void *)clut);
}

int cleanup_pop(cleanup_t *clu, int execute){
    if (clu == NULL) return -1;
    cleanup_task_t *clut;
    if (stack_pop(clu->stk, (void **)(&clut)) < 0) return -2;
    if (clut == NULL) return -3;
    if (execute != 0){
        (*clut->routine)(clut->arg);
    }
    free(clut);
    return 0;
}

void cleanup_rollback(cleanup_t *clu){
    if (clu == NULL) return;
    int i;
    for (i = 0; i < cleanup_size(clu); i++){
        cleanup_pop(clu, 1);
    }
    return;
}


void cleanup_destroy(cleanup_t *clu){
    if (clu == NULL) return;
    stack_destroy(clu->stk);
}
int cleanup_size(cleanup_t *clu){
    if (clu == NULL) return -1;
    return stack_size(clu->stk);
}
