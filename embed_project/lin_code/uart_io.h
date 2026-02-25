// uart_io.h
#ifndef UART_IO_H_
#define UART_IO_H_

#pragma once

#include <stdint.h>
#include <stdbool.h>

// UART non-blocking I/O
// - RX: 1-byte chunk receive -> RX ring push -> re-arm
// - TX: TX ring enqueue -> drain_step starts HW TX
// - poll_complete advances TX ring on completion

void uart_io_init(void);
void uart_io_poll(void);                 // call frequently in main loop

int  uart_read_byte(uint8_t *out);       // returns 1 if byte available
void uart_write_char(char c);            // enqueue only (non-blocking)
void uart_write_str(const char *s);      // enqueue only (non-blocking)


#endif /* UART_IO_H_ */
