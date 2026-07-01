-- EXPLAIN ANALYZE: prueba que cada indice SE USA de verdad.
--   Index Scan / Bitmap Index Scan / Index Only Scan -> uso el indice.
--   Seq Scan                                         -> leyo toda la tabla.

-- B-TREE: rango numerico 
EXPLAIN ANALYZE
SELECT id, categoria, medicion FROM sensores WHERE medicion > 90000 ORDER BY medicion;

-- HASH: igualdad exacta 
EXPLAIN ANALYZE
SELECT id, categoria, serie FROM sensores WHERE serie = md5('aire-rimac-07');

-- GiST: contencion espacial
EXPLAIN ANALYZE
SELECT id, categoria, coord FROM sensores WHERE coord <@ BOX(POINT(-5, -5), POINT(5, 5));

-- GiST: KNN por cercania 
EXPLAIN ANALYZE
SELECT id, categoria FROM sensores ORDER BY coord <-> POINT(0, 0) LIMIT 3;

-- ANTI-EJEMPLO: rango sobre la columna con indice HASH -> Seq Scan
-- (demuestra el limite del Hash: no soporta > ni <).
EXPLAIN ANALYZE
SELECT count(*) FROM sensores WHERE serie > md5('aire-rimac-07');
