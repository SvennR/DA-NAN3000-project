const getPoem = document.getElementById('get-poem');
const id = document.getElementById('poem-list');
const updatelist = document.getElementById('updateList');
const getAllPoems = document.getElementById('get-all');
const getLogin = document.getElementById('login-button');
const username = document.getElementById('usrn');
const passwordField = document.getElementById('passw');
const logoutBtn = document.getElementById('logout-button');

// Determine what to display below <hr> element
// will be set by getPoemMethod() and getAllPoemsMethod()
//  one poem  (getStartPoems = "0"
//  all poems (getStartPoems = "1")
//   no poems (getStartPoems = "" )
var getStartPoems = "";

// using sendHttpRequest() to "promisify" XMLHttpRequest()
// https://github.com/academind/xhr-fetch-axios-intro/blob/xhr/xhr.js
const sendHttpRequest = (method, url, data) => {
    const promise = new Promise((resolve, reject) => {
        const xhr = new XMLHttpRequest();
        xhr.open(method, url, true);

        xhr.responseType = 'json';

        // signals appending json data
        if (data)
            xhr.setRequestHeader('Content-Type', 'application/json');

        xhr.onload = () => {
            if (xhr.status >= 400)
                reject(xhr.response);
            else
                resolve(xhr.response);
        };

        xhr.onerror = () => {
            reject('Noe gikk galt!');
        };

        xhr.send(JSON.stringify(data));

    });
    return promise;
};

// generate a list of poems for the drop-down menu
// called when loading the page, adding poems, deleting poems, "oppdater listen" button etc
const list = () => {
    sendHttpRequest('GET', '/diktdb/dikt/')
        .then(responseData => {
            let clearList = document.getElementById("poem-list");
            clearList.innerHTML = ''; // clears list

            for (var i = 0; i < responseData.length; i++) {
                let op = document.createElement('OPTION');
                op.value = responseData[i].diktID; // Option value
                op.innerHTML = responseData[i].diktID; // Option name
                document.getElementById("poem-list").appendChild(op); // appends to list
            }
        });
};


const triggerDynamicCaching = () => {
    sendHttpRequest('GET', '/diktdb/dikt/')
        .then(responseData => {

            for (var i = 0; i < responseData.length; i++) {
                sendHttpRequest('GET', '/diktdb/dikt/' + responseData[i].diktID)
                    .then(responseData => { console.log(responseData.dikt) });
            }
        });
};

const getPoemMethod = () => {
    sendHttpRequest('GET', '/diktdb/session/' + getCookie('restsid'))
        .then(responseData => {
            var sessionEmail = responseData.epostadresse;

            sendHttpRequest('GET', '/diktdb/dikt/' + id.value)
                .then(responseData => {
                    getStartPoems = "0";
                    let clear = document.getElementById('fillIn');
                    clear.innerHTML = '';

                    if (responseData.diktid != undefined) {
                        let tr = document.createElement("TR");
                        let tdId = document.createElement("TD"); tdId.className = "id";
                        let tdPoem = document.createElement("TD");
                        let tdEvent = document.createElement("TD");
                        let txtarea = document.createElement("textarea");
                        let ch = document.createElement("BUTTON"); ch.className = "chDikt";
                        let del = document.createElement("BUTTON"); del.className = "delDikt";
                        tdId.innerHTML = responseData.diktid; // populate diktid
                        txtarea.cols = "45"; // width
                        txtarea.rows = "8" // heigth
                        txtarea.innerHTML = responseData.dikt; // populate dikt
                        ch.innerHTML = "Oppdater" // button name
                        del.innerHTML = "Slett" // button name
                        tr.appendChild(tdId);
                        tr.appendChild(tdPoem);
                        tr.appendChild(tdEvent);
                        tdPoem.appendChild(txtarea);

                        document.getElementById("fillIn").appendChild(tr);

                        // only display "Oppdater" and "Slett" 
                        // if epostadresse of "current session" matches epostadresse of poem object
                        if (responseData.epostadresse.localeCompare(sessionEmail) == 0) {
                            tdEvent.appendChild(ch);
                            tdEvent.appendChild(del);
                        }

                        // Update poem
                        const updatePoem = () => {
                            sendHttpRequest('PUT', '/diktdb/dikt/' + tdId.innerHTML, {
                                "dikt": txtarea.value
                            })

                                .then(responseData => {
                                    console.log(responseData);
                                    alert("Dikt nummer " + tdId.innerHTML + " er oppdatert")
                                })

                                .catch(err => {
                                    console.log(err);
                                    alert("Du kan ikke endre andres dikt")
                                });
                        }

                        // Delete Poem
                        const deletePoem = () => {
                            sendHttpRequest('DELETE', '/diktdb/dikt/' + tdId.innerHTML)

                                .then(responseData => {
                                    if (responseData.status == "Error")
                                        alert("Det har oppstått en feil (" + responseData.message + ")");
                                    else {
                                        tr.innerHTML = '';
                                        list();
                                        alert("Dikt nummer " + tdId.innerHTML + " er slettet");
                                    }

                                })

                                .catch(err => {
                                    console.log(err);
                                });
                        }
                        ch.addEventListener('click', updatePoem);
                        del.addEventListener('click', deletePoem);
                    }
                    else
                        alert('Id er ikke gyldig');
                })
                .catch(err => {
                    console.log(err);
                });

        })
        .catch(err => {
            console.log(err);
        });


};

// GET-request all poems in the database
const getAllPoemsMethod = () => {
    sendHttpRequest('GET', '/diktdb/session/' + getCookie('restsid'))
        .then(responseData => {
            var sessionEmail = responseData.epostadresse;
            console.log("GETTING ALL POEMS: " + sessionEmail);

            sendHttpRequest('GET', '/diktdb/dikt/')
                .then(responseData => {
                    getStartPoems = "1";
                    let clearTbody = document.getElementById('fillIn');
                    clearTbody.innerHTML = '';
                    for (let i = 0; i < responseData.length; i++) {
                        let tr = document.createElement("TR");
                        let tdId = document.createElement("TD"); tdId.class = "id";
                        let tdPoem = document.createElement("TD");
                        let tdEvent = document.createElement("TD");
                        let txtarea = document.createElement("textarea");
                        let ch = document.createElement("BUTTON"); ch.className = "ch";
                        let del = document.createElement("BUTTON"); del.className = "del";
                        tdId.innerHTML = responseData[i].diktID; // populate diktid
                        txtarea.innerHTML = responseData[i].dikt; // populate dikt
                        txtarea.cols = "45"; // width
                        ch.innerHTML = "Oppdater" // button name
                        del.innerHTML = "Slett" // button name
                        tr.appendChild(tdId);
                        tr.appendChild(tdPoem);
                        tr.appendChild(tdEvent);
                        tdPoem.appendChild(txtarea);
                        document.getElementById("fillIn").appendChild(tr);

                        // only display "Oppdater" and "Slett" 
                        // if epostadresse of "current session" matches epostadresse of poem object
                        if (responseData[i].epostadresse.localeCompare(sessionEmail) == 0) {
                            tdEvent.appendChild(ch);
                            tdEvent.appendChild(del);
                        }

                        // Update Poem
                        const updatePoem = () => {
                            sendHttpRequest('PUT', '/diktdb/dikt/' + tdId.innerHTML, {
                                "dikt": txtarea.value
                            })
                                .then(responseData => {
                                    console.log(responseData);
                                    alert("Dikt nummer " + tdId.innerHTML + " er oppdatert")
                                })
                                .catch(err => {
                                    console.log(err);
                                    alert("Du kan ikke endre andres dikt")
                                });
                        }

                        // Delete Poem
                        const deletePoem = () => {
                            console.log("get all (delete poem button)");
                            sendHttpRequest('DELETE', '/diktdb/dikt/' + tdId.innerHTML)
                                .then(responseData => {
                                    tr.innerHTML = ''; // clears the row
                                    list(); // updates the list
                                    alert("Dikt nummer " + tdId.innerHTML + " er slettet")
                                })
                                .catch(err => {
                                    console.log(err);
                                    alert("Du kan ikke slette andres dikt")
                                });
                        }
                        ch.addEventListener('click', updatePoem);
                        del.addEventListener('click', deletePoem);
                    }
                })

        }).catch(err => {
            console.log("error");

        });
}

//Log in
const login = () => {
    sendHttpRequest('POST', '/diktdb/session/', {
        "epostadresse": username.value,
        "passord": passwordField.value
    })

        .then(responseData => {

            if (responseData.status == 'Error') {
                alert('Tast inn riktig brukernavn og passord');
                return;
            }

            sendHttpRequest('GET', '/diktdb/session/' + responseData.restsid)
                .then(responseData => {

                    if (getStartPoems == "0") {
                        getPoemMethod();
                    }
                    else if (getStartPoems == "1") {
                        getAllPoemsMethod();
                    }
                    document.getElementById("loginForm").style.visibility = "hidden"; // removes loginform
                    document.getElementById("info").style.visibility = "visible"; // logout buttom visible
                    let deleteAllBtn = document.createElement('BUTTON');
                    deleteAllBtn.type = 'button'; deleteAllBtn.innerHTML = 'Slett alle egne dikt'; // delete own poem button
                    let addInfo = document.getElementById('w'); addInfo.innerHTML = 'Velkommen ' + responseData.name; // change headline
                    let addTextArea = document.createElement('TEXTAREA'); addTextArea.className = 'textId';
                    addTextArea.rows = '8'; addTextArea.cols = '30'; addTextArea.placholder = 'Skriv et nytt dikt her...'; // new poem text area
                    let addBtn = document.createElement('BUTTON'); addBtn.type = 'button'; addBtn.innerHTML = 'Legg til'; // add poem button

                    document.getElementById("addForm").appendChild(addTextArea);
                    document.getElementById("addForm").appendChild(addBtn);
                    if (document.getElementById("deleteBtnForm") != undefined)
                        document.getElementById("deleteBtnForm").appendChild(deleteAllBtn);

                    // Deletes every poems from user
                    const deleteAllMethod = () => {
                        sendHttpRequest('DELETE', '/diktdb/dikt/')
                            .then(responseData => {
                                list();
                                if (getStartPoems == "1")
                                    getAllPoemsMethod();
                                alert('Alle dine egne dikt er nå slettet');
                            })
                            .catch(err => {
                                alert('noe gikk galt. Sjekk om du er logget inn');
                            });
                    };

                    // Adding a poem
                    const addPoem = () => {
                        console.log("login version addPoem()");
                        sendHttpRequest('POST', '/diktdb/dikt/', {
                            "dikt": addTextArea.value
                        })

                            .then(responseData => {
                                console.log(responseData);
                                if (responseData.status == "Success")
                                    alert("Dikt ble lagt inn: " + addTextArea.value);

                                addTextArea.value = '';
                                list();
                                if (getStartPoems == "1")
                                    getAllPoemsMethod();
                            })

                            .catch(err => {
                                console.log(err);
                                alert("Vennligst logg inn!")
                            });
                    };

                    deleteAllBtn.addEventListener('click', deleteAllMethod);
                    addBtn.addEventListener('click', addPoem);

                });

        })
        .catch(err => {
            console.log(err);
            alert("Du har tastet feil. Tast inn riktig brukernavn og passord!")
        });
};

// Log out
const logout = () => {
    sendHttpRequest('DELETE', '/diktdb/session/' + getCookie('restsid'))
        .then(responseData => {
            console.log(responseData)
            document.getElementById("info").style.visibility = "hidden";
            document.getElementById("loginForm").style.visibility = "visible";
            document.getElementById('addForm').innerHTML = '';
            document.getElementById('fillIn').innerHTML = '';
            if (document.getElementById("deleteBtnForm") != undefined)
                document.getElementById('deleteBtnForm').innerHTML = '';
        })
        .catch(err => {
            console.log(err);
        });

};

/* Returns a cookie value
(from w3schools)
Take the cookiename as parameter (cname).
Create a variable (name) with the text to search for (cname + "=").
Decode the cookie string, to handle cookies with special characters, e.g. '$'
Split document.cookie on semicolons into an array called ca (ca = decodedCookie.split(';')).
Loop through the ca array (i = 0; i < ca.length; i++), and read out each value c = ca[i]).
If the cookie is found (c.indexOf(name) == 0), return the value of the cookie (c.substring(name.length, c.length).
If the cookie is not found, return "INVALID-JUNK-COOKIE". */

function getCookie(cname) {
    var name = cname + "=";
    var decodedCookie = decodeURIComponent(document.cookie);
    var ca = decodedCookie.split(';');
    for (var i = 0; i < ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0) == ' ') {
            c = c.substring(1);
        }
        if (c.indexOf(name) == 0) {
            return c.substring(name.length, c.length);
        }
    }
    return "INVALID-JUNK-COOKIE";
};

// Decides the layout of the website, based on "logged in" or "logged out"
const sessionLayout = () => {
    sendHttpRequest('GET', '/diktdb/session/' + getCookie('restsid'))
        .then(responseData => {
            if (responseData.status == 'Logged out') {
                document.getElementById("info").style.visibility = "hidden"; // hide logout button
                document.getElementById("loginForm").style.visibility = "visible"; // show login form
                return;
            }

            document.getElementById("loginForm").style.visibility = "hidden"; // hide login-form
            document.getElementById("info").style.visibility = "visible"; // show logout button
            let deleteAllBtn = document.createElement('BUTTON');
            deleteAllBtn.type = 'button'; deleteAllBtn.innerHTML = 'Slett alle egne dikt'; // create delete all button
            let addInfo = document.getElementById('w'); addInfo.innerHTML = 'Velkommen ' + responseData.name; // change headline
            let addTextArea = document.createElement('TEXTAREA'); addTextArea.className = 'textId';
            addTextArea.rows = '8'; addTextArea.cols = '30'; addTextArea.placholder = 'Skriv et nytt dikt her...'; // new poem field
            let addBtn = document.createElement('BUTTON'); addBtn.type = 'button'; addBtn.innerHTML = 'Legg til'; // new poem button

            document.getElementById("addForm").appendChild(addTextArea);
            document.getElementById("addForm").appendChild(addBtn);
            document.getElementById("deleteBtnForm").appendChild(deleteAllBtn);

            // Deletes all of user's poems
            const deleteAllMethod = () => {
                sendHttpRequest('DELETE', '/diktdb/dikt/')
                    .then(responseData => {
                        list();
                        if (getStartPoems == "1")
                            getAllPoemsMethod();
                        alert('Alle dine egne dikt er nå slettet');
                    })
                    .catch(err => {
                        alert('noe gikk galt. Sjekk om du er logget inn');
                    });
            };

            // Adding a poem
            const addPoem = () => {
                console.log("Session layout version addPoem()");
                var poemNum = "";
                sendHttpRequest('POST', '/diktdb/dikt/', {
                    "dikt": addTextArea.value
                })
                    .then(responseData => {
                        console.log(responseData);
                        if (responseData.status == "Success") {
                            poemNum = responseData.diktID;
                            alert("Dikt ble lagt inn: " + addTextArea.value);

                            // GET poem to trigger dynamic caching
                            sendHttpRequest('GET', '/diktdb/dikt/' + poemNum)
                                .then(responseData => {
                                    console.log(responseData.dikt);
                                }).catch(err => { console.log(err) });
                            // update drop-down list
                            // update display of all poems under <hr> element 
                            list();
                            if (getStartPoems == "1")
                                getAllPoemsMethod();
                        }
                        addTextArea.value = '';
                    })
                    .catch(err => {
                        console.log(err);
                        alert("Vennligst logg inn!")
                    });

            };

            deleteAllBtn.addEventListener('click', deleteAllMethod);
            addBtn.addEventListener('click', addPoem);
        })
        .catch(err => {
            console.log(err);
        });
};


getPoem.addEventListener('click', getPoemMethod);
getAllPoems.addEventListener('click', getAllPoemsMethod);
getLogin.addEventListener('click', login);
logoutBtn.addEventListener('click', logout);
updatelist.addEventListener('click', list);
window.addEventListener('load', sessionLayout);

list();
setTimeout(triggerDynamicCaching, 3000);