# MC6809 single instruction test vectors

Single-step test vectors for the Motorola MC6809, in the same MOO format as the
SingleStepTests corpora used by the other CPU verification tests. There is no publicly
available SingleStepTests corpus for the 6809, so these vectors were generated with the
included generator, which runs single instructions with fully random state on the MAME
m6809 core and records the resulting state and bus traffic.

## Layout

- `v1/<stem>.moo.gz` - 1000 tests per opcode. The stem is the lowercase hex encoding of
  the instruction including the page prefix, for example `3f.moo.gz` or `10 a3.moo.gz`.
- `v1/metadata.json` - per opcode status: `normal`, `undocumented` (behavior MAME
  implements without an official specification) or `skipped`.
- `v1/manifest.txt` - one `<stem> <test count>` line per generated file.
- `testgen/` - the test vector generator. The MAME m6809 core files it executes are
  hard copies in `testgen/mame/`.

The standalone register-transfer helpers apply the mixed-width CC/DP behavior observed
on a real FM-77AV and documented by 77AVEMU. This intentionally differs from MAME's
current CC/DP duplication behavior.

Skipped opcodes: `13` (SYNC) and `3c` (CWAI) wait for an interrupt line and never
complete in a single step; `14`, `15` and `cd` (FREERUN) enter a CPU test mode that free
runs until reset.

## Register order

The `REGS` chunk uses this register order for the bitmask (all values stored as u16,
the 8 bit registers zero-extended):

```
["pc", "s", "u", "x", "y", "dp", "a", "b", "cc"]
```

## Cycle encoding

Cycles use the legacy MOO cycle layout: u8 pin bitfield (bit 0 = address valid, bit 1 =
data valid), u16 address, u8 data, 4 byte ASCII status. One entry per machine cycle:

- `r-m-` - memory read (pin bitfield 0x03)
- `-wm-` - memory write (pin bitfield 0x03)
- `----` - internal cycle without bus access (pin bitfield 0x00); on the MC6809 this
  only occurs inside the undocumented opcode `38`

Dummy bus cycles are recorded as ordinary reads, matching the MC6809 bus behavior: dead
cycles read from `0xFFFF` (VMA cycles) or re-read the instruction stream at PC. The
cycle list length is the exact machine cycle count of the instruction. An emulator
verified against these traces must model the same bus pattern: dead cycles perform real
reads (from `0xFFFF` or the instruction stream), they are not idle cycles.

## Test content

- `BYTS` holds the instruction bytes as they were in memory at PC before execution.
  For the undocumented store-immediate opcodes (`8f`, `cf`, `10 8f`, `10 cf`, `11 8f`,
  `11 cf`) the last byte is overwritten by the instruction itself.
- Initial and final RAM list the same address set: the instruction bytes plus every
  address the instruction accessed.
- Registers, DP, CC and the full 64 KB of memory are random per test. Interrupt lines
  are held clear. PC may wrap across `0xFFFF`.
- The `PORT` chunk is always empty and no `EXCP`/`QUEU`/`HASH` chunks are written.

## Reproducibility

Tests are generated deterministically with a splitmix64 RNG seeded per test:

```
seed = 0x6809A11E5EED xor (((prefix << 8) | opcode) << 24) xor test_index
```

Regenerate with:

```bash
cd testgen
make
./build/m6809_testgen
```

During generation every documented opcode is cross-checked against the MC6809 datasheet
cycle table (including indexed postbyte penalties, long branch taken timing, PSH/PUL
register mask costs and the RTI E flag split).

## License

Test data generator is licensed under BSD-3.
Test data itself is licensed under MIT-0.
