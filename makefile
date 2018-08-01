#
# Makefile for GCC ARM.
# 
# Based on Aktiv JCS makefile from Guardant Code example application.
#
# Kovalenko Igor / 2017
#
# On command line:
#
# "make all"      - Make software.
# "make clean"    - Clean out built project files.
# "make filename" - Just compile filename into the assembler code only.
#
#  To rebuild project do "make clean" then "make all".
#

# Path of include files:
CFG_INCLUDE_DIR         = /gccarm/arm-none-eabi/include

# Path of library:
CFG_LIB_DIR             = /gccarm/arm-none-eabi/lib/thumb/v6-m

# Path of service utils:
CFG_GCC_DIR             = /gccarm/bin/

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level!)
#OPT = 0
#OPT = 1
OPT = 2
#OPT = s
#
# !!!!!
# Ќа уровне оптимизации -Os, есть проблемы с выравниванием структур в пам€ти.
# !!!!!
#

# List C source files here:
SRC     = bcomp.c
SRC    += adc.c
SRC    += analog.c
SRC    += bconfig.c
SRC    += beep.c
SRC    += buttons.c
SRC    += can.c
SRC    += config.c
SRC    += crc8.c
#SRC    += drive.c
SRC    += eeprom.c
#SRC    += elog.c
SRC    += errors.c
SRC    += event.c
SRC    += graph.c
SRC    += i2c.c
SRC    += leds.c
#SRC    += lzss.c
#SRC    += menu.c
#SRC    += nmea.c
SRC    += obd.c
#SRC    += oled128.c
#SRC    += oled128_sh1106.c
#SRC    += oled128_ssd1306.c
SRC    += printf.c
SRC    += sounds.c
SRC    += system_LPC11xx.c
SRC    += timer.c
SRC    += virtuino.c
SRC    += uart0.c
#SRC    += warning.c
#SRC    += wheels.c
SRC    += ./fonts/font12x16.c
SRC    += ./fonts/font6x8.c
SRC    += ./icons/icons11.c

# List C++ source files here:
# Use file-extension cpp for C++-files!
#CPPSRC   = module1.cpp
#CPPSRC  += module2.cpp
#CPPSRC  += module3.cpp
#CPPSRC  += module4.cpp

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
#ASRC  = asm1.s
#ASRC += asm2.s
#ASRC += asm3.s

# MCU name:
MCU      = cortex-m0

# Output format (can be srec, ihex, binary):
FORMAT   = ihex

# Target file name (without extension):
TARGET   = SAMPLE

# Linker Script Name:
LDSCRIPT  = rom

# Startup File Name:
ASTARTUP = startup_LPC11xx_gcc

# ARM Toolchain Prefix
ARM_TOOLS_PREFIX = arm-none-eabi

# List any extra directories to look for include files here.
# Each directory must be seperated by a space.
EXTRAINCDIRS = ./ ./arm $(CFG_INCLUDE_DIR) 

# Startup file into ASM sources:
ASRC = $(ASTARTUP).s

# Full List of sources:
ALLSRC = $(SRC) $(CPPSRC) $(ASRC)

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for C here:
CDEFS =  -D__GCC__ -D__ARM__ -DCORTEX -D_LPC1100_ -DCFG_GCC

# Place -I options here:
CINCS = 

# Place -D or -U options for ASM here:
ADEFS = 

# Compiler flags.
#  -g*:      generate debugging information
#  -O*:      optimization level
#  -f...:    tuning, see GCC manual and avr-libc documentation
#  -Wall...: warning level
#  -Wa,...:  tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing

CFLAGS  = -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
CFLAGS += -funsigned-char -funsigned-bitfields -fshort-enums -fpack-struct 
CFLAGS += -Wall -Wcast-align -Wcast-qual -Wimplicit 
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS += -Wno-pragmas -Wno-main
CFLAGS += -fdata-sections -ffunction-sections 

# flags only for C
# Turn on any warnings:
CONLYFLAGS  = -Wnested-externs -Wstrict-prototypes 
#CONLYFLAGS += -Wmissing-prototypes -Wnested-externs -Wmissing-declarations
# no listings
#CONLYFLAGS += -Wa,-adhmlns=.lst/$(<:.c=.lst)
CONLYFLAGS += $(CSTANDARD)

# flags only for C++ (g++)
#CPPFLAGS   = -Wa,-adhmlns=.lst/$(<:.cpp=.lst)
CPPFLAGS   = -lg -lm -lstdc++
CPPFLAGS  += -fno-rtti -fno-exceptions -fno-use-cxa-atexit

# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
# no listings
#ASFLAGS = $(ADEFS) -Wa,-adhlns=.lst/$(<:.s=.lst),-gstabs 

# Linker flags.
#  -Wl,...: tell GCC to pass this to linker.
#  -Map:    create map file
#  --cref:  add cross reference to	map file
LDFLAGS  = -nostartfiles -Wl,-Map=.out_gcc/$(TARGET).map,--cref
LDFLAGS += -lc -lm -lgcc -L./ -L$(CFG_LIB_DIR) -mthumb-interwork
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -T$(LDSCRIPT).ld

# Define programs and commands.
CC        = $(CFG_GCC_DIR)$(ARM_TOOLS_PREFIX)-gcc
CPP       = $(CFG_GCC_DIR)$(ARM_TOOLS_PREFIX)-g++
AS        = $(CFG_GCC_DIR)$(ARM_TOOLS_PREFIX)-as
OBJCOPY   = $(CFG_GCC_DIR)$(ARM_TOOLS_PREFIX)-objcopy
OBJDUMP   = $(CFG_GCC_DIR)$(ARM_TOOLS_PREFIX)-objdump
SIZE      = $(CFG_GCC_DIR)$(ARM_TOOLS_PREFIX)-size
NM        = $(CFG_GCC_DIR)$(ARM_TOOLS_PREFIX)-nm
REMOVE     = rm -f
COPY       = cp

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE = Size:
MSG_FLASH = Creating hex file:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:

# Define all object files.
OBJ = $(SRC:.c=.o) $(ASRC:.s=.o) $(CPPSRC:.cpp=.o)

# Define all listing files.
LST = $(ASRC:.s=.lst) $(SRC:.c=.lst) $(SRCT:.c=.lst) $(CPPSRC:.cpp=.lst)

# Compiler flags to generate dependency files.
FTEMP = $(@F)
GENDEPFLAGS = -MD -MP -MF .dep/$(FTEMP:.o=.o.d)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mcpu=$(MCU) -mthumb -mthumb-interwork -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) -mthumb -mthumb-interwork -I. $(ASFLAGS)

# Default target.
all: begin prebuild build postbuild sizeafter finished end

# Build target.
build: elf hex

# Link: create ELF output file from object files.
elf: $(OBJ)
	$(CC) -c $(ALL_ASFLAGS) $(ASRC) -o .obj/$(ASRC:.s=.o)
	@echo $(MSG_LINKING) .out_gcc/$(TARGET).elf
	$(CC) $(ALL_CFLAGS) $(addprefix .obj/,$(OBJ)) --output .out_gcc/$(TARGET).elf $(LDFLAGS)

hex:
	@echo
	@echo $(MSG_FLASH) hex
	$(OBJCOPY) -O $(FORMAT) .out_gcc/$(TARGET).elf .out_gcc/$(TARGET).hex

# Eye candy.
begin:
	@echo $(MSG_BEGIN)

# Files check.
prebuild:
	@if [ ! -e .dep ]; then mkdir .dep; fi;
	@if [ ! -e .obj ]; then mkdir .obj; fi;
	@if [ ! -e .obj/fonts ]; then mkdir .obj/fonts; fi;
	@if [ ! -e .obj/icons ]; then mkdir .obj/icons; fi;
	@if [ ! -e .lst ]; then mkdir .lst; fi;
	@if [ ! -e .out_gcc ]; then mkdir .out_gcc; fi;

postbuild:
	$(OBJDUMP) -xD .out_gcc/$(TARGET).ELF --disassembler-options=force-thumb > .out_gcc/$(TARGET).asm

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	
# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) .out_gcc/$(TARGET).hex
ELFSIZE = $(SIZE) -A .out_gcc/$(TARGET).elf

sizeafter:
	@if [ -f .out_gcc/$(TARGET).elf ]; then echo; echo $(MSG_SIZE); $(ELFSIZE); fi;

# Compile: create object files from C source files.
%.o : %.c
	@echo $(MSG_COMPILING) $@ $<
	$(CC) -c $(ALL_CFLAGS) $(CONLYFLAGS) $< -o .obj/$(@:.o=.o)

# Compile: create object files from C++ source files.
%.o : %.cpp
	@echo $(MSG_COMPILINGCPP) $<
	$(CPP) -c $(ALL_CFLAGS) $(CPPFLAGS) $< -o .obj/$(@:.o=.o)

# Assemble: create object files from assembler source files.
%.o : %.s
	@echo
	@echo $(MSG_ASSEMBLING) $<
	@if [ -e ".obj" ]; then $(CC) -c $(ALL_ASFLAGS) $< -o .obj/$(@:.o=.o) ; fi;

# Target: clean project.
clean: begin clean_list finished end

clean_list:
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) -r .out_gcc
	$(REMOVE) -r .lst
	$(REMOVE) -r .obj
	$(REMOVE) -r .dep
	mkdir .out_gcc
	mkdir .obj
	mkdir .obj/fonts
	mkdir .obj/icons
	mkdir .lst
	mkdir .dep

# Listing of phony targets.
.PHONY: all begin finish end sizeafter gccversion \
prebuild build elf hex lss sym clean clean_list finished end
