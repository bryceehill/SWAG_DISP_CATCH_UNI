/*
 * main.h
 *
 *  Created on: Jun 19, 2023
 *      Author: bhill
 */

#ifndef MAIN_H_
#define MAIN_H_
#define ButtonThreshold 15000

//#pragma DATA_SECTION(mapArray,".myData");
//#pragma DATA_ALIGN(mapArray,1);
//  #pragma RETAIN(mapArray);
//    __no_init const unsigned char  mapArray[] @ 0xFE00;
const char mapArray[98][5] = {{0,0,0,0,0},      // Space
                              {0,0, 0x7D, 0, 0}, // Exclamation Mark
                              {0,0x60, 0, 0x60, 0}, // double quote
                              {0x14,0x7F,0x14,0x7F,0x14},  //Hash Tag
                              {0x12,0x2A, 0x7F, 0x2A, 0x24},  //$
                              {0x62, 0x64, 0x08, 0x13, 0x23}, //%
                              {0x36, 0x49, 0x35, 0x02, 0x05},   //&
                              {0,0,0x60,0,0},                   //'
                              {0,0,0x3E,0x41,0},                 // (
                              {0,0x41,0x3E,0,0},                // )
                              {0x2A, 0x1C, 0x3E, 0x1C, 0x2A},  // *
                              {0x08,0x08,0x3E, 0x08, 0x08},     // +
                              {0,1,0x06,0,0},                   //,
                              {0,8,8,8,0},                          // -
                              {0,3,3,0,0},                      //.
                              {0x02,0x04,0x08,0x10,0x20},      // /
                              {0x3E, 0x45, 0x49,0x51, 0x3E},  //0
                              {0x00, 0x21, 0x7F, 0x01, 0x00}, // 1
                              {0x21, 0x43,0x45, 0x49, 0x31},  // 2
                              {0x42, 0x41, 0x51, 0x69, 0x46},
                              {0x0C, 0x14, 0x24, 0x7F, 0x04},
                              {0x72, 0x51,0x51,0x51,0x4E},
                              {0x1E, 0x29, 0x49, 0x49, 0x06},
                              {0x40, 0x47, 0x48, 0x50, 0x60},
                              {0x36, 0x49, 0x49,0x49, 0x36},
                              {0x30, 0x49, 0x49, 0x4A, 0x3C}, //9
                              {0x00, 0x36, 0x36, 0x00, 0x00}, //:
                              {0x00, 0x35, 0x36, 0x00, 0x00}, //;
                              {0x08, 0x14, 0x22, 0x41, 0x00},//<
                              {0x14,0x14,0x14,0x14,0x14},// =
                              {0x00, 0x41, 0x22, 0x14, 0x08}, //>
                              {0x20, 0x40, 0x45, 0x48, 0x30}, //?
                              {0x26, 0x49, 0x4F, 0x41, 0x3E}, //@
                              {0x3F, 0x44, 0x44, 0x44, 0x3F}, //A
                              {0x7F, 0x49, 0x49, 0x49, 0x36},
                              {0x3E, 0x41, 0x41, 0x41, 0x22},
                              {0x7F, 0x41, 0x41, 0x22, 0x1C},
                              {0x7F, 0x49, 0x49, 0x49, 0x41},
                              {0x7F, 0x48, 0x48, 0x48, 0x40},
                              {0x3E, 0x41, 0x49, 0x49, 0x2F},
                              {0x7F, 0x08, 0x08, 0x08, 0x7F},
                              {0x00, 0x41, 0x7F, 0x41, 0x00},
                              {0x02, 0x01, 0x41, 0x7E, 0x40},
                              {0x7F, 0x08, 0x14, 0x22, 0x41},
                              {0x7F, 0x01, 0x01, 0x01, 0x01},
                              {0x7F, 0x20, 0x10, 0x20, 0x7F},
                              {0x7F, 0x10, 0x08, 0x04, 0x7F}, //N
                              {0x3E, 0x41, 0x41, 0x41, 0x3E}, //O
                              {0x7F, 0x48, 0x48,  0x48, 0x30},//P
                              {0x3E, 0x41, 0x45, 0x42, 0x3D},
                              {0x7F, 0x48, 0x4C, 0x4A, 0x31},
                              {0x31, 0x49, 0x49, 0x49, 0x46},
                              {0x40, 0x40, 0x7F, 0x40, 0x40},
                              {0x7E, 0x01, 0x01, 0x01, 0x7E},
                              {0x7C, 0x02, 0x01, 0x02, 0x7C},
                              {0x7E, 0x01, 0x0E, 0x01, 0x7E},
                              {0x63, 0x14, 0x08, 0x14, 0x63},
                              {0x70, 0x08, 0x07, 0x08, 0x70},
                              {0x43, 0x45, 0x49, 0x51, 0x61},  //Z
                              {0,0, 0x7F,0x41, 0},                 // [
                              {0x20, 0x10, 0x08,0x04,0x02},                   // forward slash
                              {0, 0x41, 0x7F, 0,0},        // ]
                              {0x10, 0x20, 0x40, 0x20, 0x10},            // ^
                              {1,1,1,1,1},     // _
                              {0,0,0x40,0x20},  // `
                              {0x02, 0x15, 0x15,0x15, 0x0F}, // a
                              {0x7F, 0x09, 0x09, 0x09, 0x06}, // b
                              {0x0E, 0x11, 0x11,0x11, 0x0A}, // c
                              {0x06, 0x09, 0x09, 0x09,0x7F}, // d
                              {0x0E, 0x15, 0x15, 0x15, 0x0D}, // e
                              {0x08,0x3F, 0x48, 0x40, 0x20},  // f
                              {0x08, 0x15, 0x15,0x15, 0x0E},    // g
                              {0x7F, 0x08, 0x08, 0x08, 0x07},   //h
                              {0, 0x11, 0x5F, 0x01, 0},         //i
                              {0x02, 0x01, 0x11, 0x5E, 0},       //j
                              {0x7F, 0x04, 0x04, 0x0A, 0x11},  //k
                              {0, 0x41, 0x7F, 0x01, 0x00},  //l
                              {0x1F, 0x10, 0x1F, 0x10, 0x0F}, //m
                              {0x1F, 0x10, 0x10, 0x10, 0x0F}, //n
                              {0x0E, 0x11, 0x11, 0x11, 0x0E}, //o
                              {0x0, 0x1F, 0x14, 0x14, 0x08},  //p
                              {0x00, 0x08, 0x14, 0x14, 0x1F}, //q
                              {0x1F, 0x08, 0x10,0x10, 0x10},    //r
                              { 0x09, 0x15, 0x15, 0x15, 0x12}, // s
                              {0x00, 0x10, 0x7E, 0x11, 0x01},  // t
                              {0x1E, 0x01,0x01, 0x02, 0x1F},  // u
                              {0x1C, 0x02, 0x01, 0x02, 0x1C},   // v
                              {0x1F, 0x02, 0x07, 0x02, 0x1F},  // w
                              {0x11, 0x0A, 0x04, 0x0A, 0x11},  // x
                              {0x19, 0x05, 0x05, 0x05, 0x1E}, //y
                              {0x11, 0x13, 0x15, 0x19, 0x11}, //z
                              {0, 0x08, 0x36, 0x41, 0x00},  // {
                              {0,0,0x7F, 0,0},              // |
                              {0,0x41, 0x36, 0x08, 0},      // }
                              {0x04, 0x08, 0x08, 0x08, 0x10}, // ~
                              {0x7F, 0x41, 0x41, 0x41, 0x7F}, // box
                              {0x7F, 0x7F, 0x7F, 0x7F, 0x7F}, // fill box
                              {0,0,0,0,0}};
                                // needs


#endif /* MAIN_H_ */
