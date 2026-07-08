MAME_M6809 := mame
CXX := g++
CXXFLAGS := -O2 -std=c++17 -Wall -Wno-unused-label -Ibuild -I.

SOURCES := main.cpp m6809_core.cpp moo_writer.cpp
HEADERS := m6809_shim.h moo_writer.h opcode_table.h

all: build/m6809_testgen

build/m6809.hxx: $(MAME_M6809)/m6809.lst $(MAME_M6809)/base6x09.lst $(MAME_M6809)/m6809make.py
	mkdir -p build
	python3 $(MAME_M6809)/m6809make.py $(MAME_M6809)/m6809.lst > $@

build/m6809_testgen: $(SOURCES) $(HEADERS) build/m6809.hxx
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) -lz

clean:
	rm -rf build

.PHONY: all clean
