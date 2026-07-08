// Standalone re-host of MAME's m6809_base_device. The include guard below matches the
// reference m6809.h so that the unmodified reference m6809inl.h can be included on top of
// this class definition. Instruction semantics come verbatim from mame via m6809inl.h and
// the generated m6809.hxx; this header only mirrors the class surface. Member names and
// types must match mame/src/devices/cpu/m6809/m6809.h exactly.
#ifndef MAME_CPU_M6809_M6809_H
#define MAME_CPU_M6809_M6809_H

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>

#define LSB_FIRST 1

#define ATTR_FORCE_INLINE __attribute__((always_inline))
#define UNEXPECTED(x) __builtin_expect(!!(x), 0)

template <typename T, typename U>
constexpr T BIT(T x, U n)
{
	return (x >> n) & T(1);
}

[[noreturn]] inline void fatalerror(const char *format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	vfprintf(stderr, format, arguments);
	va_end(arguments);
	fputc('\n', stderr);
	abort();
}

constexpr int CLEAR_LINE = 0;
constexpr int ASSERT_LINE = 1;
constexpr int INPUT_LINE_NMI = 32;

#define M6809_IRQ_LINE  0   /* IRQ line number */
#define M6809_FIRQ_LINE 1   /* FIRQ line number */

union PAIR16
{
	struct { uint8_t l, h; } b;
	struct { int8_t l, h; } sb;
	uint16_t w;
	int16_t sw;
};

class m6809_base_device
{
public:
	class memory_interface {
	public:
		virtual ~memory_interface() {}
		virtual uint8_t read(uint16_t adr) = 0;
		virtual uint8_t read_opcode(uint16_t adr) = 0;
		virtual uint8_t read_opcode_arg(uint16_t adr) = 0;
		virtual void write(uint16_t adr, uint8_t val) = 0;
	};

	// addressing modes
	enum
	{
		ADDRESSING_MODE_IMMEDIATE   = 0,
		ADDRESSING_MODE_EA          = 1,
		ADDRESSING_MODE_REGISTER_A  = 2,
		ADDRESSING_MODE_REGISTER_B  = 3,
		ADDRESSING_MODE_REGISTER_D  = 4
	};

	// flag bits in the cc register
	enum
	{
		CC_C        = 0x01,         // Carry
		CC_V        = 0x02,         // Overflow
		CC_Z        = 0x04,         // Zero
		CC_N        = 0x08,         // Negative
		CC_I        = 0x10,         // Inhibit IRQ
		CC_H        = 0x20,         // Half (auxiliary) carry
		CC_F        = 0x40,         // Inhibit FIRQ
		CC_E        = 0x80          // Entire state pushed
	};

	// flag combinations
	enum
	{
		CC_VC   = CC_V | CC_C,
		CC_ZC   = CC_Z | CC_C,
		CC_NZ   = CC_N | CC_Z,
		CC_NZC  = CC_N | CC_Z | CC_C,
		CC_NZV  = CC_N | CC_Z | CC_V,
		CC_NZVC = CC_N | CC_Z | CC_V | CC_C,
		CC_HNZVC = CC_H | CC_N | CC_Z | CC_V | CC_C
	};

	// interrupt vectors
	enum
	{
		VECTOR_SWI3         = 0xFFF2,
		VECTOR_SWI2         = 0xFFF4,
		VECTOR_FIRQ         = 0xFFF6,
		VECTOR_IRQ          = 0xFFF8,
		VECTOR_SWI          = 0xFFFA,
		VECTOR_NMI          = 0xFFFC,
		VECTOR_RESET_FFFE   = 0xFFFE
	};

	// exception numbers for debugger
	enum
	{
		EXCEPTION_SWI   = 1,
		EXCEPTION_SWI2  = 2,
		EXCEPTION_SWI3  = 3,
		EXCEPTION_XFIRQ = 4,
		EXCEPTION_XRES  = 5
	};

	union M6809Q
	{
		union
		{
			struct { uint8_t f, e, b, a; };
			struct { uint16_t w, d; };
		} r;
		struct { PAIR16 w, d; } p;
		uint32_t q;
	};

	// no-op stand-ins for the MAME devcb callbacks
	struct null_write_line
	{
		void operator()(int) {}
	};

	struct null_read8
	{
		bool isunset() const { return true; }
		uint8_t operator()(uint16_t) const { return 0; }
	};

	// Memory interface
	std::unique_ptr<memory_interface> m_mintf;

	// CPU registers
	PAIR16                      m_pc;               // program counter
	PAIR16                      m_ppc;              // previous program counter
	M6809Q                      m_q;                // accumulator a and b (plus e and f on 6309)
	PAIR16                      m_x, m_y;           // index registers
	PAIR16                      m_u, m_s;           // stack pointers
	uint8_t                     m_dp;               // direct page register
	uint8_t                     m_cc;
	PAIR16                      m_temp;
	uint8_t                     m_opcode;

	// other internal state
	uint8_t *                   m_reg8;
	PAIR16 *                    m_reg16;
	int                         m_reg;
	bool                        m_nmi_line;
	bool                        m_nmi_asserted;
	bool                        m_firq_line;
	bool                        m_irq_line;
	bool                        m_lds_encountered;
	int                         m_icount;
	int                         m_addressing_mode;
	PAIR16                      m_ea;                 // effective address

	// Callbacks
	null_write_line             m_lic_func;
	null_read8                  m_vector_read_func;
	null_write_line             m_syncack_write_func;

	// other state
	uint32_t                    m_state;
	bool                        m_cond;
	bool                        m_free_run;

	// eat cycles
	inline void eat(int cycles)                              { m_icount -= cycles; }
	void eat_remaining();

	// read a byte from given memory location
	inline uint8_t read_memory(uint16_t address)             { eat(1); return m_mintf->read(address); }

	// write a byte to given memory location
	inline void write_memory(uint16_t address, uint8_t data) { eat(1); m_mintf->write(address, data); }

	// read_opcode() is like read_memory() except it is used for reading opcodes
	inline uint8_t read_opcode(uint16_t address)             { eat(1); return m_mintf->read_opcode(address); }

	// read_opcode_arg() is identical to read_opcode() except it is used for reading opcode arguments
	inline uint8_t read_opcode_arg(uint16_t address)         { eat(1); return m_mintf->read_opcode_arg(address); }

	// read_opcode() and bump the program counter
	inline uint8_t read_opcode()                             { return read_opcode(m_pc.w++); }
	inline uint8_t read_opcode_arg()                         { return read_opcode_arg(m_pc.w++); }
	inline void dummy_read_opcode_arg(uint16_t delta)        { read_opcode_arg(m_pc.w + delta); }
	inline void dummy_vma(int count)                         { for(int i=0; i != count; i++) { read_opcode_arg(0xffff); } }

	// state stack - implemented as a uint32_t
	void push_state(uint16_t state)                 { m_state = (m_state << 9) | state; }
	uint16_t pop_state()                            { uint16_t result = m_state & 0x1ff; m_state >>= 9; return result; }
	void reset_state()                              { m_state = 1; }

	// effective address reading/writing
	uint8_t read_ea()                               { return read_memory(m_ea.w); }
	void write_ea(uint8_t data)                     { write_memory(m_ea.w, data); }
	void set_ea(uint16_t ea)                        { m_ea.w = ea; m_addressing_mode = ADDRESSING_MODE_EA; }
	void set_ea_h(uint8_t ea_h)                     { m_ea.b.h = ea_h; }
	void set_ea_l(uint8_t ea_l)                     { m_ea.b.l = ea_l; m_addressing_mode = ADDRESSING_MODE_EA; }

	// operand reading/writing
	uint8_t read_operand();
	uint8_t read_operand(int ordinal);
	uint8_t read_vector(int ordinal);
	void write_operand(uint8_t data);
	void write_operand(int ordinal, uint8_t data);

	// instructions
	void daa();
	void mul();

	// miscellaneous
	void nop()                                      { }
	template<class T> T rotate_right(T value);
	template<class T> uint32_t rotate_left(T value);
	void set_a()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_A; }
	void set_b()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_B; }
	void set_d()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_D; }
	void set_imm()                                  { m_addressing_mode = ADDRESSING_MODE_IMMEDIATE; }
	void set_regop8(uint8_t &reg)                   { m_reg8 = &reg; m_reg16 = nullptr; }
	void set_regop16(PAIR16 &reg)                   { m_reg16 = &reg; m_reg8 = nullptr; }
	uint8_t &regop8()                               { assert(m_reg8 != nullptr); return *m_reg8; }
	PAIR16 &regop16()                               { assert(m_reg16 != nullptr); return *m_reg16; }
	bool is_register_register_op_16_bit()           { return m_reg16 != nullptr; }
	bool add8_sets_h()                              { return true; }
	bool hd6309_native_mode()                       { return false; }

	// index reg
	uint16_t &ireg();

	// flags
	template<class T> T set_flags(uint8_t mask, T a, T b, uint32_t r);
	template<class T> T set_flags(uint8_t mask, T r);

	// branch conditions
	inline bool cond_hi() { return !(m_cc & CC_ZC); }                                                // BHI/BLS
	inline bool cond_cc() { return !(m_cc & CC_C); }                                                 // BCC/BCS
	inline bool cond_ne() { return !(m_cc & CC_Z); }                                                 // BNE/BEQ
	inline bool cond_vc() { return !(m_cc & CC_V); }                                                 // BVC/BVS
	inline bool cond_pl() { return !(m_cc & CC_N); }                                                 // BPL/BMI
	inline bool cond_ge() { return (m_cc & CC_N ? true : false) == (m_cc & CC_V ? true : false); }   // BGE/BLT
	inline bool cond_gt() { return cond_ge() && !(m_cc & CC_Z); }                                    // BGT/BLE
	inline void set_cond(bool cond)  { m_cond = cond; }
	inline bool branch_taken()       { return m_cond; }

	// interrupt registers
	bool firq_saves_entire_state()      { return false; }
	uint16_t partial_state_registers()  { return 0x81; }
	uint16_t entire_state_registers()   { return 0xFF; }

	// miscellaneous
	uint16_t read_tfr_exg_816_register(uint8_t reg);
	uint16_t read_exg_168_register(uint8_t reg);
	void write_exgtfr_register(uint8_t reg, uint16_t value);
	bool is_register_addressing_mode();
	bool is_ea_addressing_mode() { return m_addressing_mode == ADDRESSING_MODE_EA; }
	uint16_t get_pending_interrupt();
	void log_illegal() {}

	// no-op stand-ins for the MAME debugger and interrupt acknowledge hooks
	void debugger_instruction_hook(uint16_t) {}
	void debugger_exception_hook(int) {}
	void debugger_wait_hook() {}
	void standard_irq_callback(int, uint16_t) {}

	void execute_one();
};

#endif // MAME_CPU_M6809_M6809_H
