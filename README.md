# Motor de búsqueda documental en C++

Sistema completo de **recuperación de información**: indexa una colección de
documentos y responde consultas devolviendo los más relevantes, ordenados por
puntuación.

Implementado desde cero en **C++**, sin librerías externas de indexación: el
tokenizador, el algoritmo de *stemming*, el índice invertido y los modelos de
ranking están escritos a mano.

**~3.400 líneas de C++** · `g++` + `make`

---

## Qué hace

```
documentos  →  tokenizar  →  stemming  →  índice invertido
                                                 ↓
consulta    →  tokenizar  →  stemming  →  BM25 / DFR  →  documentos ordenados
```

Sobre un corpus de **717 prospectos de medicamentos** y la colección de
referencia **TIME** (423 documentos y 83 consultas con relevancias conocidas),
que permite medir objetivamente la calidad de los resultados.

---

## Arquitectura

| Módulo |  | Responsabilidad |
|---|---:|---|
| `tokenizador` |  | Trocear el texto en términos |
| `stemmer` |  | Reducir cada palabra a su raíz |
| `indexadorHash` |  | Índice invertido con tablas hash |
| `indexadorInformacion` |  | Estructuras de datos del índice |
| `buscador` |  | Modelos de ranking y consultas |

### `Tokenizador`

Separa el texto en términos con delimitadores configurables. La parte con más
detalle son los **casos especiales**, donde una separación ingenua destrozaría el
significado:

- URLs y correos electrónicos (`https://ua.es`, `nombre@dominio.com`)
- Números con decimales y separador de miles (`1.234,56`)
- Acrónimos (`E.E.U.U.`) y palabras con guion
- Normalización opcional a minúsculas y **eliminación de acentos**

Internamente usa un `array<bool,256>` para consultar si un carácter es
delimitador en tiempo constante, en lugar de recorrer la cadena de delimitadores
en cada carácter.

### `Stemmer`

Implementación del **algoritmo de Porter** para español e inglés: reduce las
variantes de una palabra a una raíz común, de modo que *"comprimido"*,
*"comprimidos"* y *"comprimida"* se indexan como el mismo término. Mejora la
cobertura de las búsquedas sin necesidad de que el usuario acierte con la forma
exacta.

### `IndexadorHash`

El **índice invertido**, construido sobre `unordered_map`:

```cpp
unordered_map<string, InformacionTermino>  indice;         // término → documentos
unordered_map<string, InfDoc>              indiceDocs;     // documento → metadatos
unordered_map<string, InformacionTerminoPregunta> indicePregunta;  // consulta actual
```

Características:

- **Reindexado incremental:** si un documento ya está indexado, se compara su
  fecha de modificación y solo se vuelve a procesar si ha cambiado, conservando
  el mismo identificador interno.
- **Palabras vacías** (*stop words*): listas para español e inglés, para no
  indexar términos sin valor discriminante.
- **Persistencia:** el índice se guarda y se recupera de disco, evitando
  reconstruirlo en cada ejecución.

### `Buscador`

Hereda de `IndexadorHash` y añade el cálculo de relevancia con **dos modelos de
ranking** intercambiables:

- **BM25** — parámetros `k1` (saturación de frecuencia) y `b` (normalización por
  longitud del documento). Valores por defecto: `k1 = 1.2`, `b = 0.75`.
- **DFR** (*Divergence From Randomness*) — parámetro `c`.

Ambos son ajustables en tiempo de ejecución, lo que permite comparar
configuraciones sobre el mismo índice.

Los resultados se emiten en **formato TREC**, el estándar de la disciplina:

```
1 BM25 EFE19950609-05926 0 64.7059 ConjuntoDePreguntas
```

---

## Evaluación

La calidad del buscador se mide con **`trec_eval`**, la herramienta estándar de
evaluación de sistemas de recuperación de información, sobre la colección TIME
con sus relevancias de referencia.

Se generaron cuatro configuraciones para comparar el efecto de cada decisión:

```
indice_BM25_conStem     indice_DFR_conStem
indice_BM25_sinStem     indice_DFR_sinStem
```

De ahí salen las métricas de **precisión y cobertura** y la curva que las
relaciona, que es la forma habitual de comparar buscadores: no basta con
devolver documentos, hay que devolver los correctos y no dejarse ninguno fuera.

---

## Compilación y uso

```bash
make                    # compila el ejecutable 'buscador'
make test               # compila y ejecuta las pruebas
make clean
```

Pruebas por módulo en `src/tad01.cpp` … `src/tad07.cpp`, cada una con su salida
esperada en el `.sal` correspondiente, para comprobar por comparación directa que
un cambio no rompe nada.

---

## Estructura

```
include/          cabeceras de los cinco módulos
lib/              implementaciones (.cpp)
src/              main y programas de prueba por módulo
corpus/           717 prospectos de medicamentos
CorpusTime/       colección TIME: documentos, consultas y relevancias
indice_*/         índices generados con cada configuración
StopWords*.txt    listas de palabras vacías (español e inglés)
trec_eval*/       herramienta de evaluación
makefile
```
