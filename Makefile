CC=g++
CXXFLAGS= --std=c++17 -Iinclude -o willow

default:
	$(CC) $(CXXFLAGS) src/willow/willow.cpp src/willow/parser/parser.cpp src/willow/semantics/type_manager.cpp src/willow/semantics/semantic_cube.cpp src/willow/parser/state.cpp src/willow/symbols/function_directory.cpp src/willow/symbols/class_directory.cpp src/willow/symbols/symbol_table.cpp src/willow/memory/memory_manager.cpp src/willow/vm/VM.cpp