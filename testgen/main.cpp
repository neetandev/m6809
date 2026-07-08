#include "m6809_shim.h"
#include "moo_writer.h"
#include "opcode_table.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

struct bus_access
{
	bool is_write;
	uint16_t address;
	uint8_t data;
	int icount_after;
};

class logging_memory : public m6809_base_device::memory_interface
{
public:
	uint8_t memory[65536];
	std::vector<bus_access> accesses;
	const m6809_base_device *device = nullptr;

	uint8_t read(uint16_t address) override { return log_read(address); }
	uint8_t read_opcode(uint16_t address) override { return log_read(address); }
	uint8_t read_opcode_arg(uint16_t address) override { return log_read(address); }

	void write(uint16_t address, uint8_t value) override
	{
		memory[address] = value;
		accesses.push_back({true, address, value, device->m_icount});
	}

private:
	uint8_t log_read(uint16_t address)
	{
		uint8_t value = memory[address];
		accesses.push_back({false, address, value, device->m_icount});
		return value;
	}
};

struct splitmix64
{
	uint64_t state;

	explicit splitmix64(uint64_t seed) : state(seed) {}

	uint64_t next()
	{
		state += 0x9E3779B97F4A7C15ULL;
		uint64_t z = state;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}
};

static const int INITIAL_ICOUNT = 10000;

static void initialize_device(m6809_base_device &device)
{
	device.m_ppc.w = 0;
	device.m_temp.w = 0;
	device.m_opcode = 0;
	device.m_reg8 = nullptr;
	device.m_reg16 = nullptr;
	device.m_reg = 0;
	device.m_nmi_line = false;
	device.m_nmi_asserted = false;
	device.m_firq_line = false;
	device.m_irq_line = false;
	device.m_lds_encountered = false;
	device.m_icount = INITIAL_ICOUNT;
	device.m_addressing_mode = 0;
	device.m_ea.w = 0;
	device.m_state = 0;
	device.m_cond = false;
	device.m_free_run = false;
}

static std::string stem_string(const opcode_info &info, bool uppercase)
{
	char buffer[8];
	const char *format = uppercase ? "%02X" : "%02x";
	std::string stem;
	if (info.prefix != 0) {
		snprintf(buffer, sizeof(buffer), format, info.prefix);
		stem += buffer;
		stem += ' ';
	}
	snprintf(buffer, sizeof(buffer), format, info.opcode);
	stem += buffer;
	return stem;
}

[[noreturn]] static void fail_test(const opcode_info &info, uint32_t test_index, uint64_t seed,
	const char *format, ...)
{
	va_list arguments;
	fprintf(stderr, "FAILED %s test %u (seed %016llx): ", stem_string(info, true).c_str(),
		test_index, (unsigned long long)seed);
	va_start(arguments, format);
	vfprintf(stderr, format, arguments);
	va_end(arguments);
	fputc('\n', stderr);
	exit(1);
}

// Expected cycle count from the datasheet table, or -1 if this combination of opcode
// and initial state has no documented timing.
static int expected_cycles(const opcode_info &info, uint8_t postbyte, uint8_t operand_byte,
	uint8_t initial_cc, uint8_t stacked_cc)
{
	if (info.documented_cycles < 0)
		return -1;

	int cycles = info.documented_cycles;
	if (info.mode == addressing_mode::INDEXED) {
		int extra = indexed_postbyte_extra_cycles(postbyte);
		if (extra < 0)
			return -1;
		cycles += extra;
	}

	switch (info.special) {
	case cycle_special::NONE:
		break;
	case cycle_special::LONG_BRANCH:
		if (branch_condition(info.opcode, initial_cc))
			cycles += 1;
		break;
	case cycle_special::PUSH_PULL:
		cycles += push_pull_bytes(operand_byte);
		break;
	case cycle_special::RTI:
		cycles = (stacked_cc & 0x80) ? 15 : 6;
		break;
	}

	return cycles;
}

struct generation_summary
{
	int minimum_cycles = 1000;
	int maximum_cycles = 0;
	uint32_t checked = 0;
	uint32_t exempt = 0;
};

static chunk_buffer generate_test(const opcode_info &info, uint32_t test_index, uint64_t seed,
	bool verbose, generation_summary &summary)
{
	splitmix64 rng(seed);

	m6809_base_device device;
	logging_memory *memory = new logging_memory();
	device.m_mintf.reset(memory);
	memory->device = &device;

	for (size_t offset = 0; offset < sizeof(memory->memory); offset += 8) {
		uint64_t value = rng.next();
		std::memcpy(memory->memory + offset, &value, 8);
	}

	initialize_device(device);
	device.m_pc.w = rng.next();
	device.m_s.w = rng.next();
	device.m_u.w = rng.next();
	device.m_x.w = rng.next();
	device.m_y.w = rng.next();
	device.m_dp = rng.next();
	device.m_q.q = 0;
	device.m_q.r.a = rng.next();
	device.m_q.r.b = rng.next();
	device.m_cc = rng.next();

	uint16_t pc = device.m_pc.w;
	int prefix_length = info.prefix != 0 ? 1 : 0;
	if (info.prefix != 0)
		memory->memory[pc] = info.prefix;
	memory->memory[(uint16_t)(pc + prefix_length)] = info.opcode;

	uint8_t postbyte = memory->memory[(uint16_t)(pc + prefix_length + 1)];
	uint8_t operand_byte = postbyte;
	int instruction_length =
		prefix_length + 1 + operand_length(info.mode, postbyte);

	uint8_t pristine[65536];
	std::memcpy(pristine, memory->memory, sizeof(pristine));

	register_state initial_registers = {
		device.m_pc.w, device.m_s.w, device.m_u.w, device.m_x.w, device.m_y.w,
		device.m_dp, device.m_q.r.a, device.m_q.r.b, device.m_cc,
	};

	int calls = 0;
	do {
		device.execute_one();
		if (++calls > 64)
			fail_test(info, test_index, seed, "instruction did not terminate");
	} while (device.m_state != 0);
	int cycles = INITIAL_ICOUNT - device.m_icount;

	if (device.m_free_run)
		fail_test(info, test_index, seed, "free run mode was entered");
	if (cycles < 2 || cycles > 21)
		fail_test(info, test_index, seed, "cycle count %d outside [2, 21]", cycles);
	if (memory->accesses.empty() || memory->accesses[0].is_write
		|| memory->accesses[0].address != pc
		|| memory->accesses[0].data != pristine[pc])
		fail_test(info, test_index, seed, "first bus access is not the opcode fetch");
	if (info.prefix != 0
		&& (memory->accesses.size() < 2 || memory->accesses[1].is_write
			|| memory->accesses[1].address != (uint16_t)(pc + 1)
			|| memory->accesses[1].data != info.opcode))
		fail_test(info, test_index, seed, "second bus access is not the opcode fetch");

	int expected = expected_cycles(info, postbyte, operand_byte, initial_registers.cc,
		pristine[initial_registers.s]);
	if (expected >= 0) {
		if (cycles != expected)
			fail_test(info, test_index, seed, "cycle count %d does not match datasheet %d",
				cycles, expected);
		summary.checked += 1;
	} else {
		summary.exempt += 1;
	}
	summary.minimum_cycles = std::min(summary.minimum_cycles, cycles);
	summary.maximum_cycles = std::max(summary.maximum_cycles, cycles);

	std::vector<moo_cycle> cycle_list;
	int previous_icount = INITIAL_ICOUNT;
	for (const bus_access &access : memory->accesses) {
		for (int gap = previous_icount - access.icount_after; gap > 1; gap--)
			cycle_list.push_back({0x00, 0, 0, "----"});
		moo_cycle cycle = {0x03, access.address, access.data, ""};
		std::memcpy(cycle.status, access.is_write ? "-wm-" : "r-m-", 5);
		cycle_list.push_back(cycle);
		previous_icount = access.icount_after;
	}
	for (int gap = previous_icount - device.m_icount; gap > 0; gap--)
		cycle_list.push_back({0x00, 0, 0, "----"});
	if ((int)cycle_list.size() != cycles)
		fail_test(info, test_index, seed, "cycle list length %zu does not match cycle count %d",
			cycle_list.size(), cycles);

	std::vector<uint16_t> touched_addresses;
	for (int i = 0; i < instruction_length; i++)
		touched_addresses.push_back((uint16_t)(pc + i));
	for (const bus_access &access : memory->accesses)
		touched_addresses.push_back(access.address);
	std::sort(touched_addresses.begin(), touched_addresses.end());
	touched_addresses.erase(std::unique(touched_addresses.begin(), touched_addresses.end()),
		touched_addresses.end());

	std::vector<std::pair<uint16_t, uint8_t>> initial_ram;
	std::vector<std::pair<uint16_t, uint8_t>> final_ram;
	for (uint16_t address : touched_addresses) {
		initial_ram.push_back({address, pristine[address]});
		final_ram.push_back({address, memory->memory[address]});
	}

	register_state final_registers = {
		device.m_pc.w, device.m_s.w, device.m_u.w, device.m_x.w, device.m_y.w,
		device.m_dp, device.m_q.r.a, device.m_q.r.b, device.m_cc,
	};

	if (verbose) {
		printf("%s %04u: cycles=%d length=%d\n", stem_string(info, true).c_str(), test_index,
			cycles, instruction_length);
		for (const moo_cycle &cycle : cycle_list)
			printf("    %s %04X %02X\n", cycle.status, cycle.address, cycle.data);
	}

	chunk_buffer test;
	test.put_u32(test_index);

	char name[16];
	snprintf(name, sizeof(name), "%s %04u", stem_string(info, true).c_str(), test_index);
	chunk_buffer name_chunk;
	name_chunk.put_u32(strlen(name));
	name_chunk.put_bytes(reinterpret_cast<const uint8_t *>(name), strlen(name));
	test.put_chunk("NAME", name_chunk);

	chunk_buffer bytes_chunk;
	bytes_chunk.put_u32(instruction_length);
	for (int i = 0; i < instruction_length; i++)
		bytes_chunk.put_u8(pristine[(uint16_t)(pc + i)]);
	test.put_chunk("BYTS", bytes_chunk);

	chunk_buffer initial_state;
	write_regs_chunk(initial_state, initial_registers);
	write_ram_chunk(initial_state, initial_ram);
	test.put_chunk("INIT", initial_state);

	chunk_buffer final_state;
	write_regs_chunk(final_state, final_registers);
	write_ram_chunk(final_state, final_ram);
	test.put_chunk("FINA", final_state);

	write_cycles_chunk(test, cycle_list);

	chunk_buffer port_chunk;
	port_chunk.put_u32(0);
	test.put_chunk("PORT", port_chunk);

	return test;
}

static uint64_t test_seed(const opcode_info &info, uint32_t test_index)
{
	uint64_t stem_key = ((uint64_t)info.prefix << 8) | info.opcode;
	return 0x6809A11E5EEDULL ^ (stem_key << 24) ^ test_index;
}

static void write_metadata(const std::filesystem::path &output_directory)
{
	FILE *metadata = fopen((output_directory / "metadata.json").c_str(), "w");
	if (metadata == nullptr) {
		fprintf(stderr, "failed to write metadata.json\n");
		exit(1);
	}
	fprintf(metadata, "{\n");
	fprintf(metadata, "    \"cpu\": \"6809\",\n");
	fprintf(metadata,
		"    \"generator\": \"neetan m6809_testgen, MAME m6809 core, cycle model MC6809E\",\n");
	fprintf(metadata, "    \"opcodes\": {\n");

	struct metadata_entry
	{
		std::string stem;
		std::string line;
	};
	std::vector<metadata_entry> entries;
	for (const opcode_info &info : OPCODE_TABLE) {
		opcode_info stem_info = info;
		std::string stem = stem_string(stem_info, false);
		std::string line = std::string("\"mnemonic\": \"") + info.mnemonic + "\", \"status\": \""
			+ (info.documented ? "normal" : "undocumented") + "\"";
		entries.push_back({stem, line});
	}
	for (const skipped_opcode &skipped : SKIPPED_OPCODES) {
		opcode_info stem_info = {skipped.prefix, skipped.opcode, skipped.mnemonic,
			addressing_mode::INHERENT, -1, cycle_special::NONE, false};
		std::string stem = stem_string(stem_info, false);
		std::string line = std::string("\"mnemonic\": \"") + skipped.mnemonic
			+ "\", \"status\": \"skipped\", \"reason\": \"" + skipped.reason + "\"";
		entries.push_back({stem, line});
	}
	std::sort(entries.begin(), entries.end(),
		[](const metadata_entry &left, const metadata_entry &right) {
			return left.stem < right.stem;
		});
	for (size_t i = 0; i < entries.size(); i++)
		fprintf(metadata, "        \"%s\": { %s }%s\n", entries[i].stem.c_str(),
			entries[i].line.c_str(), i + 1 < entries.size() ? "," : "");

	fprintf(metadata, "    }\n");
	fprintf(metadata, "}\n");
	fclose(metadata);
}

static void run_sanity_checks();

int main(int argc, char **argv)
{
	std::string output_directory = "../v1";
	uint32_t test_count = 1000;
	std::string opcode_filter;
	bool verbose = false;

	for (int i = 1; i < argc; i++) {
		std::string argument = argv[i];
		if (argument == "--out" && i + 1 < argc) {
			output_directory = argv[++i];
		} else if (argument == "--count" && i + 1 < argc) {
			test_count = strtoul(argv[++i], nullptr, 10);
		} else if (argument == "--opcode" && i + 1 < argc) {
			opcode_filter = argv[++i];
		} else if (argument == "--verbose") {
			verbose = true;
		} else if (argument == "--sanity") {
			run_sanity_checks();
			return 0;
		} else {
			fprintf(stderr,
				"usage: m6809_testgen [--out DIR] [--count N] [--opcode STEM] [--verbose] [--sanity]\n");
			return 1;
		}
	}

	std::filesystem::path output_path = std::filesystem::absolute(output_directory);
	std::filesystem::create_directories(output_path);

	FILE *manifest = nullptr;
	if (opcode_filter.empty()) {
		manifest = fopen((output_path / "manifest.txt").c_str(), "w");
		if (manifest == nullptr) {
			fprintf(stderr, "failed to write manifest.txt\n");
			return 1;
		}
	}

	uint32_t generated_files = 0;
	for (const opcode_info &info : OPCODE_TABLE) {
		std::string stem_lower = stem_string(info, false);
		if (!opcode_filter.empty() && stem_lower != opcode_filter)
			continue;

		std::vector<chunk_buffer> tests;
		generation_summary summary;
		for (uint32_t test_index = 0; test_index < test_count; test_index++)
			tests.push_back(
				generate_test(info, test_index, test_seed(info, test_index), verbose, summary));

		std::string file_name = stem_lower + ".moo.gz";
		write_moo_file((output_path / file_name).c_str(), test_count, "6809", tests);

		printf("%-6s %-7s %4u tests  cycles %2d..%2d  checked %4u  exempt %4u\n",
			stem_lower.c_str(), info.mnemonic, test_count, summary.minimum_cycles,
			summary.maximum_cycles, summary.checked, summary.exempt);
		if (manifest != nullptr)
			fprintf(manifest, "%s %u\n", stem_lower.c_str(), test_count);
		generated_files += 1;
	}

	if (manifest != nullptr)
		fclose(manifest);

	if (opcode_filter.empty()) {
		size_t table_size = sizeof(OPCODE_TABLE) / sizeof(OPCODE_TABLE[0]);
		if (generated_files != table_size) {
			fprintf(stderr, "generated %u files but the opcode table has %zu entries\n",
				generated_files, table_size);
			return 1;
		}
		write_metadata(output_path);
	}

	printf("generated %u files\n", generated_files);
	return 0;
}

static void run_sanity_case(const char *name, const uint8_t *bytes, size_t length,
	int expected)
{
	m6809_base_device device;
	logging_memory *memory = new logging_memory();
	device.m_mintf.reset(memory);
	memory->device = &device;

	std::memset(memory->memory, 0x5A, sizeof(memory->memory));
	initialize_device(device);
	device.m_pc.w = 0x1000;
	device.m_s.w = 0x8000;
	device.m_u.w = 0;
	device.m_x.w = 0;
	device.m_y.w = 0;
	device.m_dp = 0;
	device.m_q.q = 0;
	device.m_cc = 0;
	for (size_t i = 0; i < length; i++)
		memory->memory[0x1000 + i] = bytes[i];

	int calls = 0;
	do {
		device.execute_one();
		if (++calls > 64) {
			fprintf(stderr, "%s did not terminate\n", name);
			exit(1);
		}
	} while (device.m_state != 0);
	int cycles = INITIAL_ICOUNT - device.m_icount;

	printf("%-12s cycles=%2d (expected %2d)%s\n", name, cycles, expected,
		cycles == expected ? "" : "  MISMATCH");
	if (cycles != expected)
		exit(1);
}

static void run_sanity_checks()
{
	const uint8_t nop[] = {0x12};
	const uint8_t lda_immediate[] = {0x86, 0x42};
	const uint8_t swi[] = {0x3F};
	const uint8_t ldd_extended[] = {0xFC, 0x20, 0x00};
	const uint8_t swi2[] = {0x10, 0x3F};
	const uint8_t mul[] = {0x3D};

	run_sanity_case("NOP", nop, sizeof(nop), 2);
	run_sanity_case("LDA #", lda_immediate, sizeof(lda_immediate), 2);
	run_sanity_case("SWI", swi, sizeof(swi), 19);
	run_sanity_case("LDD ext", ldd_extended, sizeof(ldd_extended), 6);
	run_sanity_case("SWI2", swi2, sizeof(swi2), 20);
	run_sanity_case("MUL", mul, sizeof(mul), 11);
	printf("all sanity checks passed\n");
}
