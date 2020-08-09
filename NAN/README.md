# DA-NAN3000
## Setup
### Terminal 1 (sette opp server/CGI/unshare-container)
```
$ git clone https://github.com/g-nord/NAN.git
$ cd NAN
$ ./everything.sh

(you are now in unshare container)
# server
```
### Terminal 2 (bygge/starte express docker container)
```
$ ./run-express.sh 

(express api is now running)
```
Dette kan ta litt tid dersom det mangler forhåndskompilerte pakker for den aktuelle plattformen (hvilket er tilfellet på ARM, der en del må kompileres). Ved senere kjøring er filene cachet og en slipper kompileringen.

#### Exit express server
For å exite express server, må man i et ny terminal session, kjøre scriptet
```
$ ./clean-express.sh
```
eller kjøre 
```
$ sudo docker stop express
```

### DiktDB Login
svennrage@gmail.com             "HASH1" \
glen.nordskog@gmail.com         "HASH2" \
espen.lien.pedersen@gmail.com   "HASH3" 

### Sqlite database
Ferdigkonstruert database ligger i express/src/dikt.db (_med innhold_)

### Test-Container
Den avslutter docker daemon og alle containere. 
Den starter docker daemon med user namespaces (opprettes en dockremap bruker og gruppe)
Deretter sletter den alle express containere og images. Bygger image på nytt igjen.
Så kjører den en container med følgende opsjoner:
- -it # --iterative + --tty
- --name express \ # Setter navnet til express
- --cap-drop=all \ # Dropper alle capabilities
- --pids-limit 200 \ # Setter tak på 200 prosesser (fork-bomb)
- --cpuset-cpus 0 \ # Spesifiserer å kjøre på cpu 0
- --cpu-shares 512 \ # Prioriterer halvparten av CPU resursene til CP
- --memory 512m \ # Setter maks minne til 512 MB
- --publish 6101:6101 \ # Binder port 6101 på host til 6101 på containeren
- express-image # Navnet på image som containeren skal kjøre
```
$ ./start-express.sh
```
## Kjøring av server
### Setup og kjøring av unshare container
```
$ ./setup-container.sh
$ ./run-container.sh

(you are now in container)

 # server
```

server kjører nå med begrenset tilgang til proc og filsystemet for øvrig.
 
### Rydde opp etter container
```(fortsatt i container)
/ # exit.sh
(dreper server, umount proc)

/ # exit
(ute av container)

(slette container)
$ ./clean-container.sh
```

### everything.sh (make server & clean container & setup container & run container)
```
$ ./everything.sh

(you are now in container)

# server
```

### Åpne en terminal der man følger med på loggfilen
```console
$ ./check-log.sh
```
Filinnholdet dukker fortløpende opp på terminalen.

### Kjøre server som root

Server kjøres nå via unshare container (se over).

LOCAL_PORT er port 81

### Utfør GET spørringer
#### Nettleser
http://prosjekt.asuscomm.com:81/mp2.3/name.html \
http://prosjekt.asuscomm.com:81/mp2.3/name.xsl \
http://prosjekt.asuscomm.com:81/mp2.3/name.xsd \
http://prosjekt.asuscomm.com:81/mp2.3/namehtmlstyle.css \
http://prosjekt.asuscomm.com:81/mp2.3/namexmlstyle.css 

http://prosjekt.asuscomm.com:81/prosjektrapport.txt (mime, exists, readable) TXT\
http://prosjekt.asuscomm.com:81/index.html (mime, exists, readable) HTML\
http://prosjekt.asuscomm.com:81/bok0.xml (mime, exists, readable) XML\
http://prosjekt.asuscomm.com:81/prosjektrapport.pdf (!mime, exists, readable) \
http://prosjekt.asuscomm.com:81/file_doesnt_exist.txt (mime, !exists) \
http://prosjekt.asuscomm.com:81/file_doesnt_exist.pdf (!mime, !exists) \
http://prosjekt.asuscomm.com:81/cant_touch_this.txt (mime, exists, !readable) eies av root (rw-------)\
http://prosjekt.asuscomm.com:81/cant_touch_this.pdf (!mime, exists, !readable) eies av root (rw-------)\
http://prosjekt.asuscomm.com:81/tux.png (mime, exists, readable, IMAGE) \
http://prosjekt.asuscomm.com:81/images/penguin.png (mime, exists, readable, IMAGE, subdir) \
http://prosjekt.asuscomm.com:81/file.svg (mime, exists, readable, IMAGE) \
http://prosjekt.asuscomm.com:81/ (webroot)

### CGI Scripts
http://prosjekt.asuscomm.com:81/cgi-bin/name.cgi \
http://prosjekt.asuscomm.com:81/mp2.3/name.html \
http://prosjekt.asuscomm.com:81/mp2.3/name.xsl \
http://prosjekt.asuscomm.com:81/mp2.3/name.xsd \
http://prosjekt.asuscomm.com:81/mp2.3/namehtmlstyle.css \
http://prosjekt.asuscomm.com:81/mp2.3/namexmlstyle.css 

#### Miljøvariabler
````
# Skriver ut 'http-body'
env | sort
````
http://prosjekt.asuscomm.com:81/cgi-bin/miljo.cgi (Prints http-body - env variables) \
http://prosjekt.asuscomm.com:81/cgi-bin/miljo.cgi?hello+world (^ plus QUERY_STRING)

http://prosjekt.asuscomm.com:81/cgi-bin/mycgi?hi+you (C program)
#### URL-input
Demonstrasjon av hvordan deler av en URL brukes som input til CGI-program, via miljøvariablelen `QUERY_STRING`.\
http://prosjekt.asuscomm.com:81/hallo_1.html \
http://prosjekt.asuscomm.com:81/cgi-bin/hallo_1.cgi 

#### HTML skjema
Demonstrasjon av hvordan et HTML-skjema brukes som input til CGI-program, via miljøvariablelen `QUERY_STRING`.\
http://prosjekt.asuscomm.com:81/hallo_2.html \
http://prosjekt.asuscomm.com:81/cgi-bin/hallo_2.cgi 

I dette eksemplet brukes også et HTML-skjema som input til et CGI-program. CGI-programmet dekoder `QUERY_STRING`. Den skriver dessuten html-kode, i motsetning til de foregående eksemplene som skrev ut ren tekst. \
http://prosjekt.asuscomm.com:81/hallo_3.html \
http://prosjekt.asuscomm.com:81/cgi-bin/hallo_3.cgi

I denne varianten skriver CGI-programmet ut skjemaet selv. \
http://prosjekt.asuscomm.com:81/cgi-bin/hallo_4.cgi 

Her brukes `POST` istedet for `GET`, og attributtet `ACTION` er utelatt. \
http://prosjekt.asuscomm.com:81/cgi-bin/hallo_5.cgi (GET & POST) (WORKS!)


### Update files in container (without killing server)
```
...
(update/create files in www/ or www/cgi-bin/)

(outside container)
$ ./update-container-files

(files inside container are now updated!)
```


## www/           (webroot)
Katalogen www/ skal være webroot. Det er denne katalogen som blir kopiert til ./unshare-container/var/www.
