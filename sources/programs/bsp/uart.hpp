#ifndef UART_HPP
#define UART_HPP

#include <stdarg.h>
#include <stdint.h>

void uart_putc(char c);
char uart_getc(void);

void uart_write(int n, const char *buf);
void uart_read(int n, char *buf);

int uart_puts(const char *str);
void uart_gets(char *buf, int size);

int uart_printf(const char* format, ...);

template <typename vec_type> 
void uart_print_vector(vec_type vector)
 {
    uint8_t vregister_val[4096]; //LMUL=8 * VREG_W=4096bits = 4096 total bytes (TODO: cap this at the architecture level)
    uint32_t csr_vtype;
    uint32_t cur_vl, cur_sew, cur_lmul, vlenb, incr;

    //read csrs for current configuration
    asm volatile ("csrr %0,vl"  : "=r" (cur_vl) );
    asm volatile ("csrr %0,vtype"   : "=r" (csr_vtype)  );
    asm volatile ("csrr %0,vlenb"   : "=r" (vlenb)  );
    cur_lmul = 0x00000007 & csr_vtype;
    cur_sew = 0x00000007 & (csr_vtype >> 3);

    //get values from encodings
    switch (cur_lmul)
    {
        case 0x0:
            cur_lmul = 1;
            break;
        case 0x1:
            cur_lmul = 2;
            break;
        case 0x2:
            cur_lmul = 4;
            break;
        case 0x3:
            cur_lmul = 8;
            break;
        default:
            cur_lmul = 1;
            break;
    }
    switch (cur_sew)
    {
        case 0x0:
            cur_sew = 8;
            incr = 1;
            break;
        case 0x1:
            cur_sew = 16;
            incr = 2;
            break;
        case 0x2:
            cur_sew = 32;
            incr = 4;
            break;
        default:
            uart_printf("INVALID SEW DETECTED\n");
            incr=1;
            return;
    }
    uart_printf("\nCurrent Vector Register Configuration:\n");
    uart_printf("Current VREG_W   = %d bits\n", vlenb*8);
    uart_printf("Current VL       = %d elements\n", cur_vl);
    uart_printf("Current LMUL     = %d registers (fractional (mf*) configurations should report 1)\n", cur_lmul);
    uart_printf("Current SEW      = %d bits\n", cur_sew);

    //dump registers into buffer
    switch (cur_lmul)
    {
        case 1:
            asm volatile ("vs1r.v	%0,(%1)"   :  : "vr" (vector), "r" (vregister_val) :);
            break;
        case 2:
            asm volatile ("vs2r.v	%0,(%1)"   :  : "vr" (vector), "r" (vregister_val) :);
            break;
        case 4:
            asm volatile ("vs4r.v	%0,(%1)"   :  : "vr" (vector), "r" (vregister_val) :);
            break;
        case 8:
            asm volatile ("vs8r.v	%0,(%1)"   :  : "vr" (vector), "r" (vregister_val) :);
            break;
        default:
            asm volatile ("vs1r.v	%0,(%1)"   :  : "vr" (vector), "r" (vregister_val) :);
            break;
    }
    

    //loop over maximum possible elements for current lmul
    for (uint32_t i = 0; i < (vlenb*cur_lmul); i+= incr)
    {
        if (i % vlenb == 0)
        {
            uart_printf("\n");
        }
        if (i/incr < cur_vl)
        {
            switch(cur_sew)
            {
            case 8:
                uart_printf("%d ", *(int8_t*)(vregister_val+i));
                break;
            case 16:
                uart_printf("%d ", *(int16_t*)(vregister_val+i));
                break;
            case 32:
                uart_printf("%d ", *(int32_t*)(vregister_val+i));
                break;
            default:
                uart_printf("INVALID SEW DETECTED\n");
                return;
            }
        }
        else
        {
            uart_printf("XX ");
        }
    }
    uart_printf("\n\n");
    return;
 }

#endif
