# PC5 (SQL) — Exploración de índices en PostgreSQL

Exploración de **3 tipos de índice** en PostgreSQL —**B-Tree**, **Hash** y **R-Tree espacial (GiST)**— sobre una **única tabla `sensores`** con **una columna por tipo de índice**. Cada script muestra las **fortalezas** y los **límites** del índice.

## Tabla `sensores` (una columna por índice)

| Columna | Tipo | Índice | Para qué |
|---|---|---|---|
| `serie` | `TEXT` | **HASH** | identificador exacto del dispositivo (md5) |
| `medicion` | `INTEGER` | **B-TREE** | valor leído → rangos y orden |
| `coord` | `POINT` | **GiST** | ubicación (x, y) → consultas espaciales |

> `capturado_en` y `categoria` son columnas descriptivas de apoyo (sin índice propio).

## Archivos (orden de ejecución)

| # | Archivo | Contenido |
|---|---|---|
| 0 | [`00_schema.sql`](00_schema.sql) | Crea `sensores`, inserta datos (8 con nombre + 2000 de relleno) y los 3 índices. |
| 1 | [`01_btree.sql`](01_btree.sql) | B-Tree: igualdad, rango, `BETWEEN`, `IN`, `ORDER BY` + límite (condición no sargable). |
| 2 | [`02_hash.sql`](02_hash.sql) | Hash: igualdad exacta + límites (rango, `ORDER BY`, `LIKE`). |
| 3 | [`03_gist.sql`](03_gist.sql) | GiST (R-Tree): contención `<@`, KNN `<->`, igualdad espacial `~=`, contiene `@>`. |
| 4 | [`04_explain.sql`](04_explain.sql) | `EXPLAIN ANALYZE`: comprueba que cada índice se usa (y un anti-ejemplo). |

Ejecución:
```bash
psql -d mi_base -f 00_schema.sql
psql -d mi_base -f 01_btree.sql
psql -d mi_base -f 02_hash.sql
psql -d mi_base -f 03_gist.sql
psql -d mi_base -f 04_explain.sql
```

---

## 1. B-Tree — el índice por defecto

- **Cómo funciona:** árbol balanceado que mantiene las claves **ordenadas**; búsqueda en **O(log n)**.
- **Fortalezas:** igualdad (`=`), rangos (`<`, `>`, `BETWEEN`), `IN` y `ORDER BY` (ya viene ordenado, evita el *sort*).
- **Límites:** una condición **no sargable** (una función sobre la columna, p. ej. `abs(medicion)=500`) impide usarlo; tampoco sirve para búsquedas espaciales/proximidad.
- **Sintaxis:** `USING btree` (o se omite, es el tipo por defecto).

## 2. Hash — igualdad exacta ultrarrápida

- **Cómo funciona:** aplica una función hash a la clave y reparte en *buckets*; igualdad en **O(1)** promedio.
- **Fortalezas:** comparación de **igualdad estricta** (`=`), ideal para tokens/hashes.
- **Límites:** **solo** `=`. No soporta rangos (`<`, `>`), `ORDER BY` ni `LIKE` → esas consultas caen en *Seq Scan*.
- **Sintaxis:** `USING hash` (debe indicarse explícitamente).

## 3. R-Tree espacial = GiST

- **Cómo funciona:** Postgres no tiene un tipo *R-Tree* nativo; implementa esa estructura dentro de **GiST**, agrupando la geometría en **bounding boxes** que pueden superponerse.
- **Fortalezas:** contención (`<@`), contiene (`@>`), intersección (`&&`), igualdad espacial (`~=`) y **KNN** por distancia (`<->`).
- **Límites:** no ordena escalares como el B-Tree ni responde al `=` normal sobre `POINT`; para datos GPS reales se suele usar **PostGIS** (que también se apoya en GiST).
- **Sintaxis:** `USING gist`.

---

## Tabla comparativa

| Característica | B-Tree | Hash | R-Tree / GiST |
|---|---|---|---|
| Caso de uso | rangos, orden e igualdad | igualdad exacta ultrarrápida | datos geométricos/espaciales |
| Complejidad | O(log n) | O(1) promedio | O(log n) espacial |
| Soporta rangos (`<`, `>`) | Sí | No | No (usa inclusión/proximidad) |
| Soporta `ORDER BY` | Sí | No | Sí, pero por distancia (`<->`) |
| Operadores | `=`, `<`, `<=`, `>=`, `>`, `BETWEEN`, `IN` | `=` | `<@`, `@>`, `&&`, `~=`, `<->` |
| Sintaxis | `USING btree` (u omitido) | `USING hash` | `USING gist` |
