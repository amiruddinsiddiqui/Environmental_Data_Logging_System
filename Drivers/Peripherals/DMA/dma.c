/*
 * dma.c
 *
 *  Created on: Nov 10, 2026
 *      Author: asus
 */


#include "stm32f4xx.h"
#include "dma.h"
#include "stdint.h"

// stream 0 and channel 1
void dma_init(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	DMA1_Stream0->CR &= ~(DMA_SxCR_DIR);  // 00:-> direction of data-> peripheral to memory

    // normal mode
    DMA1_Stream0->CR &= ~DMA_SxCR_CIRC;
	DMA1_Stream0->CR |= DMA_SxCR_MINC;  // memory increment

	DMA1_Stream0->CR &= ~DMA_SxCR_PSIZE; // data size-> 8bit
	DMA1_Stream0->CR &= ~DMA_SxCR_MSIZE; // data size-> 8bit

	DMA1_Stream0->CR |= DMA_SxCR_TCIE; // TX interrupt enable

	DMA1_Stream0->CR |= DMA_SxCR_CHSEL_0; // 001:-> channel 1

	NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

void dma_config(uint32_t src_addr, uint32_t dest_addr, uint8_t size) {
	DMA1_Stream0->CR &= ~DMA_SxCR_EN;

	while(DMA1_Stream0->CR & DMA_SxCR_EN);

    	// clear previous DMA flags
		DMA1->LIFCR = DMA_LIFCR_CFEIF0 | DMA_LIFCR_CDMEIF0 |
				  	  DMA_LIFCR_CTEIF0 |
					  DMA_LIFCR_CHTIF0 |
					  DMA_LIFCR_CTCIF0;

	DMA1_Stream0->NDTR = size; // size of the transfer

	DMA1_Stream0->PAR = src_addr; // peripheral address
	DMA1_Stream0->M0AR = dest_addr; // memory address

	DMA1_Stream0->CR |= DMA_SxCR_EN; // enable dma stream
}


volatile uint8_t dma_rx_done;


void DMA1_Stream0_IRQHandler(void) {
    if (DMA1->LISR & DMA_LISR_TCIF0) {
        DMA1->LIFCR = DMA_LIFCR_CTCIF0; // clear DMA Stream 0 Transfer Complete interrupt flag

        // DMA reception completed: generate STOP
        I2C1->CR1 |= I2C_CR1_STOP;

        // disable I2C DMA requests
        I2C1->CR2 &= ~(I2C_CR2_DMAEN | I2C_CR2_LAST);

        // disable DMA stream
        DMA1_Stream0->CR &= ~DMA_SxCR_EN;

        dma_rx_done = 1;
    }

    if (DMA1->LISR & DMA_LISR_TEIF0) {
        DMA1->LIFCR = DMA_LIFCR_CTEIF0;
    }
}


