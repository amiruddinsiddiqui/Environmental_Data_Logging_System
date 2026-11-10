/*
 * dma.h
 *
 *  Created on: Nov 10, 2026
 *      Author: asus
 */

#ifndef PERIPHERALS_DMA_DMA_H_
#define PERIPHERALS_DMA_DMA_H_

extern volatile uint8_t dma_rx_done;
void dma_init(void);
void dma_config(uint32_t src_addr, uint32_t dest_addr, uint8_t size);


#endif /* PERIPHERALS_DMA_DMA_H_ */
