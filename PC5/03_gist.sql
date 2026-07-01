-- INDICE GiST  (columna: coord)  == el "R-Tree" espacial de Postgres
-- Opera la geometria sirve para: contencion, interseccion, proximidad (KNN), igualdad espacial.

-- Fortaleza 1) Contencion (<@): sensores DENTRO de una caja cerca del origen.
--              La caja son 2 esquinas: (xmin,ymin) y (xmax,ymax).
SELECT id, categoria, coord
FROM sensores
WHERE coord <@ BOX(POINT(-5, -5), POINT(5, 5))
ORDER BY id;

-- Fortaleza 2) Proximidad / KNN (<->): los 3 sensores MAS CERCANOS al origen.
SELECT id, categoria, coord, coord <-> POINT(0, 0) AS distancia
FROM sensores
ORDER BY coord <-> POINT(0, 0)
LIMIT 3;

-- Fortaleza 3) Igualdad espacial (~=): quien esta EXACTAMENTE en (2, 1).
SELECT id, categoria FROM sensores WHERE coord ~= POINT(2, 1);

-- Fortaleza 4) Contiene (@>): puntos dentro de un circulo de radio 10 en el origen.
SELECT id, categoria, coord
FROM sensores
WHERE CIRCLE(POINT(0, 0), 10) @> coord
ORDER BY id;
