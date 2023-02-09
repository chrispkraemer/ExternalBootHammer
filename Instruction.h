#include <string>
#include <vector>


using namespace std;

enum Register {
	r0,
	r1,
	r2,
	r3,
	r4,
	r5,
	r6,
	r7,
	r8,
	r9,
	sl, //stack limit
    fp, // frame pointer
    ip, //intra-procedure-scratch register
    sp, //stack pointer
	lr, // link register
	pc,
	psr,
	primask,
	c,
	na

};
	

enum InstrType {
	nop,
	MOV_VIA_LSL,
	LSL,
	LSR,
	ASR,
	ADD,
	SUB,
	ADD_3_I,
	SUB_3_I,
	MOV,
	CMP,
	ADD_8_I,
	SUB_8_I,
	AND_reg,
	EOR_reg,
	LSL_reg,
	LSR_reg,
	ASR_reg,
	ADC_reg,
	SBC_reg,
	ROR_reg,
	TST_reg,
	RSB_I,
	CMP_reg,
	CMN_reg,
	ORR_reg,
	MUL_reg,
	BIC_reg,
	MVN_reg,
	BX,
	BLX,
	LDR,
	STR,
	STRH,
	STRB,
	LDRSB,
	LDRH,
	LDRB,
	LDRSH,
	ADR,
	STM,
	LDM,
	CBNZ,
	CBZ,
	SXTH,
	SXTB,
	UXTH,
	UXTB,
	PUSH,
	SETEND,
	CPS,
	REV,
	REV16,
	REVSH,
	POP,
	BKPT,
	IT,
	YIELD,
	WFE,
	WFI,
	SEV,
	UDF,
	SVC,
	B,
	BL,
    DMB,
    DSB,
    ISB,
    MRS,
    MSR 

};

enum Condition {
	eq,
	ne,
    cs,
	cc,
    mi,
    pl,
    hs,
    vc,
    hi,
    ls,
    ge,
    lt,
    gt,
    le,
    vs,
    lo,
	none
};

enum Option {
    sy = 0b1111,
    un = 0b0111,
    st = 0b1110,
    unst = 0b0110,
    no_option
};


class Instruction {
	
//make this a union later, maybe separate into 32 bit and 16 bit
	public:
		void decodeInstr(string textOfInstruction);
		void decodeInstr16(unsigned long bytecode);
		void decodeInstr32(unsigned long bytecode);
		unsigned int encodeInstr(Instruction Instr);

		InstrType name = InstrType::nop;
		unsigned addr = 0;
		bool hasImm = false;
		bool is32 = false;
		unsigned int bytecode = 0;
		Register Rd = Register::na;
		Register Rn = Register::na;
		Register Rm = Register::na;
		Condition cond = Condition::none;
        Option opt = Option::no_option;
		unsigned imm = 0; //can trim this down later, slighly inefficient to have imm and jumpaddr, they can be saved in the same space
		unsigned int jumpaddr = 0;
		Instruction();

};

