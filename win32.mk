.PHONY : all

CC = gcc.exe
CCP_DEFS = $(addprefix -D, UNICODE WIN32_LEAN_AND_MEAN _USE_MATH_DEFINES)

all : main.exe



main.exe : src/main.c src/dbg.c src/dbg_wm.c src/gm_ccb.c
	$(CC) -o $@ $(CCP_DEFS) -mwindows -municode -march=pentium4 -Wall $^
