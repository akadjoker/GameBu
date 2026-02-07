#!/bin/bash
# Script de teste - Verifica se tudo compila corretamente

echo "================================"
echo "DIV Loader - Build Test Script"
echo "================================"
echo ""

# Cores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Contador de erros
ERRORS=0

# Função para verificar comando
check_command() {
    if command -v $1 &> /dev/null; then
        echo -e "${GREEN}✓${NC} $1 encontrado"
        return 0
    else
        echo -e "${RED}✗${NC} $1 não encontrado"
        return 1
    fi
}

# Verificar dependências
echo "1. Verificando dependências..."
echo ""

if ! check_command gcc; then
    ERRORS=$((ERRORS + 1))
    echo -e "${YELLOW}  Instale com: sudo apt install build-essential${NC}"
fi

if ! check_command make; then
    ERRORS=$((ERRORS + 1))
    echo -e "${YELLOW}  Instale com: sudo apt install make${NC}"
fi

echo ""

# Verificar Raylib
echo "2. Verificando Raylib..."
echo ""

# Tentar compilar um programa de teste
cat > /tmp/raylib_test.c << 'EOF'
#include <raylib.h>
int main(void) {
    return 0;
}
EOF

if gcc /tmp/raylib_test.c -lraylib -o /tmp/raylib_test 2>/dev/null; then
    echo -e "${GREEN}✓${NC} Raylib está instalado"
    rm -f /tmp/raylib_test /tmp/raylib_test.c
else
    echo -e "${RED}✗${NC} Raylib não está instalado"
    ERRORS=$((ERRORS + 1))
    echo -e "${YELLOW}  Ubuntu/Debian: sudo apt install libraylib-dev${NC}"
    echo -e "${YELLOW}  macOS: brew install raylib${NC}"
    rm -f /tmp/raylib_test.c
fi

echo ""

# Verificar arquivos do projeto
echo "3. Verificando arquivos do projeto..."
echo ""

FILES=(
    "file_div_raylib.h"
    "file_div_raylib.c"
    "example.c"
    "advanced_example.c"
    "div_converter.c"
    "Makefile"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo -e "${GREEN}✓${NC} $file"
    else
        echo -e "${RED}✗${NC} $file não encontrado"
        ERRORS=$((ERRORS + 1))
    fi
done

echo ""

# Tentar compilar
if [ $ERRORS -eq 0 ]; then
    echo "4. Compilando projeto..."
    echo ""
    
    if make clean > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} make clean"
    fi
    
    if make all 2>&1 | tee /tmp/build.log; then
        echo ""
        echo -e "${GREEN}✓${NC} Compilação bem-sucedida!"
        echo ""
        echo "Executáveis criados:"
        ls -lh div_*demo* div_converter* 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
    else
        echo ""
        echo -e "${RED}✗${NC} Erro na compilação"
        echo ""
        echo "Log de erros:"
        tail -20 /tmp/build.log
        ERRORS=$((ERRORS + 1))
    fi
else
    echo "4. ${YELLOW}Pulando compilação devido a erros anteriores${NC}"
fi

echo ""
echo "================================"

# Resumo
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}SUCESSO! Tudo está funcionando.${NC}"
    echo ""
    echo "Próximos passos:"
    echo "  1. Execute: ./div_loader_demo"
    echo "  2. Execute: ./div_advanced_demo"
    echo "  3. Veja o QUICKSTART.md para mais informações"
else
    echo -e "${RED}FALHOU com $ERRORS erro(s).${NC}"
    echo ""
    echo "Por favor, corrija os erros acima antes de continuar."
fi

echo "================================"
echo ""

exit $ERRORS
