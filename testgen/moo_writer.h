#ifndef MOO_WRITER_H
#define MOO_WRITER_H

#include <cstdint>
#include <vector>

struct chunk_buffer
{
	std::vector<uint8_t> data;

	void put_u8(uint8_t value);
	void put_u16(uint16_t value);
	void put_u32(uint32_t value);
	void put_tag(const char tag[4]);
	void put_bytes(const uint8_t *bytes, size_t length);
	void put_chunk(const char tag[4], const chunk_buffer &payload);
};

struct moo_cycle
{
	uint8_t pin_bitfield;
	uint16_t address;
	uint8_t data;
	char status[5];
};

struct register_state
{
	uint16_t pc;
	uint16_t s;
	uint16_t u;
	uint16_t x;
	uint16_t y;
	uint8_t dp;
	uint8_t a;
	uint8_t b;
	uint8_t cc;
};

void write_regs_chunk(chunk_buffer &state, const register_state &registers);
void write_ram_chunk(chunk_buffer &state, const std::vector<std::pair<uint16_t, uint8_t>> &entries);
void write_cycles_chunk(chunk_buffer &test, const std::vector<moo_cycle> &cycles);
void write_moo_file(const char *path, uint32_t test_count, const char cpu_name[4],
	const std::vector<chunk_buffer> &tests);

#endif
