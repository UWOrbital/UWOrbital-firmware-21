.PHONY: build run


include_dirs := -I.
include_dirs += -I../../FreeRTOS/FreeRTOS/Source/include
include_dirs += -I../../FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix

source_files := $(wildcard *.c)
source_files += $(wildcard ../../FreeRTOS/FreeRTOS/Source/*.c)
source_files += $(wildcard ../../FreeRTOS/FreeRTOS/Source/portable/MemMang/heap_3.c)
source_files += $(wildcard ../../FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/*.c)
source_files += $(wildcard ../../FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/*.c)


linker := -lpthread
build: $(source_files)
	mkdir -p build
	gcc -Og -g $(include_dirs) $(source_files) $(linker) -o a.out

test: build
	./a.out test

bench: build
	./a.out bench
