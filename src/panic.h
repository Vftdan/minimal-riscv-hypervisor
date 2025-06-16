#ifndef SRC_PANIC_H_
#define SRC_PANIC_H_

__attribute__((noreturn)) void panic(void);
void dump_caller_registers(void);

#endif /* end of include guard: SRC_PANIC_H_ */
