#
# this makefile will compile and and all source
# found in the ~/develop/MyApp directory.  This represents a sample
# development directory structure and project
# 
# =======================================================
#                  MyApp Example
# =======================================================
# FINAL BINARY Target
./bin/encodeInput : ./obj/encodeInput.o ./obj/encodeASM.o ./obj/encodeSREC.o
	cc ./obj/encodeInput.o ./obj/encodeASM.o ./obj/encodeSREC.o -o ./bin/encodeInput
#
# =======================================================
#                     Dependencies
# =======================================================                     

./obj/encodeInput.o : ./src/encodeInput.c ./inc/encodeASM.h ./inc/encodeSREC.h
	cc -c ./src/encodeInput.c -o ./obj/encodeInput.o

./obj/encodeASM.o : ./src/encodeASM.c ./inc/encodeASM.h
	cc -c ./src/encodeASM.c -o ./obj/encodeASM.o

./obj/encodeSREC.o : ./src/encodeSREC.c ./inc/encodeSREC.h
	cc -c ./src/encodeSREC.c -o ./obj/encodeSREC.o
#
# =======================================================
# Other targets
# =======================================================                     
all : ./bin/encodeInput

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
	rm -f ./inc/*.h~
	rm -f ./src/*.c~



