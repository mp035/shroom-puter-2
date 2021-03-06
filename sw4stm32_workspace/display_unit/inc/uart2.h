#ifndef _UART_H
#define _UART_H

#include "stm32l0xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <circular_buffer.h>

extern volatile CircularBuffer uart2TxBuff, uart2RxBuff;


// note: Uart2Init can also re-init the UART
// for configuration changes. Just call it with the
// new settings.
int Uart2Init(int baud, bool useParity, bool oddParity);
void Uart2WaitForTxFifoSpace( int space_needed ); // waits until the TX fifo is empty, use this instead of looping on Uart1TxFree()
int Uart2TxFree(); // Gets the number of empty bytes in the TX fifo, use this to check for space before calling Uart1Tx
int Uart2Tx(uint8_t* data, int len); // Pushes data into the TX fifo, non-blocking, returns how many bytes were successfully pushed.
int Uart2TxString(char* data); // Writes a string to the TX fifo, this function blocks when TX FIFO is full and waits for space to become available, instead of aborting, like the standard TX does.
int Uart2WaitForData(int numBytes, int timeoutMs); // Returns 0 when data is available, or 1 on timeout.  Use this function to wait for data to arrive instead of polling Uart1RxLen
int Uart2RxLen(); // Gets the number of bytes waiting in the RX fifo, use this to check for data before calling Uart1Rx.
int Uart2Rx(uint8_t *data, int len); // Reads at most len bytes from the RX fifo, returns the number of bytes successfully read.
void Uart2DiscardTxBuffer(); // clears the TX FIFO
void Uart2DiscardRxBuffer(); // clears the RX FIFO
bool Uart2HasRxErrorInFifo(); // returns true if one of the bytes in the fifo has an error.

// bitmask flags for errors
#define UART_OVERRUN_ERROR (1 << 0)
#define UART_PARITY_ERROR (1 << 1)
#define UART_FRAMING_ERROR (1 << 2)
#define UART_NOISE_DETECTED (1 << 3) // if a break is received, a null byte will be added to the fifo with this flag.
uint8_t Uart2GetRxErrorFlags(); // gets the errors for the last byte not yet read from the fifo.

#endif
