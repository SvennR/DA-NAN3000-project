DROP TABLE IF EXISTS Bruker;
DROP TABLE IF EXISTS Sesjon;
DROP TABLE IF EXISTS Dikt;

CREATE TABLE Bruker(
    epostadresse VARCHAR(50) NOT NULL,
    passordhash   VARCHAR(60),
    fornavn       VARCHAR(30),
    etternavn     VARCHAR(30),

    PRIMARY KEY (epostadresse)
);

CREATE TABLE Sesjon(
    sesjonsID    VARCHAR(40) NOT NULL,
    epostadresse VARCHAR(50),

    PRIMARY KEY (sesjonsID),
    FOREIGN KEY (epostadresse) REFERENCES Bruker (epostadresse)
);

CREATE TABLE Dikt(
    diktID        INTEGER NOT NULL,
    dikt          VARCHAR(1000),
    epostadresse VARCHAR(50),

    PRIMARY KEY (diktID),
    FOREIGN KEY (epostadresse) REFERENCES Bruker (epostadresse)
);