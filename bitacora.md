# Bitácora de Desarrollo - Generador de Tablas LR(1)

**Curso:** Compiladores (CS342) - UCSP  
**Proyecto:** Generador de Tablas LR(1) en C++17  

---

## 1. Descripción del Diseño e Implementación

El proyecto implementa un generador automático de tablas de análisis sintáctico LR(1) a partir de cualquier gramática libre de contexto escrita en un archivo de texto.

### Componentes Clave:
1. **Representación de Ítems LR(1)**:  
   Cada ítem LR(1) se representa estrictamente como una tripleta de enteros `[rule_id, dot_pos, la_id]`. Esto garantiza una comparación de alta eficiencia y la ordenación determinista en `std::set<Item>`.

2. **Cálculo de FIRST y FIRST de Cadenas**:  
   - `first()`: Calcula mediante un algoritmo de punto fijo el conjunto `FIRST` de todos los símbolos (terminales y no terminales), registrando la marca de vacuidad `ε` para no terminales anulables.
   - `firstCadena(beta)`: Evalúa el conjunto `FIRST` de una secuencia $\beta = s_1 s_2 \dots s_k$.

3. **Colección Canónica LR(1) Determinista**:  
   - `closure(s)`: Extiende un conjunto de ítems procesando las producciones de no terminales inmediatamente después del punto.
   - `gotoState(s, x)`: Calcula los ítems alcanzables al reconocer el símbolo $x$.
   - `coleccionCanonica()`: Construye los estados $CC = \{cc_0, cc_1, \dots\}$ evaluando las transiciones según el **orden exacto de primera aparición de los símbolos en la gramática** (excluyendo el símbolo de inicio).

4. **Construcción de Tablas y Manejo de Conflictos (`llenarTablas`)**:  
   - Registra acciones `shift j`, `reduce k` y `accept` en la matriz `Action`, y transiciones de estados en `Goto`.
   - Si una celda en `Action` recibe dos acciones distintas (Shift/Reduce o Reduce/Reduce), el sistema **no escoge ninguna arbitrariamente**: reporta el estado, el terminal y las acciones en conflicto, e interrumpe la ejecución retornando código de salida no cero (`exit(1)`).

---

## 2. Batería de Pruebas y Resultados

La batería de pruebas se organizó en el directorio `tests/`:

| Archivo de Prueba | Tipo de Prueba | Resultado Esperado | Resultado Obtenido |
| :--- | :--- | :--- | :--- |
| `tests/test_parentesis.txt` | Gramática de Referencia (PDF pág 1-2) | 12 estados deterministas sin conflictos, coincidencias exactas con la tabla del PDF. | **EXITOSO** (0 conflictos, coincidencia 100%). |
| `tests/test_vacias.txt` | Gramática con producciones vacías ($\varepsilon$) | Cálculo correcto de FIRST con $\varepsilon$ y tabla LR(1) válida. | **EXITOSO** (0 conflictos, código de salida 0). |
| `tests/test_shift_reduce.txt` | Detección de conflicto Shift/Reduce (Dangling Else) | Detección explícita de conflicto en Estado 24 (`s25 vs r2`) y código de salida 1. | **EXITOSO** (Conflicto detectado, código de salida 1). |
| `tests/test_reduce_reduce.txt` | Detección de conflicto Reduce/Reduce | Detección explícita de conflicto en Estado 4 (`r3 vs r4`) y código de salida 1. | **EXITOSO** (Conflicto detectado, código de salida 1). |

---

## 3. Compilación y Ejecución

El proyecto incluye un `Makefile` compilable con la herramienta estándar `make`:

```bash
make
./parser tests/test_parentesis.txt
```

Para limpiar los binarios e intermediarios:
```bash
make clean
```
