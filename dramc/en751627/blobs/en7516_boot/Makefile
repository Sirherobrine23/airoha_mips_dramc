
ifneq ($(strip $(TCSUPPORT_CPU_EN7516) $(TCSUPPORT_CPU_EN7527) $(TCSUPPORT_CPU_EN7580)),)
BOOT2BACK = $(BOOTROM_UNOPEN_IMG_PATH)/boot2.img
BOOT2TXTBACK = $(BOOTROM_UNOPEN_IMG_PATH)/boot2.text
MOVEDATABACK = $(BOOTROM_UNOPEN_IMG_PATH)/move_data.img
MOVEDATATXTBACK = $(BOOTROM_UNOPEN_IMG_PATH)/move_data.text
endif

####### Variables Declares #########################
CC 		= $(BOOTROM_CC)
LD 		= $(BOOTROM_LD)
NM 		= $(BOOTROM_NM)
OBJCOPY = $(BOOTROM_OBJCOPY)
OBJDUMP = $(BOOTROM_OBJDUMP)
CFLAGS	= $(BOOTROM_CFLAG)
IMG2BIN	= $(BOOTROM_IMG2BIN)

RM	= rm
####################################################


all:

ifneq ($(TCSUPPORT_BB_FIX_UNOPEN),1)
	make -C move_data $(BOOTRAM_PARAMETER)
	make -C boot2 $(BOOTRAM_PARAMETER) LZMA_IMG=1

	cp ./boot2/output/boot2.img ./boot2.img -f
	cp ./boot2/output/boot2.text ../boot2.txt -f
	cp ./move_data/output/move_data.img ./move_data.img -f	
	cp ./move_data/output/move_data.text ../move_data.txt -f

	#/* backup */
ifneq ($(strip $(TCSUPPORT_CPU_EN7516) $(TCSUPPORT_CPU_EN7527) $(TCSUPPORT_CPU_EN7580)),)
	cp ./boot2/output/boot2.img $(BOOT2BACK) -f
	cp ./boot2/output/boot2.text $(BOOT2TXTBACK) -f
	cp ./move_data/output/move_data.img $(MOVEDATABACK) -f
	cp ./move_data/output/move_data.text $(MOVEDATATXTBACK) -f
endif
else
	#/* restore */
	cp $(BOOT2BACK) ./boot2.img -f
	cp $(BOOT2TXTBACK) ../boot2.txt -f
	cp $(MOVEDATABACK) ./move_data.img -f
	cp $(MOVEDATATXTBACK) ../move_data.txt -f
endif

	$(CC) $(CFLAGS) -c move_data_load.c -o move_data_load.o
	$(CC) $(CFLAGS) -c boot2load.c -o boot2load.o

ifeq ($(strip $(TCSUPPORT_SECURE_BOOT_V1)),)
	$(OBJCOPY) --add-section .move_data=move_data.img move_data_load.o
	$(OBJCOPY) --add-section .boot2=boot2.img boot2load.o
endif

clean:
	make -C move_data clean
	make -C boot2 clean
	$(RM) -f *.o
	$(RM) -f *.img
