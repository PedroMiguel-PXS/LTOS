# make file to make the LTOS
# translate to english

AS = nasm
CC = gcc
LD = gcc

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -m32 -T src/linker.ld -ffreestanding -O2 -nostdlib -no-pie

# folders (using quotes because of spaces)
OBJ_DIR = src/objects
BUILD_DIR = build

# final object files
OBJS = $(OBJ_DIR)/boot.o $(OBJ_DIR)/kernel.o $(OBJ_DIR)/gui.o $(OBJ_DIR)/wc.o $(OBJ_DIR)/ttt.o $(OBJ_DIR)/msche.o $(OBJ_DIR)/mtask.o $(OBJ_DIR)/eye.o $(OBJ_DIR)/shell.o

all: $(BUILD_DIR)/ltos1.bin

# rule to compile the Assembly
$(OBJ_DIR)/boot.o: src/Boot/boot.s
	$(AS) -felf32 src/Boot/boot.s -o $(OBJ_DIR)/boot.o

# rule to compile the Kernel
$(OBJ_DIR)/kernel.o: src/LTOS_Kernel/kernel.c
	$(CC) $(CFLAGS) -c src/LTOS_Kernel/kernel.c -o $(OBJ_DIR)/kernel.o

# rule to compile the GUI
$(OBJ_DIR)/gui.o: src/GUI/gui.c
	$(CC) $(CFLAGS) -c src/GUI/gui.c -o $(OBJ_DIR)/gui.o

# rule to compile the Window Creator
$(OBJ_DIR)/wc.o: src/window_creator/WC_func.c
	$(CC) $(CFLAGS) -c src/window_creator/WC_func.c -o $(OBJ_DIR)/wc.o

# rule to compile the tictactoe
$(OBJ_DIR)/ttt.o: src/window_creator/Apps/tictactoe.c
	$(CC) $(CFLAGS) -c src/window_creator/Apps/tictactoe.c -o $(OBJ_DIR)/ttt.o

# rule to compile the scheduler
$(OBJ_DIR)/msche.o: src/m-scheduler/msche.c
	$(CC) $(CFLAGS) -c src/m-scheduler/msche.c -o $(OBJ_DIR)/msche.o

# rule to compile the multi-task manager
$(OBJ_DIR)/mtask.o: src/mtask/mtask.c
	$(CC) $(CFLAGS) -c src/mtask/mtask.c -o $(OBJ_DIR)/mtask.o

# rule to link everything
$(BUILD_DIR)/ltos1.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/ltos1.bin $(OBJS)

# rule to compile the eye effect
$(OBJ_DIR)/eye.o: src/window_creator/Apps/eye_effect.c
	$(CC) $(CFLAGS) -c src/window_creator/Apps/eye_effect.c -o $(OBJ_DIR)/eye.o

# rule to compile the shell app
$(OBJ_DIR)/shell.o: src/window_creator/Apps/shell_app.c
	$(CC) $(CFLAGS) -c src/window_creator/Apps/shell_app.c -o $(OBJ_DIR)/shell.o

run: all
	# its -kernel, bc this is .bin, not .iso or .img
	qemu-system-i386 -usb -device usb-tablet -kernel $(BUILD_DIR)/ltos1.bin

clean:
	rm -f $(OBJ_DIR)/*.o $(BUILD_DIR)/*.bin
