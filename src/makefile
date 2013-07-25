


CXXFLAGS = -fno-builtin -nostdinc -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I./include 
LDFLAGS = -T link.ld
OCFLAGS = -Obinary

CXX = g++ $(CXXFLAGS) 
LD = ld $(LDFLAGS) 
OC = objcopy $(OCFLAGS) 
YASM = yasm -f elf 



OBJECTS :=
OBJECTS += $(patsubst %.asm,%.o,  $(wildcard startup/*.asm)) \
		$(patsubst %.cpp,%.o,  $(wildcard kernel/*.cpp))  \
		$(patsubst %.asm,%.o,  $(wildcard kernel/*.asm))  \
		$(patsubst %.cpp,%.o,  $(wildcard mm/*.cpp))  \
		$(patsubst %.cpp,%.o,  $(wildcard klibc/*.cpp))  
		   
.PHONY: kernel clean
kernel: $(OBJECTS)
		$(LD) -o kernel.exe $(OBJECTS)
		$(OC) kernel.exe kernel.bin
		$(OC) kernel.exe Y:\kernel.bin

clean: 
		del startup\*.o
		del kernel\*.o
		del mm\*.o
		del klibc\*.o
 
 %.o : %.asm 
		 $(YASM) -o $@ $< 