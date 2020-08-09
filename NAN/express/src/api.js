const express = require('express');
const router = express.Router();
const bcrypt = require('bcrypt');
const crypto = require('crypto'); // crypto.randomBytes() for session id
const app = express();
const os = require('os');
const port = 6101;
const cookieParser = require("cookie-parser");
const sqlite3 = require('sqlite3').verbose()

const db = new sqlite3.Database('dikt.db');
app.use(cookieParser());

// Express looks up the files relative to the static directory, 
// so the name of the static directory is not part of the URL.
// therefore we also create a virtual path prefix "/static" 
app.use('/static', express.static('static'));

// enable CORS (cross origin resource sharing)
app.use((req, res, next) => {
  //res.append('Access-Control-Allow-Origin', '*');
  res.append('Access-Control-Allow-Origin', 'http://prosjekt.asuscomm.com:81');
  next();
});

app.use(express.urlencoded({ extended: true })); // parse urlencoded POST body
//app.use(bodyParser.json({
//  strict: false
//}));
app.use(express.json({
  strict: false
}));
app.use('/diktdb', router);

// HENTE ALLE DIKT
// statuscode er testet ok (200)
router.get('/dikt/', function (req, res) {
  console.log("\nGET (alle dikt)");

  db.serialize(function () {
    db.all("SELECT * FROM Dikt", function (feil, rader) {
      if (rader) {
        res.status(200).json(rader);
        return 0;
      }
    });
  });
});

// HENTE UT ETT BESTEMT DIKT (GITT DIKTID)
// aux = 1 angir at diktID ikke eksisterer
router.get('/dikt/:id', function (req, res) {
  console.log("\n/dikt executed (GET diktID " + req.params.id + ")");

  db.serialize(function () {
    db.get("SELECT * FROM Dikt WHERE diktID = $id", req.params.id, function (feil, rad) {
      if (rad) {
        console.log(rad.dikt);
        res.status(200).json({ diktid: rad.diktID, dikt: rad.dikt, epostadresse: rad.epostadresse, aux: 0 });
        return 0;
      }

      else if (rad == undefined) {
        console.log("ERROR: 404");
        res.status(404).json({ diktID: parseInt(req.params.id), dikt: "Diktet eksisterer ikke", epostadresse: "Udefinert", aux: 1 });
        return 0;
      }
    });
  });
});

// LEGGE INN DIKT
router.post('/dikt/', function (req, res) {
  console.log("\n/dikt (ADD) executed");

  if (req.cookies.restsid == undefined) {
    console.log("ERROR: No restsid COOKIE supplied by client");
    res.status(400).json({ status: "Error", message: "Missing data to fulfill request (session cookie)" });
    return 0;
  }

  else if (req.body.dikt == undefined) {
    console.log("ERROR: No req.body.dikt supplied by client");
    res.status(400).json({ status: "Error", message: "Missing data to fulfill request (req.body.dikt)" });
    return 0;
  }

  console.log("Got restsid COOKIE: " + req.cookies.restsid);
  sessionCookie = req.cookies.restsid;

  db.serialize(function () {
    db.run("PRAGMA foreign_keys = ON");

    // sjekke om sesjonen eksisterer
    db.get("SELECT epostadresse FROM Sesjon WHERE sesjonsID = $id", sessionCookie, function (feil, rad) {
      if (rad) {
        console.log("SESSION EXISTS");
        console.log("epostadresse: " + rad.epostadresse);

        db.run("INSERT INTO Dikt (dikt, epostadresse) VALUES ($dikt, $epostadresse);", req.body.dikt, rad.epostadresse, function (err) {
          console.log("INSERT executed!");

          if (err) {
            console.log("ERROR: INSERT FAILED!");
            console.log(err.message);
            res.status(500).json({ status: "Error", message: err.message });
            return 0;
          }

          else {
            console.log("SUCCESS: 201 Created (diktID: " + this.lastID + ")");
            res.status(201).json({ status: "Success", message: "Dikt created", diktID: this.lastID });
            return 0;
          }

        });

      }

      else if (rad == undefined) {
        console.log("ERROR: Session not found!");
        res.status(401).json({ status: "Error", message: "SessionID not found" });
        return 0;
      }

    });

  });

});

// ENDRE EGNE DIKT
// mange BRANCHES for å kunne gi korrekte statuskoder/feilmeldinger
router.put('/dikt/:id', function (req, res) {
  console.log("\n/dikt executed (UPDATE diktID " + req.params.id + ")");

  if (req.cookies.restsid == undefined) {
    console.log("ERROR: No restsid COOKIE supplied by client");
    res.status(400).json({ status: "Error", message: "Missing data to fulfill request (session cookie)" });
    return 0;
  }

  else if (req.body.dikt == undefined) {
    console.log("ERROR: No req.body.dikt supplied by client");
    res.status(400).json({ status: "Error", message: "Missing data to fulfill request (req.body.dikt)" });
    return 0;
  }

  console.log("Got restsid COOKIE: " + req.cookies.restsid);

  db.serialize(function () {
    db.run("PRAGMA foreign_keys = ON");

    // sjekke om sesjonen eksisterer
    db.get("SELECT epostadresse FROM Sesjon WHERE sesjonsID = $id", req.cookies.restsid, function (feil, rad) {
      if (rad) {
        console.log("SESSION EXISTS");
        console.log("epostadresse: " + rad.epostadresse);

        // sjekke om diktID eksisterer
        db.get("SELECT diktID FROM Dikt WHERE diktID = $diktid", req.params.id, function (err, row) {
          console.log("SELECT executed");

          if (err) {
            console.log("ERROR: 500");
            console.log(err.message);
            res.status(500).json({ status: "Error", message: err.message });
            return 0;
          }

          if (row == undefined) {
            console.log("ERROR: diktID " + req.params.id + " does not exist");
            res.status(404).json({ status: "Error", message: "diktID " + req.params.id + " does not exist" });
            return 0;
          }

          console.log("(diktID " + req.params.id + " exists)");

          // UPDATE dikt dersom diktID eies av bruker    
          db.run("UPDATE Dikt SET dikt = $diktinnhold WHERE epostadresse = $radepostadresse AND diktID = $diktID",
            req.body.dikt, rad.epostadresse, req.params.id, function (err) {

              console.log("UPDATE executed");

              if (err) {
                console.log("ERROR: 500");
                console.log(err.message);
                res.status(500).json({ status: "Error", message: err.message });
                return 0;
              }

              else if (this.changes == 0) {
                console.log("ERROR: No changes after UPDATE statement");
                console.log("(diktID " + req.params.id + " is not owned by " + rad.epostadresse + ")");
                res.status(401).json({ status: "Error", message: "diktID " + req.params.id + " is not owned by " + rad.epostadresse });
                return 0;
              }

              console.log("SUCCESS: UPDATED diktID " + req.params.id);
              res.status(200).json({ status: "Success", message: rad.epostadresse + " UPDATED diktID " + req.params.id });
              return 0;

            });

        });

      } // ends "session exists" branch 

      else if (rad == undefined) {
        console.log("ERROR: Invalid SessionID!");
        res.status(401).json({ status: "Error", message: "Invalid SessionID" });
        return 0;
      }

    });

  });

});

// SLETTE EGNE DIKT (gitt diktID)
// mange BRANCHES for å gi korrekte statuskoder/feilmeldinger
router.delete('/dikt/:id', function (req, res) {
  console.log("\n/dikt executed (DELETE diktID " + req.params.id + ")");

  if (req.cookies.restsid == undefined) {
    console.log("ERROR: No restsid COOKIE supplied by client");
    res.status(400).json({ status: "Error", message: "Missing data to fulfill request (session cookie)" });
    return 0;
  }

  console.log("Got restsid COOKIE: " + req.cookies.restsid);

  db.serialize(function () {
    db.run("PRAGMA foreign_keys = ON");

    // sjekke om sesjonen eksisterer
    db.get("SELECT epostadresse FROM Sesjon WHERE sesjonsID = $id", req.cookies.restsid, function (feil, rad) {
      if (rad) {
        console.log("SESSION EXISTS");
        console.log("epostadresse: " + rad.epostadresse);

        // sjekke om diktID eksisterer
        db.get("SELECT diktID FROM Dikt WHERE diktID = $diktid", req.params.id, function (err, row) {
          console.log("SELECT executed");

          if (err) {
            console.log("ERROR: 500");
            console.log(err.message);
            res.status(500).json({ status: "Error", message: err.message });
            return 0;
          }

          if (row == undefined) {
            console.log("ERROR: diktID " + req.params.id + " does not exist");
            res.status(404).json({ status: "Error", message: "diktID " + req.params.id + " does not exist" });
            return 0;
          }

          console.log("(diktID " + req.params.id + " exists)");

          // DELETE diktID dersom diktID eies av bruker    
          db.run("DELETE FROM Dikt WHERE epostadresse = $radepostadresse AND diktID = $diktID",
            rad.epostadresse, req.params.id, function (err) {

              console.log("DELETE executed");

              if (err) {
                console.log("ERROR: 500");
                console.log(err.message);
                res.status(500).json({ status: "Error", message: err.message });
                return 0;
              }

              else if (this.changes == 0) {
                console.log("ERROR: No changes after DELETE statement");
                console.log("(diktID " + req.params.id + " is not owned by " + rad.epostadresse + ")");
                res.status(401).json({ status: "Error", message: "diktID " + req.params.id + " is not owned by " + rad.epostadresse });
                return 0;
              }

              console.log("SUCCESS: DELETED diktID " + req.params.id);
              res.status(200).json({ status: "Success", message: rad.epostadresse + " DELETED diktID " + req.params.id });
              return 0;

            });

        });

      } // ends "session exists" branch 

      else if (rad == undefined) {
        console.log("ERROR: Invalid SessionID!");
        res.status(401).json({ status: "Error", message: "Invalid SessionID" });
        return 0;
      }

    });

  });

});

// SLETTE ALLE EGNE DIKT
router.delete('/dikt/', function (req, res) {
  console.log("\n/dikt executed (DELETE ALLE DIKT)");

  if (req.cookies.restsid == undefined) {
    console.log("ERROR: No restsid COOKIE supplied by client");
    res.status(400).json({ status: "Error", message: "Missing data to fulfill request (session cookie)" });
    return 0;
  }

  console.log("Got restsid COOKIE: " + req.cookies.restsid);

  db.serialize(function () {
    db.run("PRAGMA foreign_keys = ON");

    // sjekke om sesjonen eksisterer
    db.get("SELECT epostadresse FROM Sesjon WHERE sesjonsID = $id", req.cookies.restsid, function (feil, rad) {

      // sesjon eksisterer
      if (rad) {
        console.log("SESSION EXISTS");
        console.log("epostadresse: " + rad.epostadresse);

        // slette alle diktID tilhørende "epostadresse"
        db.run("DELETE FROM Dikt WHERE epostadresse = $epostadresse", rad.epostadresse, function (err) {
          console.log("DELETE Executed");

          if (err) {
            console.log("ERROR: " + err.message);
            res.status(500).json({ status: "Error", message: err.message });
            return 0;
          }

          if (this.changes == "0") {
            console.log("ERROR: NOTHING TO DELETE (user has no associated diktID's)");
            res.status(404).json({ status: "Error", message: "Nothing to DELETE (user has no associated diktID's)" });
            return 0;
          }

          // DELETE resulterte i endringer
          console.log("DELETE SUCCESS: 200 OK " + "(deleted " + this.changes + " dikt)");
          res.status(200).json({ status: "Success", message: "Deleted " + this.changes + " dikt" });
          return 0;
        });

      } // ends "session exists" branch 

      else if (rad == undefined) {
        console.log("ERROR: Invalid SessionID!");
        res.status(401).json({ status: "Error", message: "Invalid SessionID" });
        return 0;
      }

    });

  });

});

// login
router.post('/session/', function (req, res) {
  console.log("\n/session (login) executed");
  console.log("user: " + req.body.epostadresse);
  console.log("pass: " + req.body.passord);

  db.serialize(function () {
    db.run("PRAGMA foreign_keys = ON");

    // retrieve passordhash of provided epostadresse
    db.get("SELECT passordhash FROM Bruker WHERE epostadresse = $epostadresse;", req.body.epostadresse, function (err, row) {
      console.log("SELECT passordhash");

      if (err) {
        console.log("ERROR: " + err.message);
        res.status(500).json({ status: "Error", message: err.message });
        return 0;
      }

      // epostadresse not in database
      else if (row == undefined) {
        console.log("ERROR: could not find user " + req.body.epostadresse);
        res.status(401).json({ status: "Error", message: "Unauthorized" });
        return 0;
      }

      // epostadresse exists
      else {
        var sessionID;
        bcrypt.compare(req.body.passord, row.passordhash, function (err, isMatch) {
          console.log("bcrypt.compare(\"" + req.body.passord + "\", row.passordhash) ..");
          if (err) throw err;

          // correct password
          else if (isMatch) {
            console.log("PASSWORD MATCH");

            // session id: pseudorandom bytes [hex encoded]
            // randomBytes introduces entropy and makes it tricky to guess session-id.

            crypto.randomBytes(20, function (err, buf) {
              if (err) throw err;
              sessionID = buf.toString('hex');
              console.log("generate sesjonsID: " + sessionID);

              // INSERT sesjonsID
              db.serialize(function () {

                db.run("PRAGMA foreign_keys = ON");
                db.run("INSERT INTO Sesjon (sesjonsID, epostadresse) VALUES ($sesjonsID, $epostadresse);",
                  sessionID, req.body.epostadresse, function (err) {

                    console.log("INSERT sesjonsID ..");
                    if (err) {
                      console.log("ERROR: " + err.message);
                      res.status(500).json({ status: "Error", message: err.message });
                      return 0;
                    }
                    else {
                      res.cookie('restsid', sessionID).json({ "restsid": sessionID });
                      console.log("sesjonsID INSERTED");
                      return 0;
                    }

                  });

              });

              console.log("LOGIN SUCCESS");
              return 0;
            });

          }

          // wrong password
          else {
            console.log("PASSWORD NOT MATCH");
            res.status(401).json({ status: "Error", message: "Incorrect user:pass combination" });
            return 0;
          }

        });

      }

    });

  });

});

// SESSION INFO (logged in? logged out? epostadresse? navn?)
router.get('/session/:restsid', function (req, res) {
  console.log("\n/session (info) executed");
  var sessionCookie = {};

  console.log("Got restsid in PATH: " + req.params.restsid);
  sessionCookie = req.params.restsid;

  console.log("Somehow got sessionID cookie: " + sessionCookie);
  console.log("Searching database for session: " + sessionCookie);

  db.serialize(function () {
    // sjekke om sesjonen eksisterer
    db.get("SELECT epostadresse FROM Sesjon WHERE sesjonsID = $id", sessionCookie, function (feil, rad) {
      if (rad) {
        // sesjonen eksisterer. hent brukerens navn fra databasen.
        db.get("SELECT epostadresse,fornavn,etternavn FROM Bruker WHERE epostadresse = $id", rad.epostadresse, function (feil, row) {
          if (row) {
            console.log("SESSION FOUND")
            console.log(row);
            res.status(200).json({ status: "Logged in", epostadresse: row.epostadresse, name: `${row.fornavn} ${row.etternavn}`, aux: "" });
            return 0;
          }

          else if (row == undefined) {
            console.log("ERROR: finner ikke Bruker tilknyttet epostadresse " + rad.epostadresse);
            res.status(404).json({ status: "Logged out", epostadresse: "", name: "", aux: "Bruker ikke funnet" });
            return 0;
          }
        });

      }

      // STATUSCODE TESTET OK
      else if (rad == undefined) {
        console.log("ERROR: Session not found!");
        res.status(404).json({ status: "Logged out", epostadresse: "", name: "", aux: "SessionID not found in database" });
        return 0;
      }
    });
  });
});

// logout
router.delete('/session/:restsid', function (req, res) {
  console.log("\n/session (logout) executed");
  console.log("Got restsid in PATH: " + req.params.restsid);
  var sessionCookie = req.params.restsid;

  console.log("Got restsid in COOKIE (ignoring): " + req.cookies.restsid);
  console.log("Searching database for session: " + sessionCookie);

  db.serialize(function () {
    // sjekke om sesjonen eksisterer
    db.get("SELECT * FROM Sesjon WHERE sesjonsID = $id", sessionCookie, function (feil, rad) {
      if (rad) {
        console.log("SESSION FOUND")
        console.log("sesjonsID: " + rad.sesjonsID);
        console.log("epostadresse: " + rad.epostadresse);

        db.run("DELETE FROM Sesjon WHERE sesjonsID = $sid;", sessionCookie, function (err) {
          console.log("DELETE executed");
          if (err) {
            console.log("ERROR: 500");
            console.log(`${err}`);
            res.status(500).json({ status: "Error", epostadresse: "", name: "", aux: `${err}` });
            return 0;
          }

          console.log("SUCCESS: 200 OK [DELETE made " + this.changes + " changes]");
          res.status(200).clearCookie("restsid").json({ status: "Logged out", epostadresse: "", name: "", aux: "Successfully logged out (SessionID deleted)" });
          return 0;
        });

      }

      else if (rad == undefined) {
        console.log("ERROR: Session not found!");
        res.status(404).json({ status: "Logged out", epostadresse: "", name: "", aux: "SessionID not found in database" });
        return 0;
      }

    });

  });

});

app.listen(port, function () {
  console.log(`express.js http server listening on port ${port} [` + os.platform() + " on " + os.arch() + "]");
});