# 컴파일러 변수 선언
CC=gcc 
FLAGS=-Wall -pedantic 
INC=-Isrc/ 
CFLAGS=$(FLAGS) -c -g --std=c99 $(INC) `sdl-config --cflags` 
LFLAGS=$(FLAGS) -lncurses `sdl-config --libs` -lSDL_mixer 
DIR_GUARD=@mkdir -p $(@D) 

# 빌드 구성
CFG=release 
ifeq ($(CFG),debug) 
FLAGS += -g -DDEBUG -DSMB_DEBUG 
endif 
ifneq ($(CFG),debug) 
ifneq ($(CFG),release) 
       @echo "Invalid configuration "$(CFG)" specified." 
       @echo "You must specify a configuration when running make, e.g." 
     	 @echo "  make CFG=debug" 
       @echo "Choices are 'release', 'debug'." 
    	 @exit 1 
endif 
endif 
  
# 소스 및 객체
SOURCES=$(shell find src/ -type f -name "*.c") 
OBJECTS=$(patsubst src/%.c,obj/$(CFG)/%.o,$(SOURCES)) 
DEPS=$(patsubst src/%.c,deps/%.d,$(SOURCES)) 
 
# 주요 대상
.PHONY: all clean clean_all 
  
all: bin/$(CFG)/main 
 
GTAGS: $(SOURCES) 
       gtags 
  
clean: 
       rm -rf obj/$(CFG)/* bin/$(CFG)/* src/*.gch GTAGS GPATH GRTAGS 
  
clean_all: 
       rm -rf bin/* obj/* deps/* 
  
# 컴파일 규칙
obj/$(CFG)/%.o: src/%.c 
      $(DIR_GUARD) 
    	$(CC) $(CFLAGS) $< -o $@ 
  
# 연결 규칙
bin/$(CFG)/main: $(OBJECTS) 
       $(DIR_GUARD) 
       $(CC) $(OBJECTS) $(LFLAGS) -o bin/$(CFG)/main 
 
# --- Dependency Rule 
deps/%.d: src/%.c 
       $(DIR_GUARD) 
       $(CC) $(CFLAGS) -MM $< | sed -e 's/~\(.*\)\.o:/\1.d \1.o:/' > $@ 
  
ifneq "$(MAKECMDGOALS)" "clean_all" 
-include $(DEPS) 
endif 
