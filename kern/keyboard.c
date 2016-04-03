/** @file keyboard.h
 *  @brief Defines interface for a keyboard manager
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs
 */

#include <keyboard.h>
#include <circ_buffer.h>
#include <stdlib.h>
#include <string.h>

#include <console.h>

#include <simics.h>

/** @brief Takes a character c and prints it to the console
 *  @param c The character to be printed
 *  @return Void
 */
void print_to_console(keyboard_t *k, char c){
    if (k == NULL) return;
    uint32_t count;
    if (circ_buf_count(k->buf, &count) < 0) return;
    switch(c){
        case '\b':
            /* if nothing avaliable to backspace, ignore
             * and don't print to console */
            if (count == 0){
                break;
            } else {
                /* we removed a character from the buffer, so now
                 * we need to remove a character from the console */
                putbyte(c);
            }
            break;
        default:
            putbyte(c);
            break;
    }
}

int write_to_buffer(keyboard_t *k, char c){
    uint32_t count;
    /* write to the keyboard's buffer */
    switch(c){
        case '\b':
            circ_buf_count(k->buf, &count);
            /* ignore backspaces on empty buffer */
            if (count == 0) break;
            /* remove one character */
            if (circ_buf_delete_front(k->buf) < 0){
                return -2;
            }
            break;
        default:
            /* write character to buffer */
            if (circ_buf_write(k->buf, (void *)(unsigned int)c) < 0){
                return -3;
            }
            break;
    }
    return 0;
}

int keyboard_init(keyboard_t *k, uint32_t len){
    if (k == NULL || len == 0) return -1;
    circ_buf_t *cbuf = malloc(sizeof(circ_buf_t));
    if (circ_buf_init(cbuf, len) < 0) return -2;
    if (sem_init(&(k->sem), 0) < 0) return -3;
    if (mutex_init(&(k->m)) < 0) return -4;
    k->buf = cbuf;
    return 0;
}

void keyboard_destroy(keyboard_t *k){
    circ_buf_destroy(k->buf);
    mutex_destroy(&(k->m));
    sem_destroy(&(k->sem));
    free(k->buf);
}

int keyboard_write(keyboard_t *k, uint32_t val){
    if (k == NULL) return -1;
    mutex_lock(&(k->m));

    /* see if we should echo to console */
    int num_resources;
    if (sem_get_value(&(k->sem), &num_resources) < 0){
        mutex_unlock(&(k->m));
        return -3;
    }

    /* atleast one readline is pending - echo val to console */
    if (num_resources < 0){
        print_to_console(k, (char)val);
    }

    if (write_to_buffer(k, (char)val) < 0){
        mutex_unlock(&(k->m));
        return -4;
    }

    /* if the given character is a new line, increment the number
     * of resources avaliable */
    if ((char)val == '\n') sem_signal(&k->sem);
    mutex_unlock(&(k->m));
    return 0;
}

int keyboard_read(keyboard_t *k, int len, char *buf){
    if (k == NULL || len <= 0 || buf == NULL) return -1;
    sem_wait(&k->sem);
    mutex_lock(&(k->m));
    /* read the line in from circ buf */
    char temp_buf[len];
    uint32_t i, val;
    for (i = 0; i < len; i++){
        if (circ_buf_read(k->buf, (void **)&val) < 0){
            mutex_unlock(&(k->m));
            return -2;
        }
        if ((char)val == '\n'){
            if (i == 0){
                temp_buf[i] = (char)val;
                i = 1;
            }
            break;
        }
        temp_buf[i] = (char)val;
    }

    memcpy(buf, temp_buf, i*sizeof(char));
    mutex_unlock(&(k->m));
    return i;
}

int keyboard_buffer_size(keyboard_t *k, uint32_t *len){
    if (k == NULL || len == NULL) return -1;
    return circ_buf_size(k->buf, len);
}
