/*
 * debug.h
 *
 *  Created on: Nov 15, 2026
 *      Author: asus
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_


#include <stdio.h>



#define DEBUG   1    // DEBUG = 1: LOG() prints messages through printf() (USART2)
				     // DEBUG = 0: LOG() does nothing; logging calls are removed during preprocessing

#if DEBUG
    #define LOG(...) printf(__VA_ARGS__) // LOG can accept any arguments and passes them to printf
#else
    #define LOG(...) ((void)0)
#endif



#endif /* INC_DEBUG_H_ */
