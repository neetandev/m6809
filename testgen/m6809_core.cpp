// Hosts the unmodified MAME m6809 instruction semantics: the inline helpers from
// m6809inl.h and the generated m6809.hxx state machine. The three register transfer
// helpers below are copied verbatim from the MAME m6809.cpp.
#include "m6809_shim.h"

#include "mame/m6809inl.h"

uint16_t m6809_base_device::read_tfr_exg_816_register(uint8_t reg)
{
	uint16_t result;

	switch(reg & 0x0F)
	{
		case  0: result = m_q.r.d;   break;  // D
		case  1: result = m_x.w;     break;  // X
		case  2: result = m_y.w;     break;  // Y
		case  3: result = m_u.w;     break;  // U
		case  4: result = m_s.w;     break;  // S
		case  5: result = m_pc.w;    break;  // PC
		case  8: result = ((uint16_t)0xff00) | m_q.r.a;   break;  // A
		case  9: result = ((uint16_t)0xff00) | m_q.r.b;   break;  // B
		case 10: result = ((uint16_t)m_cc) << 8 | m_cc;   break;  // CC
		case 11: result = ((uint16_t)m_dp) << 8 | m_dp;   break;  // DP
		default: result = 0xffff; break;
	}

	return result;
}

uint16_t m6809_base_device::read_exg_168_register(uint8_t reg)
{
	uint16_t result;

	switch(reg & 0x0F)
	{
		case  0: result = m_q.r.d;   break;  // D
		case  1: result = m_x.w;     break;  // X
		case  2: result = m_y.w;     break;  // Y
		case  3: result = m_u.w;     break;  // U
		case  4: result = m_s.w;     break;  // S
		case  5: result = m_pc.w;    break;  // PC
		case  8: result = ((uint16_t)0xff00) | m_q.r.a;   break;  // A
		case  9: result = ((uint16_t)0xff00) | m_q.r.b;   break;  // B
		case 10: result = ((uint16_t)0xff00) | m_cc;   break;  // CC
		case 11: result = ((uint16_t)0xff00) | m_dp;   break;  // DP
		default: result = 0xffff; break;
	}

	return result;
}

void m6809_base_device::write_exgtfr_register(uint8_t reg, uint16_t value)
{
	switch(reg & 0x0F)
	{
		case  0: m_q.r.d = value;    break;  // D
		case  1: m_x.w   = value;    break;  // X
		case  2: m_y.w   = value;    break;  // Y
		case  3: m_u.w   = value;    break;  // U
		case  4: m_s.w   = value;    break;  // S
		case  5: m_pc.w  = value;    break;  // PC
		case  8: m_q.r.a = (uint8_t)value; break;  // A
		case  9: m_q.r.b = (uint8_t)value; break;  // B
		case 10: m_cc    = (uint8_t)value; break;  // CC
		case 11: m_dp    = (uint8_t)value; break;  // DP
	}
}

void m6809_base_device::execute_one()
{
	switch(pop_state())
	{
#include "m6809.hxx"
	}
}
