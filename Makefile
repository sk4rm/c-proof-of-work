.PHONY: clean all

all: ProofOfWork.exe

ProofOfWork.exe: main.obj
	$(CC) /Fe:ProofOfWork.exe main.obj

main.obj: main.c
	$(CC) /c main.c

clean:
	del ProofOfWork.exe main.obj 2>nul