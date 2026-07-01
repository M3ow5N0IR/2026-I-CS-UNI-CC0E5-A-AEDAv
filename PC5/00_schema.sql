-- PC5 (SQL) - Exploracion de indices en PostgreSQL

DROP TABLE IF EXISTS sensores CASCADE;

CREATE TABLE sensores (
    id           SERIAL       PRIMARY KEY,
    serie        TEXT         NOT NULL,   -- HASH : codigo unico del dispositivo (md5)
    categoria    VARCHAR(40)  NOT NULL,   -- descriptivo (sin indice)
    medicion     INTEGER      NOT NULL,   -- B-TREE : valor leido (rango/orden)
    capturado_en TIMESTAMP    NOT NULL DEFAULT NOW(),  -- fecha de la lectura
    coord        POINT        NOT NULL    -- GiST : ubicacion (x, y) del sensor
);

-- Filas "con nombre" ubicadas cerca del origen (objetivo de las demos espaciales).
INSERT INTO sensores (serie, categoria, medicion, capturado_en, coord) VALUES
    (md5('aire-centro-01'),  'aire',    500,   NOW() - INTERVAL '5 minutes',  POINT(0, 0)),
    (md5('aire-norte-02'),   'aire',    800,   NOW() - INTERVAL '12 minutes', POINT(2, 1)),
    (md5('ruido-sur-03'),    'ruido',   150,   NOW() - INTERVAL '20 minutes', POINT(3, -2)),
    (md5('clima-este-04'),   'clima',   95000, NOW() - INTERVAL '40 minutes', POINT(-4, 4)),
    (md5('clima-oeste-05'),  'clima',   40,    NOW() - INTERVAL '2 hours',    POINT(50, 50)),
    (md5('ruido-borde-06'),  'ruido',   99999, NOW() - INTERVAL '3 days',     POINT(120, 80)),
    (md5('aire-rimac-07'),   'aire',    1200,  NOW() - INTERVAL '10 minutes', POINT(1, 1)),
    (md5('clima-lima-08'),   'clima',   33000, NOW() - INTERVAL '1 hour',     POINT(-1, -1));

-- Relleno de 2000 filas 
INSERT INTO sensores (serie, categoria, medicion, capturado_en, coord)
SELECT md5('bulk-' || g),
       (ARRAY['aire','ruido','clima'])[1 + (g % 3)],
       (g * 7919) % 100000,
       NOW() - (g || ' minutes')::interval,
       POINT((g % 400) - 200, (g % 300) - 150)
FROM generate_series(1, 2000) AS g;

-- Un indice por tipo .
CREATE INDEX idx_sensores_serie_hash   ON sensores USING hash  (serie);      -- HASH
CREATE INDEX idx_sensores_medicion_bt  ON sensores USING btree (medicion);   -- B-TREE
CREATE INDEX idx_sensores_coord_gist   ON sensores USING gist  (coord);      -- GiST

-- Actualiza estadisticas para que EXPLAIN refleje la realidad.
ANALYZE sensores;

-- Comprobar los indices creados.
SELECT indexname, indexdef
FROM pg_indexes
WHERE tablename = 'sensores'
ORDER BY indexname;
