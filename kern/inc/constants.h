/** @file constants.h
 *  @brief some constants about bits and bytes
 *  @author Christopher Wei (cjwei)
 */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_


/** @brief lower byte mask */
#define C_BYTE_MASK 0x000000FF
/** @brief lower 2 byte mask */
#define C_L2B_MASK 0x0000FFFF
/** @brief upper 2 byte mask */
#define C_U2B_MASK 0xFFFF0000
/** @brief MSB 20 bit mask */
#define MSB_20_MASK 0xFFFFF000
/** @brief LSB 12 bit mask */
#define LSB_12_MASK 0x00000FFF
/** @brief number of bits in a byte */
#define C_BYTE_WIDTH 8
/** @brief number of bits in 2 bytes */
#define C_2BYTE_WIDTH 16

/* Flag Designations */
/** @brief denotes a set flag */
#define SET 1
/** @brief denotes an unset flag */
#define UNSET 0
/** @brief denotes a arbitrary flag */
#define DONT_CARE 0

#endif /* _CONSTANTS_H_ */
