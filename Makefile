.PHONY: clean all

all: ProofOfWork.exe

ProofOfWork.exe: main.obj
	$(CC) /Fe:ProofOfWork.exe main.obj /link /LTCG

main.obj: main.c
	$(CC) /c /O2 /GL /std:c11 /experimental:c11atomics main.c

clean:
	del ProofOfWork.exe main.obj 2>nul