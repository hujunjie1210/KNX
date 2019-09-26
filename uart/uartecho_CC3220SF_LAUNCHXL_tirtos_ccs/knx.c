/*
 * knx.c
 *
 *  Created on: 2019?6?25?
 *      Author: JUNHU2
 */

#include <knx.h>
#include <stdint.h>
#include <stddef.h>
/* transmission */
/* TPUART2 DATA SERVICES */


int KNXFormatStdFrameTX(char buffer[], int max_buffer_length, int *buffer_length, char rept, char prio, uint16_t sa, uint16_t da, char at, char LSDU_byte, char LSDU[], int LSDU_length )
{
    //  Control Field
           //   Bit  |
           //  ------+---------------------------------------------------------------
           //    7   | Frame Type  - 0x0 for extended frame
           //        |               0x1 for standard frame
           //  ------+---------------------------------------------------------------
           //    6   |  0
           //        |
           //  ------+---------------------------------------------------------------
           //    5   | Repeat Flag - 0x0 repeat
           //        |               0x1 not repeat
           //  ------+---------------------------------------------------------------
           //    4   |  1
           //        |
           //  ------+---------------------------------------------------------------
           //    3   | Priority    - 0x0 system
           //        |               0x1 normal
           //  ------+               0x2 urgent
           //    2   |               0x3 low
           //        |
           //  ------+---------------------------------------------------------------
           //    1   |  0
           //        |
           //  ------+---------------------------------------------------------------
           //    0   |  0
           //        |
           //  ------+---------------------------------------------------------------
           //0x90 0x94 0x98 0x9C  0xB0 0xB4 0xB8 0xBC


         int length=0;
         int i,j;

         if (LSDU_length+7 > max_buffer_length){
             return 0;
         }
         buffer[length] = KNX_DATA_STD_FRAME_FORMAT_CTRL |
                                     ((KNX_DATA_STD_FRAME_FORMAT << KNX_DATA_STD_FRAME_FORMAT_SHIFT) & KNX_DATA_STD_FRAME_FORMAT_MASK)|
                                     ((rept << KNX_DATA_STD_FRAME_REP_SHIFT) & KNX_DATA_STD_FRAME_REP_MASK)|
                                     ((prio << KNX_DATA_STD_FRAME_PRIO_SHIFT) & KNX_DATA_STD_FRAME_PRIO_MASK) ;
         length++;
         buffer[length] = ((sa>>KNX_ADDRESS_SHIFT) & KNX_ADDRESS_MASK);//high sa
         length++;
         buffer[length] = (sa & KNX_ADDRESS_MASK);//low sa
         length++;
         buffer[length] = ((da>>KNX_ADDRESS_SHIFT)& KNX_ADDRESS_MASK);//high da
         length++;
         buffer[length] = (da & KNX_ADDRESS_MASK);//low da
         length++;
         buffer[length] = ((at << KNX_ADDRESS_TYPE_SHIFT) & KNX_ADDRESS_TYPE_MASK)|
                           ((LSDU_byte << KNX_LSDU_BYTE_SHIFT)& KNX_LSDU_BYTE_MASK)|((LSDU_length-1) & KNX_LSDU_LENGTH_MASK);
         length++;
         for(i=0; i<LSDU_length;i++)
         {
             buffer[length]=LSDU[i];
             length++;
         }
         for(j=0;j<length;j++)
         {
             buffer[length]=(~(buffer[length]^buffer[j]));
         }
         length++;
         *buffer_length = length;
         return 1;
}


int KNXFormateEXTFrameTX(char buffer[], int max_buffer_length, int *buffer_length, char rept, char prio, uint16_t sa, uint16_t da, char at, char LSDU_byte, char LSDU[], int LSDU_length )
{
         int length=0;
         int i,j;

         if (LSDU_length+7 > max_buffer_length){
             return 0;
         }
         buffer[length] = KNX_DATA_STD_FRAME_FORMAT_CTRL |
                                     ((KNX_DATA_EXT_FRAME_FORMAT << KNX_DATA_STD_FRAME_FORMAT_SHIFT) & KNX_DATA_STD_FRAME_FORMAT_MASK)|
                                     ((rept << KNX_DATA_STD_FRAME_REP_SHIFT) & KNX_DATA_STD_FRAME_REP_MASK)|
                                     ((prio << KNX_DATA_STD_FRAME_PRIO_SHIFT) & KNX_DATA_STD_FRAME_PRIO_MASK) ;
         length++;

         buffer[length] = KNX_DATA_STD_FRAME_FORMAT_CTRLE |
                                     ((at << KNX_ADDRESS_TYPE_SHIFT) & KNX_ADDRESS_TYPE_MASK)|
                                     ((LSDU_byte << KNX_LSDU_BYTE_SHIFT)& KNX_LSDU_BYTE_MASK)|
                                     ((LSDU_length-1) & KNX_LSDU_LENGTH_MASK);

         length++;

         buffer[length] = ((sa>>KNX_ADDRESS_SHIFT) & KNX_ADDRESS_MASK);//high sa
         length++;

         buffer[length] = (sa & KNX_ADDRESS_MASK);//low sa
         length++;

         buffer[length] = ((da>>KNX_ADDRESS_SHIFT)& KNX_ADDRESS_MASK);//high da
         length++;

         buffer[length] = (da & KNX_ADDRESS_MASK);//low da
         length++;

         buffer[length] = ((LSDU_length-1) & KNX_LSDU_LENGTH_MASK_EXT);
           length++;

         for(i=0; i<LSDU_length;i++)
         {
             buffer[length]=LSDU[i];
             length++;
         }
         for(j=0;j<length;j++)
         {
             buffer[length]=(~(buffer[length]^buffer[j]));
         }
         length++;
         *buffer_length = length;
         return 1;

}


