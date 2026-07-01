-- INDICE B-TREE  (columna: medicion)
-- Mantiene las claves ORDENADAS -> igualdad, rangos, IN y ORDER BY.
-- Operadores: =, <, <=, >=, >, BETWEEN, IN, IS NULL.

-- Fortaleza 1) Igualdad exacta.
SELECT id, categoria, medicion FROM sensores WHERE medicion = 1200;

-- Fortaleza 2) Rango (esto Hash NO puede).
SELECT id, categoria, medicion FROM sensores WHERE medicion > 90000 ORDER BY medicion;

-- Fortaleza 3) BETWEEN (rango cerrado).
SELECT id, categoria, medicion FROM sensores WHERE medicion BETWEEN 100 AND 1000 ORDER BY medicion;

-- Fortaleza 4) IN (varias igualdades).
SELECT id, categoria, medicion FROM sensores WHERE medicion IN (40, 150, 500, 800);

-- Fortaleza 5) ORDER BY: el indice ya viene ordenado, evita el paso de sort.
SELECT id, categoria, medicion FROM sensores ORDER BY medicion DESC LIMIT 5;

-- Limite) al aplicar una funcion sobre la columna el B-Tree no se puede usar -> Seq Scan (leer toda la tabla).
SELECT count(*) FROM sensores WHERE abs(medicion) = 500;
