#include "stm32l0xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <uart2.h>
#include "circular_buffer.h"
#include "gpio.h"

#ifndef UART2_RX_BUFFER_SIZE
#define UART2_RX_BUFFER_SIZE 64
#endif

NEW_CIRC_BUFFER(uart2TxBuff, 64);
NEW_CIRC_BUFFER(uart2RxBuff, UART2_RX_BUFFER_SIZE);
NEW_CIRC_BUFFER(uart2ErrBuff, UART2_RX_BUFFER_SIZE);

static int uart2ErrorCount = 0;
static bool uart2FifoOverrun = false;

// returns the actual baud rate.
int Uart2Init(int baud, bool useParity, bool oddParity)
{

	// ensure USART Clock is Enabled
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	// select system clock source
	RCC->CCIPR |= RCC_CCIPR_USART2SEL_1; // UART2SEL 0b01
	// **************************************************************

	NVIC_DisableIRQ(USART2_IRQn); // disable the interrupt in NVIC

	// deterimine required word length
	uint32_t wordLength = 0;
	uint32_t parityControlEnable = 0;
	uint32_t paritySelection = 0;
	if (useParity)
	{

#ifndef USART_CR1_M0
		// the stm32F030x6 CMSIS file is broken, it does not declare M1, it just declares M0 as M
		wordLength = USART_CR1_M;
#else
		wordLength = USART_CR1_M0;
#endif

		parityControlEnable = USART_CR1_PCE;
		if (oddParity)
		{
			paritySelection = USART_CR1_PS;
		}
	}

	uint32_t fclk = SystemCoreClock; // we are using system clock/1 as our peripheral clock, change this if required

	// find oversampling rate with the lowest error for the
	// given baud rate
	float error16 = ((float)fclk/(float)baud);
	error16 -= (uint32_t)error16;
	float error8 = ((float)fclk*2.0/(float)baud);
	error8 -= (uint32_t)error8;

	// while we are calculating, find out whether
	// we need to round up, and hold on to the answer
	// for later.
	bool roundUp16=false, roundUp8=false;

	if (error16 >= 0.5)
	{
		error16 = 1.0 - error16;
		roundUp16 = true;
	}

	if (error8 >= 0.5)
	{
		error8 = 1.0 - error8;
		roundUp8 = true;
	}

	uint32_t UARTDIV;
	uint32_t oversamp = 0;

	// determine whether to oversample by 8 or 16
	if (error16 > error8 )
	{
		// oversample by 8
		oversamp = USART_CR1_OVER8;
		UARTDIV = fclk * 2 / baud;
		if (roundUp8)
		{
			UARTDIV += 1;
		}
	}
	else
	{
		// oversample by 16
		UARTDIV = fclk / baud;
		if (roundUp16){
			UARTDIV += 1;
		}
	}

	// cap the divider at the maximum baud rate.
	if (UARTDIV < 16)
	{
		UARTDIV = 16;
	}

	uint32_t appliedBaud;
	// record the actual baud rate that was applied.
	if (oversamp)
	{
		appliedBaud = fclk * 2 / UARTDIV;
	}
	else
	{
		appliedBaud = fclk / UARTDIV;
	}


	USART2->CR1 &= ~USART_CR1_UE; // ensure the usart is stopped before attempting to configure

	//TODO: for rs485 we need to set up the DEAT and DEDT times.
	USART2->CR1 = oversamp | wordLength | parityControlEnable | paritySelection |
			USART_CR1_TXEIE | USART_CR1_RXNEIE | USART_CR1_IDLEIE | USART_CR1_TE |
			USART_CR1_RE;

	// if 2 stop bits are required, set CR2 to USART_CR2_STOP_1
	USART2->CR2 = 0;


	//TODO: the bit DEP in CR3 selects the RS485 driver enable polarity detection.
	// DEP = 1 = active low, DEP = 0 = active high
	//TODO: the bit DEM enables the DE pin (ie. we need DEM = 1 for our RS485 design)
	USART2->CR3 = 0;

	// program the baud rate
	if (oversamp)
	{
		uint32_t temp = UARTDIV >> 1;
		temp &= 0b111;
		UARTDIV &= 0xFFFFFFF0;
		USART2->BRR = (UARTDIV | temp);
	}
	else
	{
		USART2->BRR = UARTDIV;
	}


	// read any dead data from the rx buffer.
	uint8_t temp;
	temp = USART2->RDR;
	(void)temp;


	// enable the receive interrupt
	USART2->CR1 |= USART_CR1_RXNEIE;

	// enable the USART
	USART2->CR1 |= USART_CR1_UE;


	NVIC_SetPriority(USART2_IRQn,2); // 3 is lowest priority, 0 is highest
	NVIC_EnableIRQ(USART2_IRQn); // enable the interrupt in NVIC


	// return the baud rate that was actually applied
	return appliedBaud;
}

int Uart2TxFree()
{
	// buffer available function is non-atomic,
	// so we need to disable interrupts before
	// calling.
	NVIC_DisableIRQ(USART2_IRQn); // disable the interrupt in NVIC
	int temp = BuffAvail(&uart2TxBuff);
	NVIC_EnableIRQ(USART2_IRQn); // enable the interrupt in NVIC
	return temp;
}

int Uart2Tx(uint8_t* data, int len)
{
	int count;
	for (count = 0; count < len; count++)
	{
		if(BuffPush(&uart2TxBuff, data+count))
		{
			break;
		}
	}
	// make sure transmit interrupt is enabled whenever a byte is written,
	// just in case the uart is idle.
	USART2->CR1 |= USART_CR1_TXEIE;
	return count;
}

int Uart2RxLen()
{
	// buffer level function is non-atomic,
	// so we need to disable interrupts before
	// calling.
	NVIC_DisableIRQ(USART2_IRQn); // disable the interrupt in NVIC
	int temp = BuffLevel(&uart2RxBuff);
	NVIC_EnableIRQ(USART2_IRQn); // enable the interrupt in NVIC
	return temp;
}

int Uart2Rx(uint8_t *data, int len)
{
	int count;
	for (count = 0; count < len; count++)
	{
		uint8_t temp;
		temp = 0;
		BuffPop(&uart2ErrBuff, &temp);
		if (temp) // if the read character has an associated error
		{
			uart2ErrorCount--; // that error is no longer in the fifo.
		}

		if(BuffPop(&uart2RxBuff, data+count))
		{
			// just for safety's sake, make sure
			// the error buffer and count don't
			// fall out of sync with the rx buffer.
			// Clear them both when the rx fifo becomes empty.
			uart2ErrorCount = 0;
			BuffClear(&uart2ErrBuff);
			break;
		}
	}
	return count;
}

void Uart2DiscardRxBuffer()
{
	NVIC_DisableIRQ(USART2_IRQn); // disable the interrupt in NVIC
	BuffClear(&uart2RxBuff);
	BuffClear(&uart2ErrBuff);
	uart2ErrorCount = 0;
	uart2FifoOverrun = false;
	NVIC_EnableIRQ(USART2_IRQn); // enable the interrupt in NVIC
}

void Uart2DiscardTxBuffer()
{
	BuffClear(&uart2TxBuff);
}

bool Uart2HasRxErrorInFifo() // returns true if one of the bytes in the fifo has an error.
{
	return (uart2ErrorCount != 0);
}

uint8_t Uart2GetRxErrorFlags() // gets the errors for the byte not yet read from the fifo.
{
	uint8_t temp = 0;
	BuffPeek(&uart2ErrBuff, &temp);
	return temp;
}

void USART2_IRQHandler(void)
{

	if ((USART2->ISR | USART_ISR_TXE) && (USART2->CR1 & USART_CR1_TXEIE))
	{
		if (BuffPop(&uart2TxBuff, (uint8_t*)&USART2->TDR))
		{
			// if there is no data in the TX buffer, then disable the
			// interrupt to prevent recursive interrupts
			USART2->CR1 &= ~USART_CR1_TXEIE;
		}
	}

	// now that USART is read, check and fix ORE flag
	if (USART2->ISR & USART_ISR_ORE)
	{
		//This should never happen, but
		// to be truly correct, we flag
		// overruns here as well.
		uart2FifoOverrun = true;
		USART2->ICR = USART_ICR_ORECF;
	}

	if ((USART2->ISR & USART_ISR_RXNE) && (USART2->CR1 & USART_CR1_RXNEIE))
	{
		uint8_t temp = USART2->RDR; // we need this temp to force a read because
									// BuffPush will not read the register if the buffer is full.
		if (BuffPush(&uart2RxBuff, &temp))
		{
			// we have an overrun, so flag it
			uart2FifoOverrun = true;
		}
		else
		{
			// reuse temp;
			temp = 0;
			if (uart2FifoOverrun)
			{
				// an overrun occurred before this byte was received
				temp |= UART_OVERRUN_ERROR;
				uart2FifoOverrun = false;
			}

			if (USART2->ISR |= USART_ISR_PE)
			{
				temp |= UART_PARITY_ERROR;
				USART2->ICR = USART_ICR_PECF;
			}

			if (USART2->ISR |= USART_ISR_FE)
			{
				temp |= UART_FRAMING_ERROR;
				USART2->ICR = USART_ICR_FECF;
			}
			if (USART2->ISR |= USART_ISR_NE)
			{
				temp |= UART_NOISE_DETECTED;
				USART2->ICR = USART_ICR_NCF;
			}

			if (temp)
			{
				uart2ErrorCount++;
			}
			BuffPush(&uart2ErrBuff, &temp);
		}
	}

	if ((USART2->ISR & USART_ISR_IDLE) && (USART2->CR1 & USART_CR1_IDLEIE))
	{
		// TODO: on idle, force an interrupt to get the buffer to flush
		USART2->CR1 &= ~USART_CR1_IDLEIE;
	}
}



