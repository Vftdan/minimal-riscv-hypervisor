#ifndef SRC_EXTINTERRUPTS_H_
#define SRC_EXTINTERRUPTS_H_

extern int uart_input_vm;
void uart_on_thre(void);
void uart_on_rda(void);

void init_external_interrupts(void);
void handle_external_interrupt(void);
void forward_external_interrupts(void);

#define PLIC_UART_IRQ 10

#endif /* end of include guard: SRC_EXTINTERRUPTS_H_ */
