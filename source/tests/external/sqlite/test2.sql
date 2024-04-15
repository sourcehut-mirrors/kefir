
.tables
.schema tbl1
.schema tbl2
.schema tbl3

INSERT INTO tbl3(id1, id2) VALUES(2, 2);

SELECT tbl1.name
    FROM tbl1
    INNER JOIN tbl3 ON tbl1.id = tbl3.id1,
               tbl2 ON tbl3.id2 = tbl2.id
    WHERE tbl2.name = 'Taxi driver';

SELECT tbl1.name, tbl2.name
    FROM tbl1, tbl2, tbl3
    WHERE tbl1.id = tbl3.id1 AND tbl2.id = tbl3.id2;

SELECT tbl2.name
    FROM tbl2
    LEFT JOIN tbl3 ON tbl2.id = tbl3.id2
    WHERE tbl3.id2 IS NULL;