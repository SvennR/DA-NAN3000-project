/* 
    Bruker    (epostadresse, passordhash, fornavn, etternavn)
    Sesjon    (sesjonsID, epostadresse)
    Dikt      (diktID, dikt, epostadresse)
*/
PRAGMA foreign_keys = ON;

INSERT INTO Bruker VALUES (
    'svennrage@gmail.com', 
    "$2b$10$3M12qFQAZNOAivBFLhz2kO2EhuVcCIKL7urNl8XDsktfaT4SENryS", 
    'Svenn',
    'Rage'
);

INSERT INTO Bruker VALUES (
    'glen.nordskog@gmail.com',
    '$2b$10$J/BRtFhEy4aeH.4JHsYN4efoUoMZ0OYVNOvEEibKNuiaLwSsmKEve',
    'Glen',
    'Nordskog'
);
INSERT INTO Bruker VALUES (
    'espen.lien.pedersen@gmail.com',
    '$2b$10$it/0qH9UIdqArnEQn1Prv.r.fKpfBc932Xp21bQxRduraC.2U8pqS',
    'Espen Lien',
    'Pedersen'
);

INSERT INTO Dikt (dikt, epostadresse) VALUES (
    'Roses are red,&#013;&#010;Violets are blue,&#013;&#010;Sandwiches are delicious,&#013;&#010;and so are you.',
    'svennrage@gmail.com'
);

INSERT INTO Dikt (dikt, epostadresse) VALUES (
    'you fit into me&#013;&#010;like a hook into an eye&#013;&#010;&#013;&#010;a fish hook&#013;&#010;an open eye',
    'svennrage@gmail.com'
);

INSERT INTO Dikt (dikt, epostadresse) VALUES (
    'Network programming&#013;&#010;Much learning&#013;&#010;such wow!',
    'glen.nordskog@gmail.com'
);

INSERT INTO Dikt (dikt, epostadresse) VALUES (
    'He was the pin in&#013;&#010;the hand grenade of my life.&#013;&#010;Total destruction.',
    'glen.nordskog@gmail.com'
);

INSERT INTO Dikt (dikt, epostadresse) VALUES (
    'Like crunchy cornflakes&#013;&#010;Gold leaves rustle underfoot&#013;&#010;Beauty in decay.',
    'espen.lien.pedersen@gmail.com'
);

INSERT INTO Dikt (dikt, epostadresse) VALUES (
    'My life has been the poem I would have writ&#013;&#010;But I could not both live and utter it.',
    'espen.lien.pedersen@gmail.com'
);
