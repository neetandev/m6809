#include "moo_writer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <zlib.h>

void chunk_buffer::put_u8(uint8_t value)
{
	data.push_back(value);
}

void chunk_buffer::put_u16(uint16_t value)
{
	data.push_back(value & 0xFF);
	data.push_back(value >> 8);
}

void chunk_buffer::put_u32(uint32_t value)
{
	data.push_back(value & 0xFF);
	data.push_back((value >> 8) & 0xFF);
	data.push_back((value >> 16) & 0xFF);
	data.push_back(value >> 24);
}

void chunk_buffer::put_tag(const char tag[4])
{
	data.insert(data.end(), tag, tag + 4);
}

void chunk_buffer::put_bytes(const uint8_t *bytes, size_t length)
{
	data.insert(data.end(), bytes, bytes + length);
}

void chunk_buffer::put_chunk(const char tag[4], const chunk_buffer &payload)
{
	put_tag(tag);
	put_u32(payload.data.size());
	data.insert(data.end(), payload.data.begin(), payload.data.end());
}

void write_regs_chunk(chunk_buffer &state, const register_state &registers)
{
	chunk_buffer regs;
	regs.put_u16(0x01FF);
	regs.put_u16(registers.pc);
	regs.put_u16(registers.s);
	regs.put_u16(registers.u);
	regs.put_u16(registers.x);
	regs.put_u16(registers.y);
	regs.put_u16(registers.dp);
	regs.put_u16(registers.a);
	regs.put_u16(registers.b);
	regs.put_u16(registers.cc);
	state.put_chunk("REGS", regs);
}

void write_ram_chunk(chunk_buffer &state, const std::vector<std::pair<uint16_t, uint8_t>> &entries)
{
	chunk_buffer ram;
	ram.put_u32(entries.size());
	for (const auto &entry : entries) {
		ram.put_u32(entry.first);
		ram.put_u8(entry.second);
	}
	state.put_chunk("RAM ", ram);
}

void write_cycles_chunk(chunk_buffer &test, const std::vector<moo_cycle> &cycles)
{
	chunk_buffer chunk;
	chunk.put_u32(cycles.size());
	for (const moo_cycle &cycle : cycles) {
		chunk.put_u8(cycle.pin_bitfield);
		chunk.put_u16(cycle.address);
		chunk.put_u8(cycle.data);
		chunk.put_bytes(reinterpret_cast<const uint8_t *>(cycle.status), 4);
	}
	test.put_chunk("CYCL", chunk);
}

void write_moo_file(const char *path, uint32_t test_count, const char cpu_name[4],
	const std::vector<chunk_buffer> &tests)
{
	chunk_buffer file;
	file.put_tag("MOO ");
	file.put_u32(12);
	file.put_u8(1);
	file.put_u8(1);
	file.put_u8(0);
	file.put_u8(0);
	file.put_u32(test_count);
	file.put_tag(cpu_name);
	for (const chunk_buffer &test : tests)
		file.put_chunk("TEST", test);

	gzFile output = gzopen(path, "wb9");
	if (output == nullptr) {
		fprintf(stderr, "failed to open %s for writing\n", path);
		exit(1);
	}
	if (gzwrite(output, file.data.data(), file.data.size()) != (int)file.data.size()) {
		fprintf(stderr, "failed to write %s\n", path);
		exit(1);
	}
	gzclose(output);
}
