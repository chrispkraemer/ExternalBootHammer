CXX=g++
RM=rm
TINY=Blink.ino.tiny.hex
BIG=Blink.ino.hex
TESTDIRS= $(shell ls -d testprograms_hex/*)


all: main 

main: Instruction.o boothammer.o Function.o
	$(CXX) boothammer.o Instruction.o Function.o -o hexBootHammer

Function.o: Function.cpp
	$(CXX) -c -g Function.cpp

Instruction.o: Instruction.cpp
	$(CXX) -c  -g Instruction.cpp

boothammer.o: boothammer.cpp
	$(CXX) -c  -g boothammer.cpp

runtiny:
	./hexBootHammer $(TINY)

runbig:
	./hexBootHammer $(BIG)

prepare:
	for dir in $(TESTDIRS) ; do \
		arm-none-eabi-objdump -D -z -m arm -M force-thumb $$dir/*.ino.hex > $$dir/hexobjdump.disas ; \
		arm-none-eabi-objdump -D -z -m arm -M force-thumb $$dir/*.ino.elf > $$dir/elfobjdump.disas ; \
		cat $$dir/hexobjdump.disas | sed -e 's/; <UNDEFINED> .*/nop/' -e 's/stc.*/nop/' -e 's/ldc2l.*/nop/' -e 's/ .v.*/nop/' -e 's/pldw.*/nop/' -e 's/ldc.*/nop/' -e 's/nop[a-z]*/nop/' \
        -e 's/nop.[a-z]*/ nop/' -e 's/ldr...w.*/nop/' -e 's/str..w.*/nop/' -e 's/mrc.*/nop/'> $$dir/editedHex.disas ; \
	done

differ:
	for dir in $(TESTDIRS) ; do \
		$(file: $($$dir))  \
    	./hexBootHammer -disas -f $$dir/*.ino.hex > $$dir/hexHammer.dump ; \
		diff -E -Z -w -b $$dir/hexHammer.dump $$dir/editedHex.disas ; \
    done
    
hex:
	for dir in $(TESTDIRS) ; do \
    	$(file: $($$dir)) \
        ./hexBootHammer -hex -f $$dir/*.ino.hex > $$dir/hexHammer.hex ; \
		./hexBootHammer -disas -f $$dir/hexHammer.hex > $$dir/disasFromHexHammer.dump; \
		tail -n +7 $$dir/disasFromHexHammer.dump > $$dir/tailDFHH.dump ;\
		tail -n +7 $$dir/editedHex.disas > $$dir/tailEditedHex.disas ;\
        diff -E -Z -w -b $$dir/tailDFHH.dump $$dir/tailEditedHex.disas; \
    done
	

clean:
	rm *.o main
	$(RM) hexBootHammer
	find testprograms_hex/ -type f -name "*.disas" -delete
