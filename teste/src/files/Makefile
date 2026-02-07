# Makefile para DIV/Bennu Loader com Raylib
# 
# Dependências:
# - Raylib (https://www.raylib.com/)
#
# Instalação do Raylib:
# Ubuntu/Debian: sudo apt install libraylib-dev
# Fedora: sudo dnf install raylib-devel
# macOS: brew install raylib
# Windows: Baixe de https://www.raylib.com/

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
INCLUDES = -I. -I/usr/local/include
LIBS = -lraylib -lm

# Detectar sistema operacional
ifeq ($(OS),Windows_NT)
    LIBS += -lopengl32 -lgdi32 -lwinmm
    EXT = .exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LIBS += -lGL -lpthread -ldl -lrt
    endif
    ifeq ($(UNAME_S),Darwin)
        LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    endif
    EXT =
endif

# Arquivo biblioteca
LIB_OBJ = file_div_raylib.o

# Executáveis
TARGETS = div_loader_demo$(EXT) div_advanced_demo$(EXT) div_converter$(EXT)

# Alvo padrão - compilar tudo
all: $(TARGETS)

# Executável básico
div_loader_demo$(EXT): $(LIB_OBJ) example.o
	$(CC) $(LIB_OBJ) example.o -o div_loader_demo$(EXT) $(LIBS)
	@echo "Compilação concluída: div_loader_demo$(EXT)"

# Executável avançado
div_advanced_demo$(EXT): $(LIB_OBJ) advanced_example.o
	$(CC) $(LIB_OBJ) advanced_example.o -o div_advanced_demo$(EXT) $(LIBS)
	@echo "Compilação concluída: div_advanced_demo$(EXT)"

# Conversor
div_converter$(EXT): $(LIB_OBJ) div_converter.o
	$(CC) $(LIB_OBJ) div_converter.o -o div_converter$(EXT) $(LIBS)
	@echo "Compilação concluída: div_converter$(EXT)"

# Compilar arquivos objeto
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Limpar arquivos compilados
clean:
	rm -f *.o $(TARGETS)

# Recompilar tudo
rebuild: clean all

# Executar demo básico
run: div_loader_demo$(EXT)
	./div_loader_demo$(EXT)

# Executar demo avançado
run-advanced: div_advanced_demo$(EXT)
	./div_advanced_demo$(EXT)

# Executar conversor (exemplo)
convert-example:
	@echo "Uso do conversor:"
	@echo "./div_converter$(EXT) input.fpg output_dir/"
	@echo "./div_converter$(EXT) input.map output.png"
	@echo "./div_converter$(EXT) input.fnt output_dir/"

# Instalação de dependências (apenas informativo)
install-deps:
	@echo "Por favor, instale o Raylib manualmente:"
	@echo "Ubuntu/Debian: sudo apt install libraylib-dev"
	@echo "Fedora: sudo dnf install raylib-devel"
	@echo "macOS: brew install raylib"
	@echo "Windows: https://www.raylib.com/"

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)

# Informações sobre dependências
.PHONY: all clean rebuild run install-deps debug

# Dependências de cabeçalhos
file_div_raylib.o: file_div_raylib.c file_div_raylib.h
example.o: example.c file_div_raylib.h
