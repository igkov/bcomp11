//;
//; Startup and main initializations. For GCC AS.
//;

.global main //; Entry point for C-program (main-function).
.global _etext                 //; -> .data (RW section address in ROM).
.global _data                  //; -> .data (RW section address in RAM).
.global _edata                 //; -> .data (RW section end address).
.global __bss_start            //; -> .bss (ZI section address).
.global __bss_end__            //; -> .bss (ZI section end address).
.global _stack                 //; Stack head address.
.global _startup               //; First entry point.
.global __ctors_start__        //; -> .ctor (Global constructors table start).
.global __ctors_end__          //; -> .ctor (Global constructors table end).
.global __errno                //; __errno sybmbol export.

.global NMI_Handler
.global HardFault_Handler
.global SVC_Handler
.global PendSV_Handler
.global SysTick_Handler
.global Default_Handler
.global WAKEUP_IRQHandler
.global CAN_IRQHandler
.global SSP1_IRQHandler
.global I2C_IRQHandler
.global TIMER16_0_IRQHandler
.global TIMER16_1_IRQHandler
.global TIMER32_0_IRQHandler
.global TIMER32_1_IRQHandler
.global SSP0_IRQHandler
.global UART_IRQHandler
.global USB_IRQHandler
.global USB_FIQHandler
.global ADC_IRQHandler
.global WDT_IRQHandler
.global BOD_IRQHandler
.global FMC_IRQHandler
.global PIOINT3_IRQHandler  
.global PIOINT2_IRQHandler 
.global PIOINT1_IRQHandler
.global PIOINT0_IRQHandler

.global ProtectDelay
.global SystemInit

.text

//;
//; Vectors table
//;
//; For assembler labels use +1 suffix,
//; for external C-functions use only function name.
//;
//;

_startup:
.word _stack                    //; Top of Stack
.word Reset_Handler+1           //; Reset Handler
.word NMI_Handler+1             //; NMI Handler
.word HardFault_Handler+1       //; Hard Fault Handler
.word 0                         //; Reserved
.word 0                         //; Reserved
.word 0                         //; Reserved
.word 0                         //; Reserved
.word 0                         //; Reserved
.word 0                         //; Reserved
.word 0                         //; Reserved
.word SVC_Handler+1             //; SVCall Handler
.word 0                         //; Reserved
.word 0                         //; Reserved
.word PendSV_Handler+1          //; PendSV Handler
.word SysTick_Handler           //; SysTick Handler
//; External Interrupts
.word WAKEUP_IRQHandler+1       //; 16+ 0: Wakeup PIO0.0
.word WAKEUP_IRQHandler+1       //; 16+ 1: Wakeup PIO0.1
.word WAKEUP_IRQHandler+1       //; 16+ 2: Wakeup PIO0.2
.word WAKEUP_IRQHandler+1       //; 16+ 3: Wakeup PIO0.3
.word WAKEUP_IRQHandler+1       //; 16+ 4: Wakeup PIO0.4
.word WAKEUP_IRQHandler+1       //; 16+ 5: Wakeup PIO0.5
.word WAKEUP_IRQHandler+1       //; 16+ 6: Wakeup PIO0.6
.word WAKEUP_IRQHandler+1       //; 16+ 7: Wakeup PIO0.7
.word WAKEUP_IRQHandler+1       //; 16+ 8: Wakeup PIO0.8
.word WAKEUP_IRQHandler+1       //; 16+ 9: Wakeup PIO0.9
.word WAKEUP_IRQHandler+1       //; 16+10: Wakeup PIO0.10
.word WAKEUP_IRQHandler+1       //; 16+11: Wakeup PIO0.11
.word WAKEUP_IRQHandler+1       //; 16+12: Wakeup PIO1.0
.word CAN_IRQHandler            //; 16+13: CAN
.word SSP1_IRQHandler+1         //; 16+14: SSP1
.word I2C_IRQHandler+1          //; 16+15: I2C
.word TIMER16_0_IRQHandler+1    //; 16+16: 16-bit Counter-Timer 0
.word TIMER16_1_IRQHandler+1    //; 16+17: 16-bit Counter-Timer 1
.word TIMER32_0_IRQHandler+1    //; 16+18: 32-bit Counter-Timer 0
.word TIMER32_1_IRQHandler+1    //; 16+19: 32-bit Counter-Timer 1
.word SSP0_IRQHandler+1         //; 16+20: SSP0
.word UART_IRQHandler+1         //; 16+21: UART
.word USB_IRQHandler+1          //; 16+22: USB IRQ
.word USB_FIQHandler+1          //; 16+24: USB FIQ
.word ADC_IRQHandler+1          //; 16+24: A/D Converter
.word WDT_IRQHandler+1          //; 16+25: Watchdog Timer
.word BOD_IRQHandler+1          //; 16+26: Brown Out Detect
.word FMC_IRQHandler+1          //; 16+27: IP2111 Flash Memory Controller
.word PIOINT3_IRQHandler+1      //; 16+28: PIO INT3
.word PIOINT2_IRQHandler+1      //; 16+29: PIO INT2
.word PIOINT1_IRQHandler+1      //; 16+30: PIO INT1
.word PIOINT0_IRQHandler+1      //; 16+31: PIO INT0

.code 16
.thumb
.align 4

//; void _startup(DWORD R0, DWORD R1, DWORD R2, DWORD R3)
//; r0, r1, r2, r3 contain parameters.
//; ( r0 - in size, r1 - out size, r2 - p1 )
Reset_Handler:
//; Stack initialize:
                LDR     R5, =_stack
                MOV     SP, R5

//; Initialize RW-section (.data):
                LDR     R5, =_etext
                LDR     R6, =_data
                LDR     R7, =_edata
                B       LoopRelEntry
LoopRel:
                LDR     R4, [R5]
                ADD     R5, #4
                STR     R4, [R6]
                ADD     R6, #4
LoopRelEntry:
                CMP     R6, R7
                BLO     LoopRel

//; Initialize ZI-section (.bss):
                MOV     R4, #0
                LDR     R5, =__bss_start__
                LDR     R6, =__bss_end__
                B       LoopZIEntry
LoopZI:
                STR     R4, [R5]
                ADD     R5, #4
LoopZIEntry:
                CMP     R5, R6
                BLO     LoopZI

//; Call C++ constructors (for global objects):
                LDR     R5, =__ctors_start__
                LDR     R6, =__ctors_end__
ctor_loop:
                CMP     R5, R6
                BEQ     ctor_end
                LDR     R4, [R5]
                ADD     R5, #4
                PUSH    {R0-R3,R5-R6}
                MOV     LR, PC
                MOV     PC, R4
                POP     {R0-R3,R5-R6}
                B       ctor_loop
ctor_end:

//; Jump to C entry point.
                LDR     R0, =ProtectDelay
                BLX     R0
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R4, =main
                LDR     R5, =__exit+1
                MOV     LR, R5
                BX      R4
__exit:
                B       .

NMI_Handler:
HardFault_Handler:
SVC_Handler:
PendSV_Handler:
//;SysTick_Handler:
Default_Handler:
WAKEUP_IRQHandler:
//;CAN_IRQHandler:
SSP1_IRQHandler:
I2C_IRQHandler:
TIMER16_0_IRQHandler:
TIMER16_1_IRQHandler:
TIMER32_0_IRQHandler:
TIMER32_1_IRQHandler:
SSP0_IRQHandler:
//;UART_IRQHandler:
USB_IRQHandler:
USB_FIQHandler:
ADC_IRQHandler:
WDT_IRQHandler:
BOD_IRQHandler:
FMC_IRQHandler:
PIOINT3_IRQHandler:
PIOINT2_IRQHandler:
PIOINT1_IRQHandler:
PIOINT0_IRQHandler:
                B       .

//; Если в сборку попадает функция __errno из стандартной библиотеки, 
//; то в ОЗУ размещается ненужный буфер размером 1024Б и появляется 
//; дополнительный код. Функция является заглушкой.
errno:
                .word       __errno
__errno:
                LDR R0, =errno;
                BX LR

.end
//; EOF
