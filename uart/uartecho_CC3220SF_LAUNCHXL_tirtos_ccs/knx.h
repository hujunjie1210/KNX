/*
 * knx.h
 *
 *  Created on: 2019?6?25?
 *      Author: JUNHU2
 */

#ifndef KNX_H_
#define KNX_H_

#define TPUART2_LDATA_START_SERVICE           0x80
#define TPUART2_LDATA_CONT_SERVICE            0x80
#define TPUART2_LDATA_END_SERVICE             0x40

/* FRAME_FORMAT Values */
#define KNX_DATA_STD_FRAME_FORMAT               1
#define KNX_DATA_EXT_FRAME_FORMAT               0

#define KNX_DATA_STD_FRAME_FORMAT_CTRL        0x10
#define KNX_DATA_STD_FRAME_FORMAT_CTRLE       0x00
#define KNX_DATA_STD_FRAME_FORMAT_MASK        0x80
#define KNX_DATA_STD_FRAME_FORMAT_SHIFT         7

/* FRAME_REPEAT Values */
#define KNX_DATA_STD_FRAME_NONREP               1
#define KNX_DATA_STD_FRAME_REP                  0

#define KNX_DATA_STD_FRAME_REP_MASK           0x20
#define KNX_DATA_STD_FRAME_REP_SHIFT            5

/* FRAME_PRIORITY Values */
#define KNX_DATA_STD_FRAME_PRIO_SYSTEM          0
#define KNX_DATA_STD_FRAME_PRIO_HIGH            1
#define KNX_DATA_STD_FRAME_PRIO_ALARM           2
#define KNX_DATA_STD_FRAME_PRIO_NORMAL          3

#define KNX_DATA_STD_FRAME_PRIO_MASK          0x0C
#define KNX_DATA_STD_FRAME_PRIO_SHIFT           2

/* Define ADDRESS Values */
#define KNX_ADDRESS_MASK                      0x00FF
//#define MY_KNX_SOURCE_ADDRESS                 0x0123
//#define MY_KNX_DESTINATION_ADDRESS            0x1234
#define KNX_ADDRESS_SHIFT                       8

/* Define ADDRESS TYPE Values */
#define KNX_ADDRESS_TYPE_INDIVIDUAL             0
#define KNX_ADDRESS_TYPE_GROUP                  1

#define KNX_ADDRESS_TYPE_SHIFT                  7
#define KNX_ADDRESS_TYPE_MASK                 0X80

/* Define LSDU BYTE Values */
#define KNX_LSDU_BYTE                           5
#define KNX_LSDU_BYTE_SHIFT                     4
#define KNX_LSDU_BYTE_MASK                    0X70

/* Define LSDU LENGTH MASK */
#define KNX_LSDU_LENGTH_MASK                  0X0F
#define KNX_LSDU_LENGTH_MASK_EXT              0XFF

/* Define  BUFFER SIZE */
#define FRAME_BUFFER_SIZE                       65

#define KNX_MAX_RESET_RETRIES                   3


#endif /* KNX_H_ */
