LINKER        = g++
LINKERFLAGS   =  
COMPILER      = g++
COMPILERFLAGS =  -std=c++14 -Wall -g
BIN           = test-aut
LIBS          = -lboost_filesystem -lboost_system
vpath %.cpp src
vpath %.o build
OBJ           = test-aut.o ndfa.o get_init_state.o groupped_dfa.o slr_act_expr_parser.o print_char32.o search_char.o sets_for_classes.o dfa.o print_commands.o act_expr_parser.o expr_scaner.o print_size_t.o error_count.o categories.o aux_expr_scaner.o groupped_dfa_by_regexp.o minimal_dfa.o expr_lr_tables.o symbol.o char_conv.o file_contents.o char_trie.o fsize.o fuse_commands.o
LINKOBJ       = build/test-aut.o build/ndfa.o build/get_init_state.o build/groupped_dfa.o build/slr_act_expr_parser.o build/print_char32.o build/search_char.o build/sets_for_classes.o build/dfa.o build/print_commands.o build/act_expr_parser.o build/expr_scaner.o build/print_size_t.o build/error_count.o build/categories.o build/aux_expr_scaner.o build/groupped_dfa_by_regexp.o build/minimal_dfa.o build/expr_lr_tables.o build/symbol.o build/char_conv.o build/file_contents.o build/char_trie.o build/fsize.o build/fuse_commands.o

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom 
	rm -f ./build/*.o
	rm -f ./build/$(BIN)

.cpp.o:
	$(COMPILER) -c $< -o $@ $(COMPILERFLAGS) 
	mv $@ ./build

$(BIN):$(OBJ)
	$(LINKER) -o $(BIN) $(LINKOBJ) $(LIBS) $(LINKERFLAGS)
	mv $(BIN) ./build