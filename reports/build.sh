#!/bin/bash

# Colores para la terminal
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Directorios dinámicos (funciona en cualquier máquina/carpeta sin rutas absolutas fijas)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
REPORT_DIR="$SCRIPT_DIR"
OUTPUT_DIR="$REPORT_DIR/output"
MAIN_TEX="main.tex"

echo -e "${BLUE}\n===== COMPILADOR LaTeX AUTOMÁTICO =====${NC}"
echo -e "${BLUE}Entorno optimizado estilo Overleaf${NC}\n"

# 1. Asegurar directorio de trabajo y carpeta de salida
cd "$REPORT_DIR" || { echo -e "${RED}Error: No se pudo acceder a $REPORT_DIR${NC}"; exit 1; }
mkdir -p "$OUTPUT_DIR"

# 2. Compilación inteligente
# Si latexmk está disponible, lo usa directamente (maneja pasadas e índices automáticamente)
if command -v latexmk >/dev/null 2>&1; then
    echo -e "${BLUE}Compilando con latexmk...${NC}"
    latexmk -pdf -interaction=nonstopmode -outdir="$OUTPUT_DIR" "$MAIN_TEX"
    COMPILATION_STATUS=$?
else
    echo -e "${YELLOW}Advertencia: 'latexmk' no detectado. Ejecutando pasadas directas con pdflatex...${NC}"
    echo -e "${BLUE}[1/3] Generando estructura base...${NC}"
    pdflatex -interaction=nonstopmode -output-directory="$OUTPUT_DIR" "$MAIN_TEX" >/dev/null 2>&1
    echo -e "${BLUE}[2/3] Actualizando índices y tablas...${NC}"
    pdflatex -interaction=nonstopmode -output-directory="$OUTPUT_DIR" "$MAIN_TEX" >/dev/null 2>&1
    echo -e "${BLUE}[3/3] Resolviendo referencias cruzadas...${NC}"
    pdflatex -interaction=nonstopmode -output-directory="$OUTPUT_DIR" "$MAIN_TEX"
    COMPILATION_STATUS=$?
fi

# 3. Verificar y procesar el PDF resultante
if [ $COMPILATION_STATUS -eq 0 ] && [ -f "$OUTPUT_DIR/main.pdf" ]; then
    echo -e "${GREEN}\n✓ Compilación EXITOSA${NC}"

    # Copiar resultado a la raíz del proyecto
    cp "$OUTPUT_DIR/main.pdf" "$PROJECT_ROOT/DR-VF-01_Reporte_Final.pdf"

    echo -e "• PDF generado: ${GREEN}$OUTPUT_DIR/main.pdf${NC}"
    echo -e "• Copia en raíz: ${GREEN}$PROJECT_ROOT/DR-VF-01_Reporte_Final.pdf${NC}"

    # Información de métricas
    FILE_SIZE=$(du -h "$OUTPUT_DIR/main.pdf" | cut -f1)
    if command -v pdfinfo >/dev/null 2>&1; then
        PAGE_COUNT=$(pdfinfo "$OUTPUT_DIR/main.pdf" | grep "Pages:" | awk '{print $2}')
        echo -e "• Tamaño: ${BLUE}$FILE_SIZE${NC} | Páginas: ${BLUE}$PAGE_COUNT${NC}"
    else
        echo -e "• Tamaño: ${BLUE}$FILE_SIZE${NC}"
    fi

    # 4. Verificación de integridad de índices
    echo -e "\n${BLUE}Verificación de índices:${NC}"
    for file in "main.toc" "main.lof" "main.lot"; do
        if [ -s "$OUTPUT_DIR/$file" ]; then
            lines=$(wc -l < "$OUTPUT_DIR/$file" | tr -d ' ')
            echo -e "  ✓ ${file} (${lines} líneas)"
        elif [ -f "$OUTPUT_DIR/$file" ]; then
            echo -e "  ${YELLOW}⚠ ${file} (Generado pero vacío)${NC}"
        else
            echo -e "  ${RED}✗ ${file} (No generado)${NC}"
        fi
    done

    echo -e "\n${GREEN}¡Proceso completado con éxito!${NC}\n"
else
    echo -e "${RED}\n✗ ERROR de compilación. Revisa los detalles en: $OUTPUT_DIR/main.log${NC}\n"
    exit 1
fi