-- INDICE HASH 
-- Hace UNA sola cosa muy bien: igualdad exacta (=), en O(1) promedio.
.

-- Fortaleza 1) Busqueda exacta por token (caso ideal del Hash).
SELECT id, categoria, serie FROM sensores WHERE serie = md5('aire-rimac-07');

-- Fortaleza 2) Otra igualdad estricta.
SELECT categoria, medicion FROM sensores WHERE serie = md5('clima-lima-08');

-- Limite 1) Rango: el Hash NO ordena las claves -> no sirve para >, < ...
SELECT count(*) FROM sensores WHERE serie > md5('aire-rimac-07');

-- Limite 2) ORDER BY: tampoco puede entregar filas ordenadas por serie.
SELECT serie FROM sensores ORDER BY serie LIMIT 5;

-- Limite 3) Coincidencia parcial (LIKE): el Hash solo entiende "igual", no patrones.
SELECT count(*) FROM sensores WHERE serie LIKE 'a%';
