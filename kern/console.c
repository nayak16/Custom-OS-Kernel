/** @file console.c
 *  @brief implements console driver.
 *
 *  Implements the console driver. One should first note the difference between
 *  a logical cursor and hardware cursor. The hardware cursor is the value read
 *  from hardware and is where the cursor is drawn; since we want to be able to
 *  hide and show our cursor, we also have the notion of logical cursor, which
 *  is translated from the hardware cursor and points to the location where the
 *  next character is to be written. This allows us to ignore any logic about
 *  whether or not to hide or show the cursor, and all we must do is translate
 *  from hardware cursor to logical cursor before we write a character. This
 *  relationship between hardware and logical cursor are described by
 *  C_HIDDEN_OFFSET which is a constant offset between hardware and logical
 *  cursor if the cursor is hidden. As noted below, the C_HIDDEN_OFFSET must be
 *  greater than or equal to C_CONSOLE_SIZE so that no overlap occurs. Some
 *  other implementation specifics to note: backspacing is treated as setting
 *  the previous character to C_DEFAULT_TERMINAL_CHAR and its color to
 *  C_DEFAULT_TERMINAL_COLOR which happens to be a space character with black
 *  background and light gray foreground. Any "empty" slots are also treated
 *  as the default color and character, so backspacing into a previous line will
 *  always set the column to (CONSOLE_WIDTH - 1), not to where the last
 *  non space character is. New lines will not replace the rest of the line with
 *  spaces, since in most computers a "Enter" results in a return carriage
 *  followed by a new line. Terminal color, as much as it pains me, is stored in
 *  a global variable.
 *
 *  @author Christopher Wei (cjwei)
 *  @bug No known bugs
 */

#include <stdio.h>

#include <console.h>
/* inb outb */
#include <x86/asm.h>
/* memmove */
#include <string.h>
/* byte width and mask */
#include <constants.h>

#include "contracts.h"

/** @brief offset of the hardware cursor to logical cursor.
 *  Note this value must be greater than or equal to C_CONSOLE_SIZE
 */
#define C_HIDDEN_OFFSET (CONSOLE_WIDTH*CONSOLE_HEIGHT)
/** @brief the total number of characters in our console */
#define C_CONSOLE_SIZE (CONSOLE_WIDTH*CONSOLE_HEIGHT)
/** @brief the number of bytes each character takes up in memory */
#define C_CHAR_WIDTH 2

/** @brief new line character */
#define CHAR_NEW_LINE '\n'
/** @brief return carriage character */
#define CHAR_RETURN_CARRIAGE '\r'
/** @brief backspace character */
#define CHAR_BACKSPACE '\b'
/** @brief space character */
#define CHAR_SPACE ' '

/** @brief the default terminal color */
#define C_DEFAULT_TERMINAL_COLOR (FGND_LGRAY | BGND_BLACK)
/** @brief the default terminal character used to fill blank spaces
 *  Not neccesarily CHAR_SPACE... what if you wanted to fill blank spaces
 *  with squiggles ~
 */
#define C_DEFAULT_TERMINAL_CHAR ' '

/** @brief denotes hidden hardware cursor */
#define GET_HPOS_STATUS_HIDDEN 1
/** @brief denotes visible hardware cursor */
#define GET_HPOS_STATUS_SHOWN 0

/** @brief denotes out of bounds row and column */
#define SET_CURSOR_OUT_OF_BOUNDS -1
/** @brief denotes cursor row col converts to invalid hardware cursor */
#define SET_CURSOR_INVALID_HPOS -2
/**
 *  GLOBAL VARIABLES
 */

/** @brief stores the current color that should be written to the terminal */
int g_terminal_color = C_DEFAULT_TERMINAL_COLOR;

/**
 *  HELPER FUNCTIONS
 */

/** @brief finds if given row, column pair is a valid console position
 *
 *  @param row The row to check
 *  @param col The column to check
 *  @return 0 on success, or a negative status code on failure
 */
int in_bounds(int row, int col){
  if (row < 0 || row >= CONSOLE_HEIGHT || col < 0 || col >= CONSOLE_WIDTH){
    return -1;
  }
  return 0;
}


/** @brief gets current status of hardware cursor
 *
 *  @param hpos Hardware cursor position
 *  @return status for hardware cursor or -1 on invalid hpos
 */
int get_hpos_status(int hpos){
  if (hpos >= C_HIDDEN_OFFSET &&
      hpos < C_CONSOLE_SIZE + C_HIDDEN_OFFSET){
    return GET_HPOS_STATUS_HIDDEN;
  }
  if (hpos >= 0 && hpos < C_CONSOLE_SIZE){
    return GET_HPOS_STATUS_SHOWN;
  }
  return -1;
}


/** @brief returns whether the given logical pos falls in the correct range
 *
 *  @param lpos The logical position to check
 *  @return 0 if valid position else 1
 */
int is_valid_lpos(int lpos){
  if (0 <= lpos && lpos < C_CONSOLE_SIZE){
    return 0;
  }
  return 1;
}


/** @brief returns whether the given hardware pos falls in the correct range
 *
 *  @param hpos The hardware position to check
 *  @return 0 if valid position else 1
 */
int is_valid_hpos(int hpos){
  return (get_hpos_status(hpos) != -1);
}


/** @brief finds the current hardware cursor location
 *
 *  It should be noted that the hardware cursor is the position that the
 *  cursor character '_' is drawn. If said cursor is hidden, then the hardware
 *  position points to an off screen position, otherwise the hardware and
 *  logical cursor are the sae.
 *
 *  Ensures a valid hardware position is returned.
 *
 *  @param Take no parameters
 *  @return The address that is pointed to by the hardware cursor
 */
int get_hardware_cursor(){
  char lsb, msb;
  int lsb_i, msb_i, hpos;

  /* read in cursor position from IO */
  outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
  lsb = inb(CRTC_DATA_REG);
  outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
  msb = inb(CRTC_DATA_REG);

  /* convert char to int and remove bit extensions */
  lsb_i = (int)lsb & C_BYTE_MASK;
  msb_i = (int)msb & C_BYTE_MASK;

  /* convert msb/lsb to cursor location */
  hpos = (msb_i << C_BYTE_WIDTH) | lsb_i;
  ENSURES(is_valid_hpos(hpos) == 0);
  return hpos;
}


/** @brief sets the cursor to hpos
 *
 *  Requires input hpos is a valid hardware position
 *
 *  @param hpos The hardware position to set to
 *  @return Void
 */
void set_hardware_cursor(int hpos){
  REQUIRES(is_valid_hpos(hpos) == 0);

  char lsb, msb;
  lsb = (char)(hpos & C_BYTE_MASK);
  msb = (char)((hpos >> C_BYTE_WIDTH) & C_BYTE_MASK);

  /* write lsb/msb pair */
  outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
  outb(CRTC_DATA_REG, lsb);
  outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
  outb(CRTC_DATA_REG, msb);
}


/** @brief converts and stores logical position to row nad col
 *
 *  Requires valid logical position and non null input pointers
 *
 *  @param lpos The logical position to convert
 *  @param row The address of row variable to store to
 *  @param col The address of column variable to store to
 *  @return Void
 */
void lpos_to_row_col(int lpos, int *row, int *col){
  //TODO: more comprehensive pointer checking
  REQUIRES(row != NULL && col != NULL);
  REQUIRES(is_valid_lpos(lpos) == 0);
  *row = (lpos / CONSOLE_WIDTH);
  *col = (lpos % CONSOLE_WIDTH);
  ENSURES(in_bounds(*row, *col) == 0);
}


/** @brief converts a row and column to a logical position
 *
 *  Requires row and column are in bounds, ensures that lpos returned is valid
 *
 *  @param row The row of row col pair
 *  @param col the column of row col pair
 *  @return The logical cursor position that corresponds to row, col
 */
int row_col_to_lpos(int row, int col){
  REQUIRES(in_bounds(row, col) == 0);
  int lpos;
  lpos = (row * CONSOLE_WIDTH) + col;
  ENSURES(is_valid_lpos(lpos) == 0);
  return lpos;
}


/** @brief converts hardware cursor to logical cursor
 *
 *  @param hpos The hardware cursor
 *  @return The logical cursor position from hardware cursor or
 *          -1 if invalid hardware cursor
 */
int hardware_to_logical(int hpos){
  switch(get_hpos_status(hpos)){
    case GET_HPOS_STATUS_SHOWN:
      return hpos;
    case GET_HPOS_STATUS_HIDDEN:
      return hpos - C_HIDDEN_OFFSET;
    default:
      return -1;
  }
}


/** @brief converts logical cursor to hardware cursor
 *
 *  @param lpos The logical cursor
 *  @param hidden The status returned from get_hpos_status
 *  @return The hardware cursor position from logical cursor or
 *          -1 if invalid logical cursor
 */
int logical_to_hardware(int lpos, int hidden){
  switch(hidden){
    case GET_HPOS_STATUS_SHOWN:
      return lpos;
    case GET_HPOS_STATUS_HIDDEN:
      return lpos + C_HIDDEN_OFFSET;
    default:
      return -1;
  }
}


/** @brief gets the address of the memory corresponding to the character at
 *         the given row and column
 *
 *  Requires (row,col) are in bounds
 *
 *  @param row The row to select
 *  @param col The column to select
 *  @return Pointer to the address storing the character at row, col
 */
char *get_console_char(int row, int col){
  REQUIRES(in_bounds(row, col) == 0);
  //TODO: make a nice ensures contract
  return ((char *)CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col));
}


/** @brief gets the address of the memory corresponding to the color at
 *         the given row and column
 *
 *  Requires (row,col) are in bounds
 *
 *  @param row The row to select
 *  @param col The column to select
 *  @return Pointer to the address storing the color at row, col
 */
char *get_console_color(int row, int col){
  REQUIRES(in_bounds(row, col) == 0);
  //TODO: make a nice ensures contract
  return ((char *)CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col)) + 1;
}

/** @brief scrolls the console up by one row, overwriting the 0th row and
 *         clearing out the last row
 *
 *  @return Void
 */
void scroll_console(){
  int n, i;
  void *src, *dest;
  /* number of byte to copy */
  n = CONSOLE_WIDTH * (CONSOLE_HEIGHT-1) * C_CHAR_WIDTH;
  /* src = row 1, col 0, address to first byte */
  src = (void *)get_console_char(1, 0);
  /* dest = row 0, col 0, address to first byte */
  dest = (void *)get_console_char(0, 0);
  /* move chunk of memory */
  memmove(dest, src, n);
  /* clear out last row */
  for (i = 0; i < CONSOLE_WIDTH; i++){
    *(get_console_char(CONSOLE_HEIGHT-1, i)) = C_DEFAULT_TERMINAL_CHAR;
    *(get_console_color(CONSOLE_HEIGHT-1, i)) = C_DEFAULT_TERMINAL_COLOR;
  }
}


/**
 *  IMPLEMENTATION
 */

int putbyte( char ch ){
  int row, col;
  int color;
  /* get current cursor location and current global terminal color */
  get_cursor(&row, &col);
  get_term_color(&color);
  switch (ch){
    case CHAR_NEW_LINE:
      /* check if we need to scroll because we are out of rows */
      if (row + 1 == CONSOLE_HEIGHT){
        /* move all rows up and set to beginning of current row */
        scroll_console();
        set_cursor(row, 0);
      } else {
        /* advance to next row */
        set_cursor(row+1, 0);
      }
      break;
    case CHAR_RETURN_CARRIAGE:
      /* go to first column of current row */
      set_cursor(row, 0);
      break;
    case CHAR_BACKSPACE:
      /* if we are at 0th column, handle backspacing to previous line */
      if (col == 0){
        /* so long as we can move backwards, delete previous row's last char */
        if (row != 0){
          draw_char(row-1, CONSOLE_WIDTH-1, C_DEFAULT_TERMINAL_CHAR,
            C_DEFAULT_TERMINAL_COLOR);
          set_cursor(row-1, CONSOLE_WIDTH-1);
        }
        /* otherwise we are at [0,0] so do nothing */
      } else {
        /* simply remove previous character and update cursor location */
        draw_char(row, col-1, C_DEFAULT_TERMINAL_CHAR,
          C_DEFAULT_TERMINAL_COLOR);
        set_cursor(row, col-1);
      }
      break;
    default:
      /* draw character at cursor */
      draw_char(row, col, ch, (char)color);
      /* based on row and column, update cursor appropriately*/
      if (col == CONSOLE_WIDTH-1){
        /* if cursor at last slot in a line */
        if (row == CONSOLE_HEIGHT-1){
          /* if cursor at last row, make a new line*/
          set_cursor(row, 0);
          scroll_console();
        } else {
          /* move to next line */
          set_cursor(row+1, 0);
        }
      } else {
        /* otherwise move to next column */
        set_cursor(row, col+1);
      }
      break;
  }
  /* cast to int, remove any bit extensions and return */
  return ((int)ch) & C_BYTE_MASK;
}


void putbytes( const char *s, int len ){
  int i;
  /* 0 length or null strings have no effect */
  if (s == NULL || len == 0){
    return;
  }
  /* put len bytes into console */
  for (i = 0; i < len; i++){
    putbyte(s[i]);
  }
}


void draw_char( int row, int col, int ch, int color ){
  //TODO: check for valid character or color
  /* out of bounds draw calls have no effect */
  if (in_bounds(row, col) == 0){
    /* get console addresses for appropriate character and set them */
    char *console_char_base = get_console_char(row, col);
    char *console_color_base = get_console_color(row, col);
    *console_char_base = ch;
    *console_color_base = (char)color;
  }
}


char get_char( int row, int col ){
  /* since we must return a char, row and col must be in bounds */
  ASSERT(in_bounds(row, col) == 0);
  return *(get_console_char(row, col));
}


int set_term_color( int color ){
  g_terminal_color = color;
  return 0;
}


void get_term_color( int *color ){
  ASSERT(color != NULL);
  *color = g_terminal_color;
}


int set_cursor( int row, int col ){
  int lpos, hpos, hidden;
  /* check for invalid row, col */
  if (in_bounds(row, col) != 0){
    return SET_CURSOR_OUT_OF_BOUNDS;
  }
  /* calculate logical position from row col */
  lpos = row_col_to_lpos(row, col);
  hidden = get_hpos_status(get_hardware_cursor());
  /* convert logical to hardware cursor */
  hpos = logical_to_hardware(lpos, hidden);
  if (hpos == -1){
    return SET_CURSOR_INVALID_HPOS;
  }
  set_hardware_cursor(hpos);
  return 0;
}


int get_cursor( int *row, int *col ){
  if (row == NULL || col == NULL) return -1;
  int hpos, lpos;
  /* get the current hardware cursor and convert to logical cursor */
  hpos = get_hardware_cursor();
  lpos = hardware_to_logical(hpos);
  if (lpos == -1) return -1;
  /* convert logical cursor to a row column pair and save into row col addr */
  lpos_to_row_col(lpos, row, col);
  return 0;
}


void hide_cursor(){
  int hpos;
  hpos = get_hardware_cursor();
  /* if not hidden, then hide cursor */
  if (get_hpos_status(hpos) == GET_HPOS_STATUS_SHOWN){
    hpos += C_HIDDEN_OFFSET;
    set_hardware_cursor(hpos);
  }
  return;
}


void show_cursor(){
  int hpos;
  hpos = get_hardware_cursor();
  /* if not shown, then show cursor */
  if (get_hpos_status(hpos) == GET_HPOS_STATUS_HIDDEN){
    hpos -= C_HIDDEN_OFFSET;
    set_hardware_cursor(hpos);
  }
  return;
}


void clear_console(){
  int row, col;
  /* for every row and column, clear to default state */
  for(row = 0; row < CONSOLE_HEIGHT; row++){
    for(col = 0; col < CONSOLE_WIDTH; col++){
      *get_console_char(row, col) = C_DEFAULT_TERMINAL_CHAR;
      *get_console_color(row, col) = C_DEFAULT_TERMINAL_COLOR;
    }
  }
  /* set logical cursor to beginning of console */
  set_cursor(0,0);
}
