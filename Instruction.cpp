#include "Instruction.h"
#include <iostream>
#include <bitset>


using namespace std;




Instruction::Instruction(unsigned addr, unsigned int bytecode){
    this->name = InstrType::nop;
    this->bytecode = bytecode;
    this->addr = addr;
}


Instruction::Instruction(InstrType name, unsigned addr, unsigned int bytecode){
	this->name = name; 
	this->bytecode = bytecode;
	this->addr = addr;
}


void Instruction::decodeInstr(string textOfInstruction){
	unsigned long long bytecode = stoi(textOfInstruction,nullptr,16);
	cout << "bytecode: " << hex << bytecode << endl;
	//this->decodeInstr(bytecode);
}


void Instruction::decodeInstr32(unsigned long bytecode){
	this->bytecode = bytecode;
	this->is32 = true;
	// selectively add 32 bit intrs as I need them, no need to do full thing atm
    if((bytecode >> 27) != 0b11110){
        this->name == InstrType::nop;
    } else if((bytecode >> 4) == 0xf3bf8f4){
        this->name = InstrType::DSB;
        this->opt = static_cast<Option>(bytecode & 0xf);
    } else if((bytecode >> 4) == 0xf3bf8f5){
        this->name = InstrType::DMB;
        this->opt = static_cast<Option>(bytecode & 0xf);
    } else if((bytecode >> 4) == 0xf3bf8f6){
        this->name = InstrType::ISB;
        this->opt = static_cast<Option>(bytecode & 0xf);
    } else if((bytecode >> 21) == 0b11110011111){
        this->name = InstrType::MRS;
        this->Rd = static_cast<Register>((bytecode >> 8) & 0xf);
        this->Rn = Register::primask;
	} else if(bytecode >> 28 == 0xf){
		//assuming bl or blx instruction for now
        unsigned rightHalf = bytecode & 0xffff;
		bool bit12 = (rightHalf >> 12) & 0b1; //should marco this
        bool bit14 = (rightHalf >> 14) & 0b1;
        if(bit14){ //this is a bl or blx instruction
            bool j1 = (rightHalf >> 13) & 0b1;
            bool j2 = (rightHalf >> 11) & 0b1;
            bool S = (bytecode >> 26) & 0b1;
            bool I1 = !(j1 ^ S);
            bool I2 = !(j2 ^ S);
            int imm32 = 0;
            unsigned imm10 = (bytecode >> 16) & 0x3ff;
            unsigned imm11 = bytecode & 0x7ff; 
            if(bit12){
                //bl
                //imm32 = SignExtend(S:I1:I2:imm10:imm11:'0', 32);
                imm32 = S;
                imm32 = imm32 << 1;
                imm32 = imm32 | I1;
                imm32 = imm32 << 1;
                imm32 = imm32 | I2;
                imm32 = imm32 << 10;
                imm32 = imm32 | imm10;
                imm32 = imm32 << 11;
                imm32 = imm32 | imm11;
                imm32 = imm32 << 1;
                if(imm32 >> 24){
                    imm32 = imm32 | 0xfe000000;
                }
                // add PC, PC = addr + 4
                this->imm = imm32 + this->addr + 4;
                this->name = InstrType::BL;
            } else {
                //blx
            }
        } else { // this is a b or b condition (wide version)
            if(bit12){ // this is b.w
                bool j1 = (rightHalf >> 13) & 0b1;
                bool j2 = (rightHalf >> 11) & 0b1;
                bool S = (bytecode >> 26) & 0b1;
                bool I1 = !(j1 ^ S);
                bool I2 = !(j2 ^ S);
                int imm32 = 0;
                unsigned imm10 = (bytecode >> 16) & 0x3ff;
                unsigned imm11 = bytecode & 0x7ff; 
                imm32 = S;
                imm32 = imm32 << 1;
                imm32 = imm32 | I1;
                imm32 = imm32 << 1;
                imm32 = imm32 | I2;
                imm32 = imm32 << 10;
                imm32 = imm32 | imm10;
                imm32 = imm32 << 11;
                imm32 = imm32 | imm11;
                imm32 = imm32 << 1;
                if(imm32 >> 24){
                    imm32 = imm32 | 0xfe000000;
                }
                // add PC, PC = addr + 4
                this->imm = imm32 + this->addr + 4;
                this->name = InstrType::B;
            } else { // this is b.cond.w
                int imm32 = ((bytecode >> 26) & 0b1) << 1; //S
                imm32 = (imm32 | ((bytecode >> 11) & 0b1)) << 1; //J2
                imm32 = (imm32 | ((bytecode >> 13) & 0b1)) << 6; //J1
                imm32 = (imm32 | ((bytecode >> 16) & 0x2f)) << 11; //imm6
                imm32 = (imm32 | ((bytecode & 0x07ff))) << 1; //imm11
                if(imm32 >> 24){ // sign extend
                    imm32 = imm32 | 0xfe000000;
                }
                this->imm = imm32 + this->addr + 4;
                this->name = InstrType::B;
                this->cond = static_cast<Condition>((bytecode >> 22) & 0xf);
                
            }
       }
    }
}


void Instruction::decodeInstr16(unsigned long bytecode){
    //get rid of this if
    this->bytecode = bytecode;
    this->is32 = false;
    //cout << hex << instrs[i] << "\t";
    unsigned opcode = bytecode >> 10;
    //cout << hex << opcode << "\t";
    if(opcode >> 4 == 0b00){
        //shift(immediate), add, subtract, move, and compare
        opcode = bytecode >> 9; //weird but what the arm doc says
        opcode = opcode & 0x3fff; 
        if(opcode >> 2 == 0b000){
            if((bytecode >> 6) == 0b0000000000){
                unsigned maybeMov = bytecode & 0xff;
                maybeMov = maybeMov >> 6;
                //if(maybeMov == 0b000){
                //If opcode is 0b00000 and 8:6 are 0b000, this is an encoding for MOV
                this->name = InstrType::MOV_VIA_LSL;
                this->Rd = static_cast<Register>(bytecode & 0b111);
                this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
                //}
            } else {
                //Logical Shift Left
                this->name = InstrType::LSL;
                this->Rd = static_cast<Register>(bytecode & 0b111);
                this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
                this->hasImm = true;
                this->imm = (bytecode >> 6) & 0b11111;
            }
        } else if(opcode >> 2 == 0b001){
            //Logical Shift Right
            this->name = InstrType::LSR;
            this->Rd = static_cast<Register>(bytecode & 0b111);
            this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
            this->hasImm = true;
            this->imm = ((bytecode >> 6) & 0b11111);
            if(!this->imm){ //weird but this is what ARMUtils.h says to do, makes it go from 1-32
                this->imm = 32;
            }

        } else if(opcode >> 2 == 0b010){
            //Arithmentic Shift Right
            this->name = InstrType::ASR;
            this->Rd = static_cast<Register>(bytecode & 0b111);
            this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
            this->hasImm = true;
            this->imm = (bytecode >> 6) & 0b11111;
            if(!this->imm){ //weird but this is what ARMUtils.h says to do, makes it go from 1-32
                this->imm = 32;
            }
        } else if(opcode == 0b01100){
            //Add register
            this->name = InstrType::ADD;
            this->Rd = static_cast<Register>(bytecode & 0b111);
            this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
            this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
        } else if(opcode == 0b01101){
            //Sub register
            this->name = InstrType::SUB;
            this->Rd = static_cast<Register>(bytecode & 0b111);
            this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
            this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
        } else if(opcode == 0b01110){
			//Add register
			this->name = InstrType::ADD_3_I;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
			this->hasImm = true;
			this->imm = (bytecode >> 6) & 0b111;
		} else if(opcode == 0b01111){
			//Subtract 3-bit immediate
			this->name = InstrType::SUB_3_I;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
			this->hasImm = true;
			this->imm = (bytecode >> 6) & 0b111;
		} else if(opcode >> 2 == 0b100){
			//MOVe immediate
			this->name = InstrType::MOV;
			this->hasImm = true;
			this->imm = (bytecode & 0xff);
			this->Rd = static_cast<Register>((bytecode >> 8) & 0b111);
		} else if(opcode >> 2 == 0b101){
			//Compare immediate 
			this->name = InstrType::CMP;
			this->hasImm = true;
			this->imm = (bytecode & 0xff);
			this->Rn = static_cast<Register>((bytecode >> 8) & 0b111);
		} else if(opcode >> 2 == 0b110){
			//Add 8 bit immediate
			this->name = InstrType::ADD_8_I;
			this->hasImm = true;
			this->imm = (bytecode & 0xff);
			this->Rd = static_cast<Register>((bytecode >> 8) & 0b111);
			this->Rn = static_cast<Register>((bytecode >> 8) & 0b111);
		} else if(opcode >> 2 == 0b111){
			//Subtract 8-bit immediate
			this->name = InstrType::SUB_8_I;
			this->hasImm = true;
			this->imm = (bytecode & 0xff);
			this->Rd = static_cast<Register>((bytecode >> 8) & 0b111);
			this->Rn = static_cast<Register>((bytecode >> 8) & 0b111);
		} else {
			cout << "Arith Instr Error at instrAddr: " << hex << 
				this->addr << " opcode: " << bitset<8>(opcode)
				<< endl;
			this->name = InstrType::nop;
		}
	} else if(opcode == 0b010000){
		//data-processing for registers
		opcode = bytecode & 0x03ff;
		opcode = opcode >> 6;
		switch(opcode){
			case 0b0000:
				//Bitwise AND
				this->name = InstrType::AND_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b0001:
				//Bitwise XOR
				this->name = InstrType::EOR_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
                break;
			case 0b0010:
				//Logical Shift Left register
				this->name = InstrType::LSL_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b0011:
				//Logical Shift Right register AND
				this->name = InstrType::LSR_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b0100:
				//Arithmetic Shift Right
				this->name = InstrType::ASR_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b0101:
				//Add with Carry
				this->name = InstrType::ADC_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b0110:
				//Sub with Carry
				this->name = InstrType::SBC_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b0111:
				//Rotate Right
				this->name = InstrType::ROR_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b1000:
				//Test
				this->name = InstrType::TST_reg;	
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b1001:
				//Reverse Subtract from 0
				this->name = InstrType::RSB_I;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b1010:
				//Compare High Registers
				this->name = InstrType::CMP_reg;
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b1011:
				//Compare Negative
				this->name = InstrType::CMN_reg;
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b1100:
				//Bitwise OR
				this->name = InstrType::ORR_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b1101:
				//Multiply 2 regs
				this->name = InstrType::MUL_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b1110:
				//Bitwise bit clear
				this->name = InstrType::BIC_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>(bytecode & 0b111);
				this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			case 0b1111:
				//Bitwise Not
				this->name = InstrType::MVN_reg;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				break;
			default:
				//error
				this->name = InstrType::nop;
				break;
		}
	} else if(opcode == 0b010001){
		// special data instructions and branch and exchange
		opcode = bytecode & 0x03FF;
		opcode = opcode >> 6;
		if(opcode == 0b0000){
			//Add Low Registers
			this->name = InstrType::ADD;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rn = static_cast<Register>(bytecode & 0b111);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode == 0b0001 || opcode == 0b0010 || opcode == 0b0011){
			//Add High Registers
			this->name = InstrType::ADD;
			unsigned fullRdn = (bytecode >> 7) &0b1;
			fullRdn = (fullRdn << 3) | (bytecode & 0b111);
			this->Rd = static_cast<Register>(fullRdn);
			this->Rn = static_cast<Register>(fullRdn);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0xf);
        } else if((opcode >> 1) == 0b001){
            this->name = InstrType::ADD;
            this->Rd = Register::sp;
            this->Rm = static_cast<Register>((bytecode >> 3) & 0xf);
		} else if(opcode == 0b0101 || opcode == 0b0110 || opcode == 0b0111 || opcode == 0b0100){
			//Compare High Registers
			this->name = InstrType::CMP;
			unsigned fullRdn = (bytecode >> 7) &0b1;
			fullRdn = (fullRdn << 3) | (bytecode & 0b111);
			this->Rn = static_cast<Register>(fullRdn);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0xf);
		} else if(opcode == 0b1000){
			//Move Low Registers
			this->name = InstrType::MOV;
            this->Rd = static_cast<Register>(bytecode & 0b111);
            this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode == 0b1001 || opcode == 0b1010 || opcode == 0b1011){
			//Move High Registers
			this->name = InstrType::MOV;
			unsigned fullRdn = (bytecode >> 7) &0b1;
			fullRdn = (fullRdn << 3) | (bytecode & 0b111);
			this->Rd = static_cast<Register>(fullRdn);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0xf);
		} else if(opcode == 0b1100 || opcode == 0b1101){
			//Branch and Exchange
			this->name = InstrType::BX;
			this->Rm = static_cast<Register>((bytecode >> 3) & 0xf);
		} else if(opcode == 0b1110 || opcode == 0b1111){
			//Branch with and Exchange
            if(this->bytecode & 0b111){
                this->name = InstrType::nop;
            } else {
                this->name = InstrType::BLX;
                this->Rm = static_cast<Register>((bytecode >> 3) & 0xf);
            }
		} else {
			//special data instructions error
			this->name = InstrType::nop;
		}
	} else if(opcode >> 1 == 0b01001){
		//Load from Literal Pool
		this->name = InstrType::LDR; //this is literal LDR 
		this->hasImm = true;
		this->imm = ((bytecode & 0xff) << 2);// + this->addr + 0x2; // need to shift this to make it a biggert number!!!!!!!!!!!!!!
		this->Rd = static_cast<Register>((bytecode >> 8) & 0b111);
		this->Rn = Register::pc; 
	} else if((opcode >> 2 == 0b0101) || (opcode >> 3 == 0b011) ||
			(opcode >> 3 == 0b100)){
		//Load/store single data item
		unsigned opA = (bytecode >> 12) & 0xf;
		unsigned opB = (bytecode >> 9) & 0b111;
		if(opA == 0b0101){
			switch(opB){
				case 0b000:
					//Store Register
					this->name = InstrType::STR;
					this->Rd = static_cast<Register>(bytecode & 0b111);
					this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
					this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
					break;
				case 0b001:
					//Store Register Halfword
					this->name = InstrType::STRH;
					this->Rd = static_cast<Register>(bytecode & 0b111);
					this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
					this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
					break;
				case 0b010:
					//Store Register Byte
					this->name = InstrType::STRB;
					this->Rd = static_cast<Register>(bytecode & 0b111);
					this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
					this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
					break;
				case 0b011:
					//Load Register Signed Byte
					this->name = InstrType::LDRSB;
					this->Rd = static_cast<Register>(bytecode & 0b111);
					this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
					this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
					break;
				case 0b100:
					//Load Register
					this->name = InstrType::LDR;
					this->Rd = static_cast<Register>(bytecode & 0b111);
					this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
					this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
					break;
				case 0b101:
					//Load Register Halfword
					this->name = InstrType::LDRH;
					this->Rd = static_cast<Register>(bytecode & 0b111);
					this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
					this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
					break;
				case 0b110:
					//Load Register Byte
					this->name = InstrType::LDRB;
					this->Rd = static_cast<Register>(bytecode & 0b111);
					this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
					this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
					break;
				case 0b111:
					//Load Register Signed Halfword
					this->name = InstrType::LDRSH;
					this->Rd = static_cast<Register>(bytecode & 0b111);
					this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
					this->Rm = static_cast<Register>((bytecode >> 6) & 0b111);
					break;
				default:
					this->name = InstrType::nop;
					break;
			}
		} else if(opA == 0b0110){
            opB = opB >> 2;
			if(!opB){
				//Store Register
				this->name = InstrType::STR;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				this->hasImm = true;
				this->imm = ((bytecode >> 6) & 0x1f) << 2;
			} else {
				//Load Register
				this->name = InstrType::LDR;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				this->hasImm = true;
				this->imm = ((bytecode >> 6) & 0x1f) << 2;
			}
		} else if(opA == 0b0111){
            opB = opB >> 2;
			if(!opB){
				//Store Register Byte
				this->name = InstrType::STRB;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				this->hasImm = true;
				this->imm = ((bytecode >> 6) & 0x1f);
			} else {
				//Load Register Byte
				this->name = InstrType::LDRB;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				this->hasImm = true;
				this->imm = ((bytecode >> 6) & 0x1f);
			}
		} else if(opA == 0b1000){
            opB = opB >> 2;
			if(!opB){
				//Store Register Halfword
				this->name = InstrType::STRH;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				this->hasImm = true;
				this->imm = ((bytecode >> 6) & 0x1f) << 1;
			} else {
				//Load Register Halfword
				this->name = InstrType::LDRH;
				this->Rd = static_cast<Register>(bytecode & 0b111);
				this->Rn = static_cast<Register>((bytecode >> 3) & 0b111);
				this->hasImm = true;
				this->imm = ((bytecode >> 6) & 0x1f) << 1;
			}
		} else if(opA == 0b1001){
            opB = opB >> 2;
			if(!opB){
				//Store Register SP relative
				this->name = InstrType::STR;
				this->hasImm = true;
				this->imm = (bytecode & 0xff) << 2;
				this->Rd = static_cast<Register>((bytecode >> 8) & 0b111);
				this->Rn = Register::sp;
			} else {
				//Load Register Sp relative
				this->name = InstrType::LDR;
				this->hasImm = true;
				this->imm = (bytecode & 0xff) << 2;
				this->Rd = static_cast<Register>((bytecode >> 8) & 0b111);
				this->Rn = Register::sp;
			}
		} else {
			//Error
			this->name = InstrType::nop;
		}
	} else if(opcode >> 1 == 0b10100){
		//Generate PC-relative address, ADR
		this->name = InstrType::ADR;
        this->imm = ((bytecode & 0xff) << 2);
        this->Rd = static_cast<Register>((bytecode >> 8) & 0b111);
        this->Rn = Register::pc;
        this->hasImm = true;
	} else if(opcode >> 1 == 0b10101){
		//Generate SP-relative address, ADD(SP plus immediate
		this->name = InstrType::ADD;
		this->hasImm = true;
		this->imm = (bytecode & 0xff) << 2;
		this->Rd = static_cast<Register>((bytecode >> 8) & 0b111);
        this->Rm = Register::sp;
	} else if(opcode >> 2 == 0b1011){
		//MISC 16 bit instructions
		opcode = bytecode & 0x0fff;
		opcode = opcode >> 5;
		if(opcode >> 2 == 0){
			//Add Immediate to SP
			this->name = InstrType::ADD;
			this->Rd = Register::sp;
			this->Rn = Register::sp;
			this->hasImm = true;
			this->imm = (bytecode & 0x7f) << 2;
		} else if(opcode >> 2 == 0b1){
			//Subtract Immediate from SP
			this->name = InstrType::SUB;
			this->Rd = Register::sp;
			this->Rn = Register::sp;
			this->hasImm = true;
			this->imm = (bytecode & 0x7f) << 2;
		} else if(opcode >> 3 == 0b1){
			//Compare and Branch on Zero
			this->name = InstrType::CBZ;
			this->Rn = static_cast<Register>(bytecode & 0b111);
			unsigned i = bytecode >> 9;
			i = (i << 5) | ((bytecode >> 3) & 0x1f);
			this->hasImm = true;
			this->imm = i << 1;
		} else if(opcode >> 1 == 0b001000){
			//Signned Extend Halfword
			this->name = InstrType::SXTH;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode >> 1 == 0b001001){
			//Signed Extend Byte
			this->name = InstrType::SXTB;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode >> 1 == 0b001010){
			//Unsigned Extend Halfword
			this->name = InstrType::UXTH;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode >> 1 == 0b001011){
			//Unsigned Extend Byte
			this->name = InstrType::UXTB;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode >> 3 == 0b0011){
			//Compare and Branch on Zero
			this->name = InstrType::CBZ;
			this->Rn = static_cast<Register>(bytecode & 0b111);
			unsigned i = bytecode >> 9;
			i = (i << 5) | ((bytecode >> 3) & 0x1f);
			this->hasImm = true;
			this->imm = i << 1;
		} else if(opcode >> 4 == 0b010){
			//Push Multiple Registers
			this->name = InstrType::PUSH;
			this->hasImm = true;
			this->imm = bytecode & 0xff;
		} else if(opcode == 0b0110010){
			//Set Endianness
			this->name = InstrType::SETEND;
			this->hasImm = true;
			this->imm = (bytecode >> 3) & 0b1;
		} else if(opcode == 0b0110011){
			//Change Processor State
			this->name = InstrType::CPS;
			this->hasImm = true;
			this->imm = bytecode & 0x1f;
		} else if(opcode >> 3 == 0b1001){
			//Compare and Branch on Nonzero
			this->name = InstrType::CBNZ;
			this->Rn = static_cast<Register>(bytecode & 0b111);
			this->hasImm = true;
			this->imm = (bytecode >> 3) & 0x1f;
		} else if(opcode >> 1 == 0b101000){
			//Byte Reverse Word
			this->name = InstrType::REV;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode >> 1 == 0b101001){
			//Byte Reverse Packed Halfword
			this->name = InstrType::REV16;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode >> 1 == 0b101011){
			//Byte Reverse Signed Halfword
			this->name = InstrType::REVSH;
			this->Rd = static_cast<Register>(bytecode & 0b111);
			this->Rm = static_cast<Register>((bytecode >> 3) & 0b111);
		} else if(opcode >> 3 == 0b1011){
			//Compare and Branch on Nonzero
			this->name = InstrType::CBNZ;
			this->Rn = static_cast<Register>(bytecode & 0b111);
			this->hasImm = true;
			this->imm = (bytecode >> 3) & 0x1f;
		} else if(opcode >> 4 == 0b110){
			//Pop multiple registers
			this->name = InstrType::POP;
			this->hasImm = true;
			this->imm = bytecode & 0xff;
		} else if(opcode >> 3 == 0b1110){
			//Breakpoint
			this->name = InstrType::BKPT;
			this->hasImm = true;
			this->imm = bytecode & 0xff;
		} else if(opcode >> 3 == 0b1111){
			unsigned opA = bytecode & 0xf0;
			opA = opA >> 4;
			unsigned opB = bytecode & 0x0f;
			// Not sure if I need hints
			if(!opA){
				if(!opB){
					this->name = InstrType::nop;
				}
			} else {
				this->name = InstrType::IT;
                this->cond = static_cast<Condition>((bytecode >> 4) & 0xf);
                this->imm = bytecode & 0xf;
			}
		} else {
			//error
			this->name = InstrType::nop;
		}
	} else if(opcode >> 1 == 0b11000){
		//Store multiple registers, STM(STMIA, STMEA)
		this->name = InstrType::STM;
		this->hasImm = true;
		this->imm = bytecode & 0xff;
		this->Rn = static_cast<Register>((bytecode >> 8) & 0b111);
	} else if(opcode >> 1 == 0b11001){
		//Load multiple registers
		this->name = InstrType::LDM;
		this->hasImm = true;
		this->imm = bytecode & 0xff;
		this->Rn = static_cast<Register>((bytecode >> 8) & 0b111);
	} else if(opcode >> 2 == 0b1101){
		//Conditional branch, supervisor call
		opcode = 0x0ff;
		opcode = opcode >> 8;
		if(opcode == 0b1110){
			//perm undef
			this->name = InstrType::UDF;
		} else if(opcode == 0b1111){
			//Supervisor Call
			this->name = InstrType::SVC;
			this->hasImm = true;
			this->imm = bytecode & 0xff;
		} else {
			//Conditional Branch    //THIS IS PROBABLY SIGN EXTENDED TOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
			this->name = InstrType::B;
			this->cond = static_cast<Condition>((bytecode >> 8) & 0xf);
			this->hasImm = true;
			this->imm = ((bytecode & 0xff) << 1);
            if(this->imm >> 8){
                int ans = this->imm | 0xfffffe00;
                this->imm = this->addr + ans + 4;
            } else {
                this->imm = this->addr + this->imm + 4; 
            }
		}
	} else if(opcode >> 1 == 0b11100){
		//Unconditional branch
		this->name = InstrType::B;
		this->hasImm = true;
		this->imm = ((bytecode & 0x7ff) << 1); //agghghghgfh this needs to sign extend!!!!!!!! i.addr +4 -4 = i.addr!!!!!
		if(this->imm >> 11 == 0b1){
			int ans = this->imm | 0xfffff800;
			this->imm = this->addr + ans + 4;
		} else {
			this->imm = this->addr + this->imm + 4;
		}
	} else {
		cout << "error not correct opcode: " << bitset<16>(bytecode) << ", " << hex << bytecode << "at addr: " << hex << this->addr << endl;
		this->name = InstrType::nop;
	}
}


unsigned int Instruction::encodeInstr(Instruction Instr){
	return 0;
}
