#ifndef OPCODE_TABLE_H
#define OPCODE_TABLE_H

#include <cstdint>

enum class addressing_mode : uint8_t
{
	INHERENT,
	IMMEDIATE8,
	IMMEDIATE16,
	XST16_IMMEDIATE,
	DIRECT,
	INDEXED,
	EXTENDED,
	RELATIVE8,
	RELATIVE16
};

enum class cycle_special : uint8_t
{
	NONE,
	LONG_BRANCH,
	PUSH_PULL,
	RTI
};

// documented_cycles is the MC6809 datasheet cycle count (for INDEXED the base count
// before the postbyte penalty, for LONG_BRANCH the not-taken count, for PUSH_PULL the
// base count before the per-byte cost, for RTI the count without the E flag set).
// A value of -1 marks timing with no datasheet entry (undocumented behavior); such
// tests are exempt from the datasheet cross-check but still bounded by [2, 21].
struct opcode_info
{
	uint8_t prefix;
	uint8_t opcode;
	const char *mnemonic;
	addressing_mode mode;
	int documented_cycles;
	cycle_special special;
	bool documented;
};

struct skipped_opcode
{
	uint8_t prefix;
	uint8_t opcode;
	const char *mnemonic;
	const char *reason;
};

constexpr skipped_opcode SKIPPED_OPCODES[] = {
	{0x00, 0x13, "SYNC", "waits for an interrupt line and never completes in a single step"},
	{0x00, 0x14, "FREERUN", "CPU test mode that free runs until reset"},
	{0x00, 0x15, "FREERUN", "CPU test mode that free runs until reset"},
	{0x00, 0x3C, "CWAI", "waits for an interrupt line and never completes in a single step"},
	{0x00, 0xCD, "FREERUN", "CPU test mode that free runs until reset"},
};

constexpr opcode_info OPCODE_TABLE[] = {
	{0x00, 0x00, "NEG", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x01, "NEG", addressing_mode::DIRECT, 6, cycle_special::NONE, false},
	{0x00, 0x02, "XNC", addressing_mode::DIRECT, -1, cycle_special::NONE, false},
	{0x00, 0x03, "COM", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x04, "LSR", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x05, "LSR", addressing_mode::DIRECT, 6, cycle_special::NONE, false},
	{0x00, 0x06, "ROR", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x07, "ASR", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x08, "ASL", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x09, "ROL", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x0A, "DEC", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x0B, "XDEC", addressing_mode::DIRECT, -1, cycle_special::NONE, false},
	{0x00, 0x0C, "INC", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x0D, "TST", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x0E, "JMP", addressing_mode::DIRECT, 3, cycle_special::NONE, true},
	{0x00, 0x0F, "CLR", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x12, "NOP", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x16, "LBRA", addressing_mode::RELATIVE16, 5, cycle_special::NONE, true},
	{0x00, 0x17, "LBSR", addressing_mode::RELATIVE16, 9, cycle_special::NONE, true},
	{0x00, 0x18, "X18", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x00, 0x19, "DAA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x1A, "ORCC", addressing_mode::IMMEDIATE8, 3, cycle_special::NONE, true},
	{0x00, 0x1B, "NOP", addressing_mode::INHERENT, 2, cycle_special::NONE, false},
	{0x00, 0x1C, "ANDCC", addressing_mode::IMMEDIATE8, 3, cycle_special::NONE, true},
	{0x00, 0x1D, "SEX", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x1E, "EXG", addressing_mode::IMMEDIATE8, 8, cycle_special::NONE, true},
	{0x00, 0x1F, "TFR", addressing_mode::IMMEDIATE8, 6, cycle_special::NONE, true},
	{0x00, 0x20, "BRA", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x21, "BRN", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x22, "BHI", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x23, "BLS", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x24, "BCC", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x25, "BCS", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x26, "BNE", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x27, "BEQ", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x28, "BVC", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x29, "BVS", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x2A, "BPL", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x2B, "BMI", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x2C, "BGE", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x2D, "BLT", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x2E, "BGT", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x2F, "BLE", addressing_mode::RELATIVE8, 3, cycle_special::NONE, true},
	{0x00, 0x30, "LEAX", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0x31, "LEAY", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0x32, "LEAS", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0x33, "LEAU", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0x34, "PSHS", addressing_mode::IMMEDIATE8, 5, cycle_special::PUSH_PULL, true},
	{0x00, 0x35, "PULS", addressing_mode::IMMEDIATE8, 5, cycle_special::PUSH_PULL, true},
	{0x00, 0x36, "PSHU", addressing_mode::IMMEDIATE8, 5, cycle_special::PUSH_PULL, true},
	{0x00, 0x37, "PULU", addressing_mode::IMMEDIATE8, 5, cycle_special::PUSH_PULL, true},
	{0x00, 0x38, "XANDCC", addressing_mode::IMMEDIATE8, -1, cycle_special::NONE, false},
	{0x00, 0x39, "RTS", addressing_mode::INHERENT, 5, cycle_special::NONE, true},
	{0x00, 0x3A, "ABX", addressing_mode::INHERENT, 3, cycle_special::NONE, true},
	{0x00, 0x3B, "RTI", addressing_mode::INHERENT, 6, cycle_special::RTI, true},
	{0x00, 0x3D, "MUL", addressing_mode::INHERENT, 11, cycle_special::NONE, true},
	{0x00, 0x3E, "XRES", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x00, 0x3F, "SWI", addressing_mode::INHERENT, 19, cycle_special::NONE, true},
	{0x00, 0x40, "NEGA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x41, "NEGA", addressing_mode::INHERENT, 2, cycle_special::NONE, false},
	{0x00, 0x42, "XNCA", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x00, 0x43, "COMA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x44, "LSRA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x45, "LSRA", addressing_mode::INHERENT, 2, cycle_special::NONE, false},
	{0x00, 0x46, "RORA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x47, "ASRA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x48, "ASLA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x49, "ROLA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x4A, "DECA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x4B, "XDECA", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x00, 0x4C, "INCA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x4D, "TSTA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x4E, "XCLRA", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x00, 0x4F, "CLRA", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x50, "NEGB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x51, "NEGB", addressing_mode::INHERENT, 2, cycle_special::NONE, false},
	{0x00, 0x52, "XNCB", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x00, 0x53, "COMB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x54, "LSRB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x55, "LSRB", addressing_mode::INHERENT, 2, cycle_special::NONE, false},
	{0x00, 0x56, "RORB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x57, "ASRB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x58, "ASLB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x59, "ROLB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x5A, "DECB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x5B, "XDECB", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x00, 0x5C, "INCB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x5D, "TSTB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x5E, "XCLRB", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x00, 0x5F, "CLRB", addressing_mode::INHERENT, 2, cycle_special::NONE, true},
	{0x00, 0x60, "NEG", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x61, "NEG", addressing_mode::INDEXED, 6, cycle_special::NONE, false},
	{0x00, 0x62, "XNC", addressing_mode::INDEXED, -1, cycle_special::NONE, false},
	{0x00, 0x63, "COM", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x64, "LSR", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x65, "LSR", addressing_mode::INDEXED, 6, cycle_special::NONE, false},
	{0x00, 0x66, "ROR", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x67, "ASR", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x68, "ASL", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x69, "ROL", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x6A, "DEC", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x6B, "XDEC", addressing_mode::INDEXED, -1, cycle_special::NONE, false},
	{0x00, 0x6C, "INC", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x6D, "TST", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x6E, "JMP", addressing_mode::INDEXED, 3, cycle_special::NONE, true},
	{0x00, 0x6F, "CLR", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0x70, "NEG", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x71, "NEG", addressing_mode::EXTENDED, 7, cycle_special::NONE, false},
	{0x00, 0x72, "XNC", addressing_mode::EXTENDED, -1, cycle_special::NONE, false},
	{0x00, 0x73, "COM", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x74, "LSR", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x75, "LSR", addressing_mode::EXTENDED, 7, cycle_special::NONE, false},
	{0x00, 0x76, "ROR", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x77, "ASR", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x78, "ASL", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x79, "ROL", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x7A, "DEC", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x7B, "XDEC", addressing_mode::EXTENDED, -1, cycle_special::NONE, false},
	{0x00, 0x7C, "INC", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x7D, "TST", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x7E, "JMP", addressing_mode::EXTENDED, 4, cycle_special::NONE, true},
	{0x00, 0x7F, "CLR", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0x80, "SUBA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x81, "CMPA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x82, "SBCA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x83, "SUBD", addressing_mode::IMMEDIATE16, 4, cycle_special::NONE, true},
	{0x00, 0x84, "ANDA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x85, "BITA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x86, "LDA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x87, "XSTA", addressing_mode::IMMEDIATE8, -1, cycle_special::NONE, false},
	{0x00, 0x88, "EORA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x89, "ADCA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x8A, "ORA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x8B, "ADDA", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0x8C, "CMPX", addressing_mode::IMMEDIATE16, 4, cycle_special::NONE, true},
	{0x00, 0x8D, "BSR", addressing_mode::RELATIVE8, 7, cycle_special::NONE, true},
	{0x00, 0x8E, "LDX", addressing_mode::IMMEDIATE16, 3, cycle_special::NONE, true},
	{0x00, 0x8F, "XSTX", addressing_mode::XST16_IMMEDIATE, -1, cycle_special::NONE, false},
	{0x00, 0x90, "SUBA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x91, "CMPA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x92, "SBCA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x93, "SUBD", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x94, "ANDA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x95, "BITA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x96, "LDA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x97, "STA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x98, "EORA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x99, "ADCA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x9A, "ORA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x9B, "ADDA", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0x9C, "CMPX", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0x9D, "JSR", addressing_mode::DIRECT, 7, cycle_special::NONE, true},
	{0x00, 0x9E, "LDX", addressing_mode::DIRECT, 5, cycle_special::NONE, true},
	{0x00, 0x9F, "STX", addressing_mode::DIRECT, 5, cycle_special::NONE, true},
	{0x00, 0xA0, "SUBA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xA1, "CMPA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xA2, "SBCA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xA3, "SUBD", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0xA4, "ANDA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xA5, "BITA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xA6, "LDA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xA7, "STA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xA8, "EORA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xA9, "ADCA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xAA, "ORA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xAB, "ADDA", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xAC, "CMPX", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0xAD, "JSR", addressing_mode::INDEXED, 7, cycle_special::NONE, true},
	{0x00, 0xAE, "LDX", addressing_mode::INDEXED, 5, cycle_special::NONE, true},
	{0x00, 0xAF, "STX", addressing_mode::INDEXED, 5, cycle_special::NONE, true},
	{0x00, 0xB0, "SUBA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xB1, "CMPA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xB2, "SBCA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xB3, "SUBD", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0xB4, "ANDA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xB5, "BITA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xB6, "LDA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xB7, "STA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xB8, "EORA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xB9, "ADCA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xBA, "ORA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xBB, "ADDA", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xBC, "CMPX", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0xBD, "JSR", addressing_mode::EXTENDED, 8, cycle_special::NONE, true},
	{0x00, 0xBE, "LDX", addressing_mode::EXTENDED, 6, cycle_special::NONE, true},
	{0x00, 0xBF, "STX", addressing_mode::EXTENDED, 6, cycle_special::NONE, true},
	{0x00, 0xC0, "SUBB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xC1, "CMPB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xC2, "SBCB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xC3, "ADDD", addressing_mode::IMMEDIATE16, 4, cycle_special::NONE, true},
	{0x00, 0xC4, "ANDB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xC5, "BITB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xC6, "LDB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xC7, "XSTB", addressing_mode::IMMEDIATE8, -1, cycle_special::NONE, false},
	{0x00, 0xC8, "EORB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xC9, "ADCB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xCA, "ORB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xCB, "ADDB", addressing_mode::IMMEDIATE8, 2, cycle_special::NONE, true},
	{0x00, 0xCC, "LDD", addressing_mode::IMMEDIATE16, 3, cycle_special::NONE, true},
	{0x00, 0xCE, "LDU", addressing_mode::IMMEDIATE16, 3, cycle_special::NONE, true},
	{0x00, 0xCF, "XSTU", addressing_mode::XST16_IMMEDIATE, -1, cycle_special::NONE, false},
	{0x00, 0xD0, "SUBB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xD1, "CMPB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xD2, "SBCB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xD3, "ADDD", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x00, 0xD4, "ANDB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xD5, "BITB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xD6, "LDB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xD7, "STB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xD8, "EORB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xD9, "ADCB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xDA, "ORB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xDB, "ADDB", addressing_mode::DIRECT, 4, cycle_special::NONE, true},
	{0x00, 0xDC, "LDD", addressing_mode::DIRECT, 5, cycle_special::NONE, true},
	{0x00, 0xDD, "STD", addressing_mode::DIRECT, 5, cycle_special::NONE, true},
	{0x00, 0xDE, "LDU", addressing_mode::DIRECT, 5, cycle_special::NONE, true},
	{0x00, 0xDF, "STU", addressing_mode::DIRECT, 5, cycle_special::NONE, true},
	{0x00, 0xE0, "SUBB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xE1, "CMPB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xE2, "SBCB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xE3, "ADDD", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x00, 0xE4, "ANDB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xE5, "BITB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xE6, "LDB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xE7, "STB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xE8, "EORB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xE9, "ADCB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xEA, "ORB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xEB, "ADDB", addressing_mode::INDEXED, 4, cycle_special::NONE, true},
	{0x00, 0xEC, "LDD", addressing_mode::INDEXED, 5, cycle_special::NONE, true},
	{0x00, 0xED, "STD", addressing_mode::INDEXED, 5, cycle_special::NONE, true},
	{0x00, 0xEE, "LDU", addressing_mode::INDEXED, 5, cycle_special::NONE, true},
	{0x00, 0xEF, "STU", addressing_mode::INDEXED, 5, cycle_special::NONE, true},
	{0x00, 0xF0, "SUBB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xF1, "CMPB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xF2, "SBCB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xF3, "ADDD", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x00, 0xF4, "ANDB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xF5, "BITB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xF6, "LDB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xF7, "STB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xF8, "EORB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xF9, "ADCB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xFA, "ORB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xFB, "ADDB", addressing_mode::EXTENDED, 5, cycle_special::NONE, true},
	{0x00, 0xFC, "LDD", addressing_mode::EXTENDED, 6, cycle_special::NONE, true},
	{0x00, 0xFD, "STD", addressing_mode::EXTENDED, 6, cycle_special::NONE, true},
	{0x00, 0xFE, "LDU", addressing_mode::EXTENDED, 6, cycle_special::NONE, true},
	{0x00, 0xFF, "STU", addressing_mode::EXTENDED, 6, cycle_special::NONE, true},
	{0x10, 0x20, "LBRA", addressing_mode::RELATIVE16, 6, cycle_special::NONE, false},
	{0x10, 0x21, "LBRN", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x22, "LBHI", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x23, "LBLS", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x24, "LBCC", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x25, "LBCS", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x26, "LBNE", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x27, "LBEQ", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x28, "LBVC", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x29, "LBVS", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x2A, "LBPL", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x2B, "LBMI", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x2C, "LBGE", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x2D, "LBLT", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x2E, "LBGT", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x2F, "LBLE", addressing_mode::RELATIVE16, 5, cycle_special::LONG_BRANCH, true},
	{0x10, 0x3E, "XSWI2", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x10, 0x3F, "SWI2", addressing_mode::INHERENT, 20, cycle_special::NONE, true},
	{0x10, 0x83, "CMPD", addressing_mode::IMMEDIATE16, 5, cycle_special::NONE, true},
	{0x10, 0x87, "XSTA", addressing_mode::IMMEDIATE8, -1, cycle_special::NONE, false},
	{0x10, 0x8C, "CMPY", addressing_mode::IMMEDIATE16, 5, cycle_special::NONE, true},
	{0x10, 0x8E, "LDY", addressing_mode::IMMEDIATE16, 4, cycle_special::NONE, true},
	{0x10, 0x8F, "XSTY", addressing_mode::XST16_IMMEDIATE, -1, cycle_special::NONE, false},
	{0x10, 0x93, "CMPD", addressing_mode::DIRECT, 7, cycle_special::NONE, true},
	{0x10, 0x9C, "CMPY", addressing_mode::DIRECT, 7, cycle_special::NONE, true},
	{0x10, 0x9E, "LDY", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x10, 0x9F, "STY", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x10, 0xA3, "CMPD", addressing_mode::INDEXED, 7, cycle_special::NONE, true},
	{0x10, 0xAC, "CMPY", addressing_mode::INDEXED, 7, cycle_special::NONE, true},
	{0x10, 0xAE, "LDY", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x10, 0xAF, "STY", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x10, 0xB3, "CMPD", addressing_mode::EXTENDED, 8, cycle_special::NONE, true},
	{0x10, 0xBC, "CMPY", addressing_mode::EXTENDED, 8, cycle_special::NONE, true},
	{0x10, 0xBE, "LDY", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x10, 0xBF, "STY", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x10, 0xC3, "XADDD", addressing_mode::IMMEDIATE16, -1, cycle_special::NONE, false},
	{0x10, 0xC7, "XSTB", addressing_mode::IMMEDIATE8, -1, cycle_special::NONE, false},
	{0x10, 0xCE, "LDS", addressing_mode::IMMEDIATE16, 4, cycle_special::NONE, true},
	{0x10, 0xCF, "XSTS", addressing_mode::XST16_IMMEDIATE, -1, cycle_special::NONE, false},
	{0x10, 0xD3, "XADDD", addressing_mode::DIRECT, -1, cycle_special::NONE, false},
	{0x10, 0xDE, "LDS", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x10, 0xDF, "STS", addressing_mode::DIRECT, 6, cycle_special::NONE, true},
	{0x10, 0xE3, "XADDD", addressing_mode::INDEXED, -1, cycle_special::NONE, false},
	{0x10, 0xEE, "LDS", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x10, 0xEF, "STS", addressing_mode::INDEXED, 6, cycle_special::NONE, true},
	{0x10, 0xF3, "XADDD", addressing_mode::EXTENDED, -1, cycle_special::NONE, false},
	{0x10, 0xFE, "LDS", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x10, 0xFF, "STS", addressing_mode::EXTENDED, 7, cycle_special::NONE, true},
	{0x11, 0x3E, "XFIRQ", addressing_mode::INHERENT, -1, cycle_special::NONE, false},
	{0x11, 0x3F, "SWI3", addressing_mode::INHERENT, 20, cycle_special::NONE, true},
	{0x11, 0x83, "CMPU", addressing_mode::IMMEDIATE16, 5, cycle_special::NONE, true},
	{0x11, 0x87, "XSTA", addressing_mode::IMMEDIATE8, -1, cycle_special::NONE, false},
	{0x11, 0x8C, "CMPS", addressing_mode::IMMEDIATE16, 5, cycle_special::NONE, true},
	{0x11, 0x8F, "XSTX", addressing_mode::XST16_IMMEDIATE, -1, cycle_special::NONE, false},
	{0x11, 0x93, "CMPU", addressing_mode::DIRECT, 7, cycle_special::NONE, true},
	{0x11, 0x9C, "CMPS", addressing_mode::DIRECT, 7, cycle_special::NONE, true},
	{0x11, 0xA3, "CMPU", addressing_mode::INDEXED, 7, cycle_special::NONE, true},
	{0x11, 0xAC, "CMPS", addressing_mode::INDEXED, 7, cycle_special::NONE, true},
	{0x11, 0xB3, "CMPU", addressing_mode::EXTENDED, 8, cycle_special::NONE, true},
	{0x11, 0xBC, "CMPS", addressing_mode::EXTENDED, 8, cycle_special::NONE, true},
	{0x11, 0xC3, "XADDU", addressing_mode::IMMEDIATE16, -1, cycle_special::NONE, false},
	{0x11, 0xC7, "XSTB", addressing_mode::IMMEDIATE8, -1, cycle_special::NONE, false},
	{0x11, 0xCF, "XSTU", addressing_mode::XST16_IMMEDIATE, -1, cycle_special::NONE, false},
	{0x11, 0xD3, "XADDU", addressing_mode::DIRECT, -1, cycle_special::NONE, false},
	{0x11, 0xE3, "XADDU", addressing_mode::INDEXED, -1, cycle_special::NONE, false},
	{0x11, 0xF3, "XADDU", addressing_mode::EXTENDED, -1, cycle_special::NONE, false},
};

// Number of operand bytes an indexed postbyte adds after the postbyte itself.
inline int indexed_postbyte_operand_bytes(uint8_t postbyte)
{
	if (!(postbyte & 0x80))
		return 0;
	switch (postbyte & 0x0F) {
	case 0x08:
	case 0x0C:
		return 1;
	case 0x09:
	case 0x0D:
	case 0x0F:
		return 2;
	default:
		return 0;
	}
}

// Datasheet indexed addressing penalty. Returns -1 for postbyte modes without a
// documented timing (they still execute in MAME and are captured in the vectors).
inline int indexed_postbyte_extra_cycles(uint8_t postbyte)
{
	if (!(postbyte & 0x80))
		return 1;

	bool indirect = (postbyte & 0x10) != 0;
	switch (postbyte & 0x0F) {
	case 0x00: return indirect ? -1 : 2;
	case 0x01: return indirect ? 6 : 3;
	case 0x02: return indirect ? -1 : 2;
	case 0x03: return indirect ? 6 : 3;
	case 0x04: return indirect ? 3 : 0;
	case 0x05: return indirect ? 4 : 1;
	case 0x06: return indirect ? 4 : 1;
	case 0x08: return indirect ? 4 : 1;
	case 0x09: return indirect ? 7 : 4;
	case 0x0B: return indirect ? 7 : 4;
	case 0x0C: return indirect ? 4 : 1;
	case 0x0D: return indirect ? 8 : 5;
	case 0x0F: return indirect ? 5 : -1;
	default: return -1;
	}
}

inline int operand_length(addressing_mode mode, uint8_t indexed_postbyte)
{
	switch (mode) {
	case addressing_mode::INHERENT:
		return 0;
	case addressing_mode::IMMEDIATE8:
	case addressing_mode::DIRECT:
	case addressing_mode::RELATIVE8:
		return 1;
	case addressing_mode::IMMEDIATE16:
	case addressing_mode::XST16_IMMEDIATE:
	case addressing_mode::EXTENDED:
	case addressing_mode::RELATIVE16:
		return 2;
	case addressing_mode::INDEXED:
		return 1 + indexed_postbyte_operand_bytes(indexed_postbyte);
	}
	return 0;
}

// Branch condition evaluation from an initial CC value, by opcode low nibble.
inline bool branch_condition(uint8_t opcode, uint8_t cc)
{
	bool carry = (cc & 0x01) != 0;
	bool overflow = (cc & 0x02) != 0;
	bool zero = (cc & 0x04) != 0;
	bool negative = (cc & 0x08) != 0;

	switch (opcode & 0x0F) {
	case 0x00: return true;
	case 0x01: return false;
	case 0x02: return !(carry || zero);
	case 0x03: return carry || zero;
	case 0x04: return !carry;
	case 0x05: return carry;
	case 0x06: return !zero;
	case 0x07: return zero;
	case 0x08: return !overflow;
	case 0x09: return overflow;
	case 0x0A: return !negative;
	case 0x0B: return negative;
	case 0x0C: return negative == overflow;
	case 0x0D: return negative != overflow;
	case 0x0E: return negative == overflow && !zero;
	case 0x0F: return !(negative == overflow && !zero);
	}
	return false;
}

// Bytes transferred by a PSH/PUL register mask.
inline int push_pull_bytes(uint8_t mask)
{
	int bytes = 0;
	if (mask & 0x01) bytes += 1;
	if (mask & 0x02) bytes += 1;
	if (mask & 0x04) bytes += 1;
	if (mask & 0x08) bytes += 1;
	if (mask & 0x10) bytes += 2;
	if (mask & 0x20) bytes += 2;
	if (mask & 0x40) bytes += 2;
	if (mask & 0x80) bytes += 2;
	return bytes;
}

#endif
