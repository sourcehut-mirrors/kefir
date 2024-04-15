DROP TABLE IF EXISTS tbl3;
DROP TABLE IF EXISTS tbl2;
DROP TABLE IF EXISTS tbl1;

CREATE TABLE tbl1 (
    id INTEGER PRIMARY KEY,
    name VARCHAR(36)
);

CREATE TABLE tbl2 (
    id INTEGER PRIMARY KEY,
    name VARCHAR(36)
);

CREATE TABLE tbl3 (
    id1 INTEGER,
    id2 INTEGER,
    FOREIGN KEY (id1) REFERENCES tbl1(id),
    FOREIGN KEY (id2) REFERENCES tbl2(id)
);

INSERT INTO tbl1(id, name) VALUES (0, 'Ivan');
INSERT INTO tbl1(id, name) VALUES (1, 'Vasily');
INSERT INTO tbl1(id, name) VALUES (2, 'Alexey');

INSERT INTO tbl2(id, name) VALUES (0, 'Cook');
INSERT INTO tbl2(id, name) VALUES (1, 'Mechanic');
INSERT INTO tbl2(id, name) VALUES (2, 'Taxi driver');

INSERT INTO tbl3(id1, id2) VALUES(0, 2);
INSERT INTO tbl3(id1, id2) VALUES(1, 0);
