#include <fstream>
#include <iostream>
//#include "Instruction.h"
#include <vector>
#include <iomanip>
#include <bitset>
#include <signal.h>
#include "Function.h"

using namespace std;

void fillFunction(Function* func, int startAddr);
void printByFunction();
void controlFlow();
string printOption(Option o);
int getProgramIndex(unsigned int addr);
unsigned int generateBL(unsigned int target_addr, unsigned int current_addr);
void bracket3Regs(Register r1, Register r2, Register r3);
void bracket2RegsImm(Register r1, Register r2, unsigned imm);
void regList(unsigned bits);
void print2Regs(Register r1, Register r2);
void print2RegsImm(Register r1, Register r2, unsigned imm);
void print3Regs(Register r1, Register r2, Register r3);
void printFormatReg(Register r);
void printInstrName(string s);
void decode(string instruction, int addr);
void printProgram();
void printInstr(Instruction i);
void checkIfThenInstr();
void printHex();
void checkAddrs();
void printByte(unsigned b);
void insertTrampoline();
void insertAndDecode(unsigned int addr, unsigned int bytecode);
string printRegister(Register r);
vector<Instruction> program;
vector<Function> functions;
vector<string> eofInstrs;
int fw = 12;
short int yesHex = 32;
string ifThenModifier = "";
short int ifThenNumber = 0;


bool disas = false;
bool hexout = false;
bool trampoline = false;
bool funcOut = false;


int main(int argc, char** argv){

	string filename;

    for(int i = 1; i < argc; i++){
        if(string(argv[i]) == "-disas"){
            disas = true;
        } else if(string(argv[i]) == "-hex"){
            hexout = true;
        } else if(string(argv[i]) == "-t"){
            trampoline = true; 
        } else if(string(argv[i]) == "-f"){
            filename = argv[i + 1];
            i++;
        } else if(string(argv[i]) == "-func"){
            funcOut = true;
        } else {
            cout << "argument not recognized\n";
            return 0;
        }
    }


	fstream fileStream;
	fileStream.open(filename);

	string line;


	int instrAddr = 0x2000;

    if(disas){
        cout << endl << filename << ": file format ihex" <<  endl << endl << endl;
        cout << "Disassembly of section .sec1:" << endl << endl;
        cout << "0000" << hex <<  instrAddr << " <.sec1>:" << endl;
    }
	unsigned halfword;
	unsigned opcodeWordOrHalfWord;
	bool is32 = 0;
	string instruction;
	unsigned halfword2;
	unsigned long word; 
    int hexLineLength;
	while(fileStream >> line){
		if(line.substr(7,2) == "00"){ //line.length() == 43
			line = line.substr(9,line.length() - 11); // line.length - 11 should give just the data part of the line

			for(int i = 0; i <= line.length(); i=i+2){
				if(i % 4 == 0 && i != 0){
					if(is32){
						halfword2 = stol(instruction, nullptr, 16);
						//... blah blah blah
						Instruction myInst(0,0);
						word = (halfword << 16) | halfword2;
						myInst.addr = instrAddr;
						//cout << "halfword1: " << hex << halfword << " halfword2: " << hex << hallword2 << " " << hex << word << endl;
						myInst.decodeInstr32(word);
						instruction = "";
						halfword2 = 0;
						halfword = 0;
						instrAddr += 0x04;
						program.push_back(myInst);
						is32 = false;

					} else {
						halfword = stol(instruction,nullptr, 16);
						opcodeWordOrHalfWord = halfword >> 11;
						//opcodeWordOrHalfWord = opcodeWordOrHalfWord & 0x1F;
						if(opcodeWordOrHalfWord == 0b11101 || opcodeWordOrHalfWord == 0b11110 || opcodeWordOrHalfWord == 0b11111){
							//is a 32 bit instruction, read next halfword
							is32 = true;
							instruction = "";
							//need to get another byte, so go around the loop again
							
						} else {
							//is a 16 bit instruction, go ahead and decode
							Instruction myInst(0,0);
							myInst.addr = instrAddr;
							myInst.decodeInstr16(halfword);
							instrAddr+=0x02;
							instruction = "";
							program.push_back(myInst);
						}

					}
				}
				instruction.insert(0,line.substr(i,2));
			}
		} else {
			//	cout << line << endl;
            eofInstrs.push_back(line); //add anything thats not record type 00 (data)
		}
	}
    controlFlow();
    if(disas){
        printProgram();
    }
    if(trampoline){
        insertTrampoline();
    }
    if(hexout){
        printHex();
    }
    if(funcOut){
        printByFunction();
    }


}

void insertAndDecode(unsigned int addr, unsigned int bytecode){

    if(bytecode >> 16){
        program.push_back(Instruction(addr, bytecode));
        program.back().decodeInstr32(bytecode);
    } else {
        program.push_back(Instruction(addr, bytecode));
        program.back().decodeInstr16(bytecode);
    }
}

unsigned int generateBL(unsigned int target_addr, unsigned int current_addr){
    // SHOULD INSERT SOME BOUNDS CHECKING
    int imm32 = target_addr - current_addr - 4;
    if(current_addr > target_addr){
        imm32 = imm32 - 2;
    }
    //imm32 = imm32 >> 1;
    unsigned int imm11,imm10, output;
    bool I2, I1, S, J1, J2; // GOT TO DEAL WITH the sign extension somehow, I think that is the problem
    imm11 = (imm32 >> 1) & 0b11111111111;
    imm10 = (imm32 >> 12) & 0x3ff;
    //imm10 = 0b0000000000;
    I2 = (imm32 >> 22) & 0b1;
    I1 = (imm32 >> 23) & 0b1;
    S = (imm32 >> 24) & 0b1;
    J2 = !(I2) ^ S;
    J1 = !(I1) ^ S;
    output = 0xf;
    output = output << 1;
    output = output | 0; //ya I know, will remove later
    output = output << 1;
    output = output | S;
    output = output << 10;
    output = output | imm10;
    output = output << 2;
    output = output | 0b11;
    output = output << 1;
    output = output | J1;
    output = output << 1;
    output = output | 0b1;
    output = output << 1;
    output = output | J2;
    output = output << 11;
    output = output | imm11;

    return output;
}

void controlFlow(){
    vector<Instruction*> mainbranches;
    int mainTop, mainBot, lastAddr;
    for(int i = 0; i < program.size(); i++){
        if(program[i].name == InstrType::BL && program[i].is32){ //could also look for ::B
            if(mainbranches.size() && (mainbranches.back()->addr + 0x4) != program[i].addr){
                //clear
                mainbranches.clear();
                mainbranches.push_back(&program[i]);
            } else {
                mainbranches.push_back(&program[i]);
            }
            if(mainbranches.size() >= 4){
                mainBot = i;
                break;
            }
        }
    }


    //build main, setup, loop, writeRAM, and readRAM functions
    Function *main = new Function("main");
    mainTop = mainBot-1;
    lastAddr = 0;
    while(program[mainTop].name != InstrType::PUSH){
        main->body.insert(main->body.begin(), &program[mainTop]);
        mainTop--;
    }
    main->body.insert(main->body.begin(), &program[mainTop]);

    while(program[mainBot].bytecode != 0x46c0){
        if(program[mainBot].name == InstrType::LDR){
            if(program[mainBot].Rn == Register::pc){
                if(program[mainBot].alignedAddr > lastAddr){
                    lastAddr = program[mainBot].alignedAddr;
                }
            }
        }
        main->body.push_back(&program[mainBot]);
        mainBot++;
    }
    main->body.push_back(&program[mainBot]);
    mainBot++;
    
    
    while(program[mainBot].addr <= lastAddr + 2){
        main->body.push_back(&program[mainBot]);
        mainBot++;
    }
    
    functions.insert(functions.begin(), *main);

    Function *setup = new Function("setup");
    Function *loop = new Function("loop");


    int setupAddr = getProgramIndex(mainbranches[1]->imm);
    //cout << hex << setupAddr << endl;
    fillFunction(setup, setupAddr);
    functions.push_back(*setup);

    int loopAddr = getProgramIndex(mainbranches[2]->imm);
    fillFunction(loop, loopAddr);
    functions.push_back(*loop);

}


void fillFunction(Function* func, int startAddr){
    int pos = startAddr;
    int lastAddr = 0;
    while(program[pos].name != InstrType::POP){
        if(program[pos].name == InstrType::LDR){
            if(program[pos].Rn == Register::pc){
                if(program[pos].alignedAddr > lastAddr){
                    lastAddr = program[pos].alignedAddr;
                }
            }
        }
        func->body.push_back(&program[pos]);
        pos++;
    }
    func->body.push_back(&program[pos]);
    pos++;
    while(program[pos].addr < lastAddr+2){
        func->body.push_back(&program[pos]);
        pos++;
    }

}

int getProgramIndex(unsigned int addr){
    unsigned int place = addr - 0x2000;
    if(addr >= program[place].addr){
        while(addr != program[place].addr){
            place++;
        }
    } else {
        while(addr != program[place].addr){
            place--;
        }
    }
    return place;
}

//make a function that takes in an BL Instruction, probably a reference, and changes the bytecode so it will jump to an input target addr

//hmmmmmmmmmmm this seems like something that should be put in instruction.cpp!!!!!!!!!!!!
void insertTrampoline(){
    vector<Instruction*> mainbranches;
    unsigned int writeRAMAddr = 0;
    unsigned int readRAMAddr = 0;
    unsigned int write32Addr = 0;
    unsigned int read32Addr = 0;
    unsigned int jumptoReadIndex = 0;
    unsigned int readToWriteJumpIndex = 0;
    unsigned int readToWriteJumpAddr = 0;
    for(int i = 0; i < program.size(); i++){
        if(program[i].name == InstrType::BL && program[i].is32){ //could also look for ::B
            if(mainbranches.size() && (mainbranches.back()->addr + 0x4) != program[i].addr){
                //clear
                mainbranches.clear();
                mainbranches.push_back(&program[i]);
            } else {
                mainbranches.push_back(&program[i]);
            }
            if(mainbranches.size() >= 4){
                break;
            }
        } else if(program[i].bytecode == 0xdead){
            if(program[i+1].bytecode == 0xbeef){
                int j = 0;
                while(program[i-j].name != InstrType::PUSH){
                    if(program[i-j].bytecode == 0x46c0){
                        if(program[i-j+1].bytecode == 0x46c0){
                            readToWriteJumpIndex = i-j;
                        }
                    }
                    j++;
                }
                //if(program[i-78].bytecode == 0xb5f0){
                    readRAMAddr = program[i-j].addr;
                //}
            }
        } else if(program[i].bytecode == 0xbeef){
            if(program[i+1].bytecode == 0xdead){
                int j = 0;
                while(program[i-j].name != InstrType::PUSH){
                    if(program[i-j].bytecode == 0x46c0){
                        if(program[i-j+1].bytecode == 0x46c0){
                            readToWriteJumpAddr = program[i-j].addr;
                        }
                    }
                    j++;
                }
                //if(program[i-74].bytecode == 0xb573){
                    writeRAMAddr = program[i-j].addr;
                //}
            }

        } else if(program[i].bytecode == 0x46c0){
            if(program[i+1].bytecode == 0x46c0){
                //found the spot to put BL to readRAM
                jumptoReadIndex = i;
            }
        }
    }

    if(!writeRAMAddr || !readRAMAddr || !jumptoReadIndex || !readToWriteJumpIndex || !readToWriteJumpAddr){
        cout << "ReadRAM: " << hex << readRAMAddr << endl;
        cout << "writeRAM: " << hex << writeRAMAddr << endl;
        cout << "jumptoReadIndex: " << jumptoReadIndex << endl;
        abort();
    }

/*
    for(int i = 0; i < mainbranches.size(); i++){
        cout << hex << mainbranches[i].addr << ", " << mainbranches[i].bytecode << ", ";
        printInstr(mainbranches[i]);
    }

*/
    /*
    target addr, current addr
    imm32 = signextend(s:I1:I2:imm10:imm11:0, 32)
    jumpaddr = currentadr + imm32
    imm32 = target_addr - current_addr
    target_addr = 0x2620 digital write
    target_addr = 0x22b0 delay
    */

    unsigned int jumptoendaddr = program.back().addr + 0x2;
    //cout << "last addr: " << hex << program.back().addr << endl;
    unsigned int nextaddr = program.back().addr + 0x2;

    unsigned int loopAndSaveAddr = nextaddr;
    insertAndDecode(nextaddr, 0xb5ff);
    insertAndDecode(nextaddr+=2, generateBL(mainbranches[2]->imm, nextaddr)); //BL to loop
    insertAndDecode(nextaddr+=4, generateBL(writeRAMAddr-0x2, nextaddr)); //BL to writeRAM
    //not sure why this one is bad and the others are fine, could be making the branch
    // decoding the branch, or my addresses are wrong
    insertAndDecode(nextaddr+=4, 0xbdff);
    insertAndDecode(nextaddr+=2, 0x46c0);
    insertAndDecode(nextaddr+=2, 0x46c0);

    unsigned int restoreOrSetupAddr = nextaddr + 0x2;
    insertAndDecode(nextaddr+=2, 0xb5ff);
    insertAndDecode(nextaddr+=2, generateBL(readRAMAddr, nextaddr));
    insertAndDecode(nextaddr+=4, 0x2800);
    insertAndDecode(nextaddr+=2, 0xd004);
    //insertAndDecode(nextaddr+=2, generateBL(loopAndSaveAddr, nextaddr)); //I need to generate a generic B instruction to branch to loop and save, but I need to first deal with the stack
                                                                        // and overwrite the LR so that loop and save will return to main, not here
                                                                        // OR just jump to main where loop is called and prayk
                                                                        // need to Mov maintoLoopaddr to a register, OR I need to B to right after the write in writeRAM
                                                                        // then mov that register's value to LR
                                                                        // pop all other registrers
                                                                        // mov SP so things work out, for now do BL
    insertAndDecode(nextaddr+=2, generateBL(mainbranches[2]->addr, nextaddr)); //BL should be ok because the program will never leave main. may still want to adjust stack, could continuously grow
    insertAndDecode(nextaddr+=4, 0x46c0);
    insertAndDecode(nextaddr+=2, 0x46c0);
    insertAndDecode(nextaddr+=2, 0xbdff);
    //insertAndDecode(nextaddr+=2, generateBL(mainbranches[1]->imm, nextaddr)); //not fully sure why I need to add 8
    insertAndDecode(nextaddr+=2, 0x46c0);
    insertAndDecode(nextaddr+=2, 0x46c0);
    //insertAndDecode(nextaddr+=4, 0xe7f9);
    insertAndDecode(nextaddr+=2, 0xe7f9);//switched this out for 2 instead of 4 b/c no branch
    insertAndDecode(nextaddr+=2, 0x46c0);
    insertAndDecode(nextaddr+=2, 0x46c0);
    insertAndDecode(nextaddr+=2, 0xb4ff);//push everything but lr
    insertAndDecode(nextaddr+=2, 0xbcff);//pop everything but pc(so no jump!)
    insertAndDecode(nextaddr+=2, 0x46c0);
    //insertAndDecode(nextaddr+=2, 0x46c0);
    //insertAndDecode(nextaddr+=2, 0x46c0);
    //insertAndDecode(nextaddr+=2, 0x46c0);

    //not sure what is wrong, something is up, try running the program some more in JLINKEXE, find bugs, profit


    // now change original jump

    //int loopaddr = getProgramIndex(mainbranches[1]->imm);
    //cout << hex << program[loopaddr].addr << endl;
    //mainbranches[1]->decodeInstr32(generateBL(restoreOrSetupAddr, mainbranches[1]->addr));
    mainbranches[2]->decodeInstr32(generateBL(loopAndSaveAddr, mainbranches[2]->addr));
    program[jumptoReadIndex].decodeInstr32(generateBL(restoreOrSetupAddr, program[jumptoReadIndex].addr));
    program.erase(program.begin()+jumptoReadIndex+1);
    program[readToWriteJumpIndex].decodeInstr32(generateBL(readToWriteJumpAddr, program[readToWriteJumpIndex].addr));
    program.erase(program.begin()+readToWriteJumpIndex+1);


   
/*
POP:
If the registers loaded include the PC, the word loaded for the PC is treated as an 
address and a branch occurs to that address. Bit[0] of the loaded value determines 
whether execution continues after this branch in ARM state or in Thumb state.
*/
    /*
    for(int i = 0; i < program.size(); i++){
        if(program[i].addr == 0x2ebe){
            program[i].decodeInstr32(generateBL(jumptoendaddr,program[i].addr));
            break;
        }
    }
    */
    checkAddrs();

    printHex();
}


void checkAddrs(){ //not working???
    try {
        unsigned int runningAddr = 0x2000;
        for(int i = 0; i < program.size(); i++){
            if(program[i].addr != runningAddr){
                cout << "bad Addr at " << hex << program[i].addr << endl;
                cout << "bytecode = " << hex << program[i].bytecode << endl;
                cout << "\t previous instrs:\n";
                for(int j = i - 10; j <= i ; j++){
                    cout << "\t" << hex << program[j].addr << ", " 
                        << hex << program[j].bytecode << endl;
                } 
                abort();
                //exit();
                SIGINT;
            }
            if(program[i].is32){
                runningAddr += 0x4;
            } else {
                runningAddr += 0x2;
            }
        }
        while(runningAddr % 16){ //pad NOPs to make the hex file look neat
            insertAndDecode(runningAddr+=0x2,0x46c0);
        }
    } catch(runtime_error e){
        cout << "aborting\n";
        abort();
    }

}


void decode(string instruction, int addr){

    //cout << "Instruction: " << instruction << endl;	
    unsigned long long bytecode = stoll(instruction,nullptr,16);
    //cout << "bytecode: " << hex << bytecode << endl;
    //instructions are weird. You human read them differently from how the machine reads them.
    unsigned singleOrDoubleInstr = bytecode >> 11;
    singleOrDoubleInstr = singleOrDoubleInstr & 0x1F;
    Instruction myInst1(0,0),myInst2(0,0); 
    if(singleOrDoubleInstr == 0b11101 || singleOrDoubleInstr == 0b11110 || singleOrDoubleInstr == 0b11111){
        //this is a 32 bit instruction
        myInst1.addr = addr;
        myInst1.decodeInstr32(bytecode);
        program.push_back(myInst1);
    } else {
        myInst1.addr = addr;
        myInst1.decodeInstr16(bytecode & 0xffff);
        myInst2.addr = addr + 0x2;
        myInst2.decodeInstr16(bytecode >> 16);
        program.push_back(myInst1);
        program.push_back(myInst2);
    }



}

void printByte(unsigned b){

    if(!(b >> 4)){
        cout << "0";
    } else {
        cout << uppercase << hex << (b >> 4);
    }

    if(!(b & 0xf)){
        cout << "0";
    } else {
        cout << uppercase << hex << (b & 0xf);
    }

}


void printHex(){
    unsigned currentBytecode = 0;
    unsigned checksum = 0;
    for(int i = 0; i < program.size(); ){
        cout << ":10" << hex << program[i].addr << "00";
        checksum = 0x10 + (program[i].addr >> 8) + (program[i].addr & 0xff) + 0x00; // ya ya I know, helps me understand whats going on
        currentBytecode = program[i].bytecode;
        int j = 16;
        while(j > 0){
            if(i >= program.size()){
                break; //uggghhh even more grose
            }
            if(program[i].is32){
                //print32inst
                checksum += ((program[i].bytecode >> 16) & 0xff) + ((program[i].bytecode >> 24) & 0xff);
                printByte((program[i].bytecode >> 16) & 0xff);
                printByte((program[i].bytecode >> 24) & 0xff);
                j-=2;
                if(!j){ //ugly but whatever
                    printByte((~(checksum % 256) + 1)&0xff);
                    cout << "\n";
                    cout << ":10" << (program[i].addr+2) << "00";
                    checksum = 0x10 + ((program[i].addr+2) >> 8) + ((program[i].addr+2) & 0xff) + 0x00; // ya ya I know, helps me understand whats going on
                    j = 16;
                }
                checksum += (program[i].bytecode & 0xff) + ((program[i].bytecode >> 8) & 0xff);
                printByte(program[i].bytecode & 0xff);
                printByte((program[i].bytecode >> 8) & 0xff);
                j-=2;
            } else {
                //print16inst
                checksum += (program[i].bytecode & 0xff) + ((program[i].bytecode >> 8) & 0xff);
                printByte(program[i].bytecode & 0xff);
                printByte(program[i].bytecode >> 8);
                j-=2;
            }
            i++;
        }
        printByte((~(checksum % 256) + 1)&0xff);
        cout << "\n";
    }
    for(int i = 0; i < eofInstrs.size(); i++){
        cout << eofInstrs[i];
        cout << endl;
    }
}


void printByFunction(){
    for(int i = 0; i < functions.size(); i++){
        cout << functions[i].name << endl;
        for(int j = 0; j < functions[i].body.size(); j++){
            cout << setw(8) << hex << functions[i].body[j]->addr << ":";
            cout << setw(7) << " ";
            if(functions[i].body[j]->is32){
                cout << setfill('0') << setw(4) << hex << (functions[i].body[j]->bytecode >> 16);
                cout << " ";
                cout << setw(4) << hex << (functions[i].body[j]->bytecode & 0xffff);
                //cout << setw(fw+8) << bitset<32>(program[i].bytecode); 
            } else {
                cout << setfill('0') << setw(4) << hex << functions[i].body[j]->bytecode;
                cout << "     ";
                //cout << setw(fw+8) << bitset<16>(program[i].bytecode);
            }
            cout << setfill(' ');
            cout << "       ";
            printInstr(*functions[i].body[j]);

        }
    }

}


void printProgram(){
    for(int i = 0; i < program.size(); i++){
        cout << setw(8) << hex << program[i].addr << ":";
        cout << setw(7) << " ";
        if(program[i].is32){
            cout << setfill('0') << setw(4) << hex << (program[i].bytecode >> 16);
            cout << " ";
            cout << setw(4) << hex << (program[i].bytecode & 0xffff);
            //cout << setw(fw+8) << bitset<32>(program[i].bytecode); 
        } else {
            cout << setfill('0') << setw(4) << hex << program[i].bytecode;
            cout << "     ";
            //cout << setw(fw+8) << bitset<16>(program[i].bytecode);
        }
        cout << setfill(' ');
        cout << "       ";
        printInstr(program[i]);
        //cout << setw(10);
        /*
           if(program[i].Rd != Register::na){
           cout << printRegister(program[i].Rd) << ", ";
           }
           if(program[i].Rn != Register::na){
           cout << printRegister(program[i].Rn) << ",";
           }
           if(program[i].Rm != Register::na){
           cout << printRegister(program[i].Rm) << " ";
		}
		if(program[i].jumpaddr){
			cout << "0x" << hex << program[i].jumpaddr;
		} else {
			cout << "#" << dec << program[i].imm;
			cout << setw(fw) << " ";
			cout << left  << "; 0x" << hex << program[i].imm  << right <<" ";
		}

		cout << endl;
		*/
	}

}

string printRegister(Register r){
	switch(r){
		case r0 : return "r0";
		case r1 : return "r1";
		case r2 : return "r2";
		case r3 : return "r3";
		case r4 : return "r4";
		case r5 : return "r5";
		case r6 : return "r6";
		case r7 : return "r7";
		case r8 : return "r8";
		case r9 : return "r9";
		case sl : return "sl";
		case fp : return "fp";
		case ip : return "ip";
		case sp : return "sp";
		case lr : return "lr";
		case pc : return "pc";
		case psr : return "psr";
		case primask : return "primask";
		case c : return "c";
		case na : return "na";
		default: return "reg error";
	}
}

string printOption(Option o){
    switch(o){
        case sy : return "sy";
        case un : return "un";
        case st : return "st";
        case unst : return "unst";
        default : return "option error";
    }
}

void printInstrName(string s){
			cout << setw(6) << left << s << right << " ";
}

void printFormatReg(Register r){
	cout << setw(3) << printRegister(r);
}

void print2Regs(Register r1, Register r2){
	printFormatReg(r1);
	cout << ",";
	printFormatReg(r2);
	cout << endl;
}

void print2RegsImm(Register r1, Register r2, unsigned imm){
	printFormatReg(r1);
	cout << ",";
	printFormatReg(r2);
	cout << ",";
	cout << setw(2) << "#" <<  dec << imm;
    if(imm > yesHex){
        cout << "     ; 0x" << hex << imm;
    }
	cout << endl;
}

void print3Regs(Register r1, Register r2, Register r3){
	printFormatReg(r1);
	cout << ",";
	printFormatReg(r2);
	cout << ",";
	printFormatReg(r3);
	cout << endl;
}

void bracket3Regs(Register r1, Register r2, Register r3){
	printFormatReg(r1);
	cout << ", [";
	cout << printRegister(r2);
	cout << ",";
	printFormatReg(r3);
	cout << "]";
}

void bracket2RegsImm(Register r1, Register r2, unsigned imm){
	printFormatReg(r1);
	cout << ", [";
	cout << printRegister(r2);
	cout << ", ";
	cout << "#" << dec << imm;
	cout << "]";

}

void regList(unsigned bits){
	bool hasPlaced = false;
	for(int i = 0; i <=7; i++){
		if(((bits >> i) & 0b1)){
			if(hasPlaced) {
				cout << ", ";
			}
			cout << printRegister(static_cast<Register>(i));
			hasPlaced = true;
		}
	}
			

}

void checkIfThenInstr(){
    if(ifThenNumber > 0){
        cout << ifThenModifier;
        ifThenNumber--;
    }
}


void printInstr(Instruction i){
	switch(i.name){
		case nop :
			printInstrName("nop");
			cout << endl;
			break;// "nop";
		case MOV_VIA_LSL : 
			printInstrName("movs");
            checkIfThenInstr();
			printFormatReg(i.Rd);
			cout << ",";
			printFormatReg(i.Rm);
			cout << endl;
			break;// "movs";
		case LSL : 
			printInstrName("lsls");
            checkIfThenInstr();
			if(i.hasImm){
				print2RegsImm(i.Rd,i.Rm,i.imm);
			} else {
				print2Regs(i.Rd, i.Rm);
			}
			break;// "lsls";
		case LSR :  
			printInstrName("lsrs");
            checkIfThenInstr();
			if(i.hasImm){
				print2RegsImm(i.Rd,i.Rm,i.imm);
			} else {
				print2Regs(i.Rd, i.Rn);
			}
			break;// "lsrs";
		case ASR :   
			printInstrName("asrs");
            checkIfThenInstr();
			if(i.hasImm){
				print2RegsImm(i.Rd,i.Rm,i.imm);
                if(i.imm > yesHex){
                    cout << "     ; 0x" << hex << i.imm;
                }
			} else {
				print2Regs(i.Rd, i.Rn);
			}
			break;// "asrs";
		case ADD :   
            if(i.Rd == Register::sp){
                if(i.hasImm){
                    printInstrName("add");
                    checkIfThenInstr();
                    printFormatReg(i.Rd);
                    cout << ", #" << dec << i.imm;
                    if(i.imm > yesHex){
                        cout << "     ; 0x" << hex << i.imm;
                    }
                    cout << endl;
                } else {
                    printInstrName("add");
                    checkIfThenInstr();
                    print2Regs(i.Rd, i.Rm);
                }
            } else if(((i.bytecode) >> 7) == 0b010001000) {
                printInstrName("add");
                checkIfThenInstr();
                print2Regs(i.Rd, i.Rm);
            } else if(((i.bytecode) >> 8) == 0x44){
                printInstrName("add");
                checkIfThenInstr();
                print2Regs(i.Rd, i.Rm);
            } else if(i.hasImm){
                printInstrName("add");
                checkIfThenInstr();
				print2RegsImm(i.Rd, i.Rm, i.imm);
            } else {
                printInstrName("adds");
                print3Regs(i.Rd, i.Rn, i.Rm);
            }
            break;// "adds";
        case SUB : 
            if(i.Rd == Register::sp){
                printInstrName("sub");
                checkIfThenInstr();
                printFormatReg(i.Rd);
                cout << ", #" << dec << i.imm;
                if(i.imm > yesHex){
                    cout << "     ; 0x" << hex << i.imm;
                }
                cout << endl;
            } else if(i.hasImm){
                printInstrName("subs");
                checkIfThenInstr();

                print2RegsImm(i.Rd, i.Rm, i.imm);

                //printFormatReg(i.Rd);
                //cout << ", #" << i.imm << endl;
                if(i.imm > yesHex){
                    cout << "     ; 0x" << hex << i.imm;
                }
            } else {
                printInstrName("subs");
                checkIfThenInstr();
                print3Regs(i.Rd, i.Rn, i.Rm);
            }
            break;// "subs";
        case ADD_3_I :   
            printInstrName("adds");
            checkIfThenInstr();
            print2RegsImm(i.Rd, i.Rn, i.imm);
            if(i.imm > yesHex){
                cout << "     ; 0x" << hex << i.imm;
            }
            break;// "adds";
        case SUB_3_I :   
            printInstrName("subs");
            checkIfThenInstr();
            //printFormatReg(i.Rd);
            //cout << ", #" << i.imm << endl;
            print2RegsImm(i.Rd, i.Rn, i.imm);
            if(i.imm > yesHex){
                cout << "     ; 0x" << hex << i.imm;
            }
            break;// "subs";
        case MOV : 
            if(i.Rd == Register::r8 && i.Rm == Register::r8){
                printInstrName("nop");
                cout << "           ; (mov r8, r8)" << endl;
            } else {
                if(i.hasImm){
                    printInstrName("movs");
                    checkIfThenInstr();
                    printFormatReg(i.Rd);
                    cout << ",";
                    cout << "#" << dec << i.imm;
                    if(i.imm > yesHex){
                        cout << "    ; 0x" << hex << i.imm;
                    }
                    cout << endl;
                } else {
                    printInstrName("mov");
                    checkIfThenInstr();
                    print2Regs(i.Rd, i.Rm);
                }
            }
            break;// "movs";
        case CMP :   
            printInstrName("cmp");
            checkIfThenInstr();
            if(i.hasImm){
                printFormatReg(i.Rn);
                cout << ",";
                cout << "#" << dec << i.imm;
                if(i.imm > yesHex){
                    cout << "     ; 0x" << hex << i.imm;
                }
                cout << endl;
            } else {
                print2Regs(i.Rn, i.Rm);
            }
            break;// "cmp";
        case ADD_8_I :   
            printInstrName("adds");
            checkIfThenInstr();
            //print2RegsImm(i.Rd, i.Rd, i.imm);
            printFormatReg(i.Rd);
            cout << ", #" << dec << i.imm;
            if(i.imm > yesHex){
                cout << "     ; 0x" << hex << i.imm;
            }
            cout << endl;
            break;// "adds";
        case SUB_8_I :   
            printInstrName("subs");
            checkIfThenInstr();
            printFormatReg(i.Rd);
            cout << ", #" << dec << i.imm;
            if(i.imm > yesHex){
                cout << "     ; 0x" << hex << i.imm;
            }
            cout << endl;
            //print2RegsImm(i.Rd, i.Rd, i.imm);
            break;// "subs";
        case AND_reg :   
            printInstrName("ands");
            checkIfThenInstr();
            print2Regs(i.Rd, i.Rm);
            break;// "ands";
        case EOR_reg :   
            printInstrName("eors");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
            break;// "eors";
        case LSL_reg :   
            printInstrName("lsls");
            checkIfThenInstr();
            print2Regs(i.Rd, i.Rm);
            break;// "lsls";
        case LSR_reg :   
			printInstrName("lsrs");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "lsrs";
		case ASR_reg :   
			printInstrName("asrs");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "asrs";
		case ADC_reg :   
			printInstrName("adcs");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "adcs";
		case SBC_reg :   
			printInstrName("sbcs");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "sbcs";
		case ROR_reg :   
			printInstrName("rors");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "rors";
		case TST_reg :   
			printInstrName("tst");
            checkIfThenInstr();
			print2Regs(i.Rn, i.Rm);
			break;// "tst";
		case RSB_I :   
			printInstrName("negs");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rn);
			break;// "rsbs";
		case CMP_reg :   
			printInstrName("cmp");
            checkIfThenInstr();
			print2Regs(i.Rn, i.Rm);
			break;// "cmp";
		case CMN_reg :   
			printInstrName("cmn");
            checkIfThenInstr();
			print2Regs(i.Rn, i.Rm);
			break;// "cmn";
		case ORR_reg :   
			printInstrName("orrs");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "orrs";
		case MUL_reg :   
			printInstrName("muls");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rn);
			break;// "muls";
		case BIC_reg :   
			printInstrName("bics");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "bics";
		case MVN_reg :   
			printInstrName("mvns");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rn);
			break;// "mvns";
		case BX :   
            if(i.Rm == Register::r0){
                printInstrName("bxns");
            } else {
                printInstrName("bx");
            }
			printFormatReg(i.Rm);
			cout << endl;
			break;// "bx";
		case BLX :   
			printInstrName("blx");
			printFormatReg(i.Rm);
			cout << endl;
			break;// "blx";
		case LDR :   
			printInstrName("ldr");
            checkIfThenInstr();
			if(i.hasImm){
				bracket2RegsImm(i.Rd,i.Rn,i.imm);
				if(i.Rn == Register::pc){
					cout << "    ; (0x" << hex <<  i.alignedAddr << ")";
                    //add alignedAddr/misc to Instruction.h so I can get it, useful for func building
				} else if(i.imm > yesHex){
                    cout << "    ; 0x" << hex << i.imm;
                }
			} else {
				bracket3Regs(i.Rd,i.Rn,i.Rm);
			}
			cout << endl;
			break;// "ldr";
		case STR :   
			printInstrName("str");
            checkIfThenInstr();
			if(i.hasImm){
				bracket2RegsImm(i.Rd,i.Rn,i.imm);
				if(i.imm > yesHex){
					cout << "    ; 0x" << hex << i.imm;
				}
			} else {
				bracket3Regs(i.Rd,i.Rn,i.Rm);
			}
			cout << endl;
			break;// "str";
		case STRH :   
			printInstrName("strh");
            checkIfThenInstr();
			if(i.hasImm){
				bracket2RegsImm(i.Rd,i.Rn,i.imm);
				if(i.imm > yesHex){
					cout << "    ; 0x" << hex << i.imm;
				}
			} else {
				bracket3Regs(i.Rd,i.Rn,i.Rm);
			}
			cout << endl;
			break;// "strh";
		case STRB :   
			printInstrName("strb");
            checkIfThenInstr();
			if(i.hasImm){
				bracket2RegsImm(i.Rd,i.Rn,i.imm);
				if(i.imm > yesHex){
					cout << "    ; 0x" << hex << i.imm;
				}
			} else {
				bracket3Regs(i.Rd,i.Rn,i.Rm);
			}
			cout << endl;
			break;// "strb";
		case LDRSB :   
			printInstrName("ldrsb");
            checkIfThenInstr();
			bracket3Regs(i.Rd,i.Rn,i.Rm);
			cout << endl;
			break;// "ldrsb";
		case LDRH :   
			printInstrName("ldrh");
            checkIfThenInstr();
			if(i.hasImm){
                bracket2RegsImm(i.Rd,i.Rn,i.imm);
                if(i.imm > yesHex){
                    cout << "    ; 0x" << hex << i.imm;
                }
			} else {
				bracket3Regs(i.Rd,i.Rn,i.Rm);
			}
			cout << endl;
			break;// "ldrh";
		case LDRB :   
			printInstrName("ldrb");
            checkIfThenInstr();
			if(i.hasImm){
				bracket2RegsImm(i.Rd,i.Rn,i.imm);
                if(i.imm > yesHex){
                    cout << "    ; 0x" << hex << i.imm;
                }
			} else {
				bracket3Regs(i.Rd,i.Rn,i.Rm);
			}
			cout << endl;
			break;// "ldrb";
		case LDRSH :   
			printInstrName("ldrsh");
            checkIfThenInstr();
			bracket3Regs(i.Rd,i.Rn,i.Rm);
			cout << endl;
			break;// "ldrsh";
		case ADR :   
            if(i.Rn == Register::pc){
                printInstrName("add");
                checkIfThenInstr();
                cout << printRegister(i.Rd) << ", " << printRegister(i.Rn) << ", #" << dec << i.imm; // << ", " << i.Rn << ", #" << dec << i.imm;
                cout << "; (adr " << printRegister(i.Rd) << ", 0x" << hex << ((i.addr + 4) - (i.addr % 4) + i.imm) << ")" <<  endl;
            } else {
                printInstrName("adr");
                checkIfThenInstr();
                printFormatReg(i.Rd);
                cout << ", 0x" << hex << i.imm << endl;
            }
			break;// "adr";
		case STM :   
			printInstrName("stmia");
            checkIfThenInstr();
			printFormatReg(i.Rn);
			cout << "!, {";
			regList(i.imm);
			cout << "}" << endl;
			break;// "stm";
		case LDM :   
			printInstrName("ldmia");
            checkIfThenInstr();
			printFormatReg(i.Rn);
            if(!((i.imm >> (i.Rn)) & 0b1)){ //overly complex way of checking if Rn is in the reglist, if so, don't add the !
                cout << "!";
            }
			cout << ", {";
			regList(i.imm);
			cout << "}";
			cout << endl;
			break;// "ldm";
		case CBNZ :   
			printInstrName("cbnz");
			//not supported by cortex-m0+
			cout << endl;
			break;// "cbnz";
		case CBZ :   
			printInstrName("cbz");
			//not supported by cortex-m0+
			cout << endl;
			break;// "cbz";
		case SXTH :   
			printInstrName("sxth");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "sxth";
		case SXTB :   
			printInstrName("sxtb");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "sxtb";
		case UXTH :   
			printInstrName("uxth");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "uxth";
		case UXTB :   
			printInstrName("uxtb");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "uxtb";
		case PUSH :   
			printInstrName("push");
            checkIfThenInstr();
			cout << "{";
            if(i.imm){
                regList(i.imm);
                if((i.bytecode >> 8) & 0b1){
                    cout << ",";
                    cout << printRegister(Register::lr); 
                }
            } else if((i.bytecode >> 8) & 0b1){
                cout << printRegister(Register::lr);
            }
            cout << "}";
			cout << endl;
			break;// "push";
		case SETEND :   
			printInstrName("setend");
            checkIfThenInstr();
			cout << endl;
			break;// "setend";
		case CPS :   
            { 
            bool affectA = (i.bytecode >> 2) & 0b1;
            bool affectI = (i.bytecode >> 1) & 0b1;
            bool affectF = i.bytecode & 0b1;
            if((i.bytecode >> 4) & 0b1){
                printInstrName("cpsid");//cps disable
                if(affectA){
                    cout << " a ";
                } else if(affectI){
                    cout << " i ";
                } else if(affectF){
                   cout << " f ";
                } else {
                    cout << "ummm problem";
                }
            } else {
                printInstrName("cpsie");
                if(affectA){
                    cout << " a ";
                } else if(affectI){
                    cout << " i ";
                } else if(affectF){
                   cout << " f ";
                } else {
                    cout << "ummm problem";
                }
            }
            
			cout << endl;
			break;// "cps";
            }
		case REV :   
			printInstrName("rev");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			break;// "rev";
		case REV16 :   
			printInstrName("rev16");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			cout << endl;
			break;// "rev16";
		case REVSH :   
			printInstrName("revsh");
            checkIfThenInstr();
			print2Regs(i.Rd, i.Rm);
			cout << endl;
			break;// "revsh";
		case POP :   
			printInstrName("pop");
            checkIfThenInstr();
			cout << "{";
            if(i.imm){
                regList(i.imm);
            }
            if((i.bytecode >> 8) & 0b1){
                if(i.imm){
                    cout << ",";
                }
                cout << " " << printRegister(Register::pc) << "}";
            } else {
                cout << "}";
            }
			cout << endl;
			break;// "pop";
		case BKPT :   
			printInstrName("bkpt"); //breakpoint always executes, no need to ifthencheck
			cout << "#" << dec << i.imm;
			cout << endl;
			break;// "bkpt";
		case IT :   
            {
			cout << "it";
            bool firstCond_0 = i.cond & 0b1;
            bool mask3 = (i.imm >> 3) & 0b1;
            bool mask2 = (i.imm >> 2) & 0b1;
            bool mask1 = (i.imm >> 1) & 0b1;
            bool mask0 = i.imm & 0b1;


            if(mask0){
                //3 instrs
                if(mask3 & firstCond_0){
                    cout << "t";
                } else {
                    cout << "e";
                }

                if(mask2 & firstCond_0){
                    cout << "t";
                } else {
                    cout << "e";
                }

                if(mask1 & firstCond_0){
                    cout << "t";
                } else {
                    cout << "e";
                }
                ifThenNumber = 4;
            } else {
                // two or less instrs
                if(mask1){
                    //2 instrs
                    if(mask3 & firstCond_0){
                        cout << "t";
                    } else {
                        cout << "e";
                    }

                    if(mask2 & firstCond_0){
                        cout << "t";
                    } else {
                        cout << "e";
                    }
                    ifThenNumber = 3;
                } else {
                    // 1 or none instrs
                    if(mask2){
                        // 1 instrs
                        if(mask3 & firstCond_0){
                            cout << "t";
                        } else {
                            cout << "e";
                        }
                        ifThenNumber = 2;
                    } else {
                        // no instrs
                        ifThenNumber = 1;
                    }
                }
            }
            cout << "   ";
                switch(i.cond){
                    case Condition::eq: //0
                        printInstrName("eq");
                        ifThenModifier = "eq";
                        break;
                    case Condition::ne: //1
                        printInstrName("ne");
                        ifThenModifier = "ne";
                        break;
                    case Condition::cs: //2
                        printInstrName("cs");
                        ifThenModifier = "cs";
                        break;
                    case Condition::cc: //3
                        printInstrName("cc");
                        ifThenModifier = "cc";
                        break;
                    case Condition::hs: //4
                        printInstrName("hs");
                        ifThenModifier = "hs";
                        break;
                    case Condition::lo: //5
                        printInstrName("lo");
                        ifThenModifier = "lo";
                        break;
                    case Condition::mi: //6
                        printInstrName("mi");
                        ifThenModifier = "mi";
                        break;
                    case Condition::pl: //7
                        printInstrName("pl");
                        ifThenModifier = "pl";
                        break;
                    case Condition::vs: //8
                        printInstrName("vs");
                        ifThenModifier = "vs";
                        break;
                    case Condition::vc: //9
                        printInstrName("vc");
                        ifThenModifier = "vc";
                        break;
                    case Condition::hi: //a
                        printInstrName("hi");
                        ifThenModifier = "hi";
                        break;
                    case Condition::lt: //b
                        printInstrName("lt");
                        ifThenModifier = "lt";
                        break;
                    case Condition::ge: //c
                        printInstrName("ge");
                        ifThenModifier = "ge";
                        break;
                    case Condition::ls: //d
                        printInstrName("ls");
                        ifThenModifier = "ls";
                        break;
                    case Condition::gt: //e
                        printInstrName("gt");
                        ifThenModifier = "gt";
                        break;
                    case Condition::le: //f
                        printInstrName("le");
                        ifThenModifier = "le";
                        break;
                    default:
                        printInstrName("error");
                        break;
                }
			cout << endl;
			break;// "it";
            }
		case YIELD :   
			printInstrName("yield");
			cout << endl;
			break;// "yield";
		case WFE :   
			printInstrName("wfe");
			cout << endl;
			break;// "wfe";
		case WFI :   
			printInstrName("wfi");
			cout << endl;
			break;// "wfi";
		case SEV :   
			printInstrName("sev");
			cout << endl;
			break;// "sev";
		case UDF :   
			printInstrName("udf");
			cout << endl;
			break;// "udf";
		case SVC :   
			printInstrName("svc");
            checkIfThenInstr();
			cout << "#" << dec << i.imm;
			cout << endl;
			break;// "svc";
		case B :   
			if(!i.is32){
                switch(i.cond){
                    case Condition::eq: //0
                        printInstrName("beq.n");
                        break;
                    case Condition::ne: //1
                        printInstrName("bne.n");
                        break;
                    case Condition::cs: //2
                        printInstrName("bcs.n");
                        break;
                    case Condition::cc: //3
                        printInstrName("bcc.n");
                        break;
                    case Condition::hs: //4
                        printInstrName("bhs.n");
                        break;
                    case Condition::lo: //5
                        printInstrName("blo.n");
                        break;
                    case Condition::mi: //6
                        printInstrName("bmi.n");
                        break;
                    case Condition::pl: //7
                        printInstrName("bpl.n");
                        break;
                    case Condition::vs: //8
                        printInstrName("bvs.n");
                        break;
                    case Condition::vc: //9
                        printInstrName("bvc.n");
                        break;
                    case Condition::hi: //a
                        printInstrName("bhi.n");
                        break;
                    case Condition::lt: //b
                        printInstrName("blt.n");
                        break;
                    case Condition::ge: //c
                        printInstrName("bge.n");
                        break;
                    case Condition::ls: //d
                        printInstrName("bls.n");
                        break;
                    case Condition::gt: //e
                        printInstrName("bgt.n");
                        break;
                    case Condition::le: //f
                        printInstrName("ble.n");
                        break;
                    default:
                        printInstrName("b.n");
                        break;
                }
			} else {
				printInstrName("b.w");
			}
			cout << "0x" << hex << i.imm;
			cout << endl;
			break;// "b";
		case BL :   
			printInstrName("bl");
			cout << "0x" << hex << i.imm;
			cout << endl;
			break;// "bl";
        case DMB:
            printInstrName("dmb");
            cout << printOption(i.opt);
            cout << endl;
            break;
        case DSB:
            printInstrName("dsb");
            cout << printOption(i.opt);
            cout << endl;
            break;
        case ISB:
            printInstrName("isb");
            cout << printOption(i.opt);
            cout << endl;
            break;
        case MRS:
            printInstrName("mrs");
            printFormatReg(i.Rd);
            cout << ", PRIMASK";
            cout << endl;
            break;
        case MSR:
            printInstrName("msr");
            cout << endl;
            break;
		default:   
			printInstrName("no type");
			cout << endl;
			break;// "no type";
	}

}
