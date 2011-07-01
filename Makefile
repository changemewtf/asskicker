# --- Project details ---
PROJ    := main
IMG	:= pics
IDIR	:= ../tonclib
GFXDIR	:= gfx

PICN 	:= walk1 walk2 walk3 jump slash1 slash2 slash3 slash4 crouch dkslash3 dkslash4 punch1 punch2 punch3
PICEXT	:= bmp
PICLIST := $(foreach PIC,$(PICN),$(GFXDIR)/$(PIC).$(PICEXT))
PICC	:= $(foreach PIC,$(PICN),$(GFXDIR)/$(PIC).c)
PICH	:= $(foreach PIC,$(PICN),$(GFXDIR)/$(PIC).h)
PICSRC	:= $(PICC) $(PICH)

COBJS   := $(PROJ).o $(IMG).o pal.o maxgba.o
IOBJS	:= $(IDIR)/core.o $(IDIR)/oam.o $(IDIR)/keypad.o
OBJS	:= $(COBJS) $(IOBJS)

GIT	:= wine /home/mcantor/code/git/git.exe
GITARG	:= -gt -gB4 -p!
PALMSTR	:= $(GFXDIR)/walk1.bmp			# master palette image
PALARG	:= -g! -o "../pal" -s "pal"

# --- Compiling defines ---
CC      := arm-eabi-gcc
LD      := arm-eabi-gcc
OBJCOPY := arm-eabi-objcopy

MODEL   := -mthumb-interwork -mthumb
EXT     := gba
SPECS   := -specs=gba_mb.specs

CFLAGS  := $(MODEL) -O2 -Wall -I$(IDIR) -g
LDFLAGS := $(SPECS) $(MODEL) -g

# --- Build rules ---
# first rule; make process starts here
build: $(PROJ).$(EXT)

# translate step: .elf -> (.mb).gba
$(PROJ).$(EXT) : $(PROJ).elf
	$(OBJCOPY) -v -O binary $< $@
	@gbafix $@

# link step: .o -> .elf
$(PROJ).elf : $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

# .c depends on .h
# compile step: .c -> .o
$(OBJS) : %.o : %.c $(IMG).c
	$(CC) -c $< $(CFLAGS) -o $@

# image compile step: *.c/*.h -> .c/.h
$(IMG).c : $(PICC)
	@rm -vf $(IMG).c $(IMG).h
	@echo Creating $(IMG)...	
	@for PIC in $(PICN); do \
		echo Adding $$PIC to $(IMG)...; \
		cat $(GFXDIR)/$$PIC.h >> $(IMG).h; \
		cat $(GFXDIR)/$$PIC.c >> $(IMG).c; \
	done

# conversion step: *.bmp -> *.c/*.h
$(PICC) : %.c : %.bmp
	$(GIT) $< $(GITARG)

# palette step: $(PALMSTR).bmp -> pal.c/.h
pal.c : $(PALMSTR)
	$(GIT) $< $(PALARG)

.PHONY : clean
clean : 
	@rm -vf $(COBJS) $(PROJ).$(EXT) $(PROJ).elf

.PHONY : cleaner
cleaner : 
	@rm -vf $(COBJS) $(PROJ).$(EXT) $(PROJ).elf $(PICSRC) $(IMG).h $(IMG).c
