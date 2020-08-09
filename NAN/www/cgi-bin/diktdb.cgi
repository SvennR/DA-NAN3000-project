#!/bin/sh
########################################
#   EXTRACT VARIABLES FROM POST BODY   #
########################################
BODY=$(timeout 3 head -c $CONTENT_LENGTH)

oifs=$IFS; IFS="&="; prev=""; key="1";
# most of these variables are actions which get triggered by pressing a HTML <button> or similar
# or parameters associated with triggering buttons
LOGIN="0"; LOGOUT="0"; SESSIONINFO="0"; OPPRETT="0"; LEGGTILDIKT="0"; CANCELDIKT="0"; GETDIKT="0";
REDIGERDIKT="0"; HVILKETDIKT="0"; DIKTNR="0"; SLETTDIKT="0"; SLETTALLEDIKT="0";
for var in ${BODY}
do
    if [[ $key = "1" ]]; then    
        if [[ "$var" == "epostadresse" ]]; then
            LOGIN="1"
            prev="epostadresse"
        elif [[ "$var" == "passord"  ]]; then
            prev="passord"
        elif [[ "$var" == "logout" ]]; then
            LOGOUT="1"
        elif [[ "$var" == "opprett" ]]; then
            OPPRETT="1"
        elif [[ "$var" == "nyttdikt" ]]; then
            prev="nyttdikt"
            NYTTDIKT="1"
        elif [[ "$var" == "reset" ]]; then
            CANCELDIKT="1"
        elif [[ "$var" == "getdikt" ]]; then
            GETDIKT="1"
            prev="getdikt"
        elif [[ "$var" == "redigerdikt" ]]; then
            prev="redigerdikt"
            REDIGERDIKT="1"
        elif [[ "$var" == "mainmenu" ]]; then
            prev="mainmenu"
        elif [[ "$var" == "hvilketdikt" ]]; then
            prev="hvilketdikt"
        elif [[ "$var" == "slettdikt" ]]; then
            prev="slettdikt"
            SLETTDIKT="1"
        elif [[ "$var" == "slettalledikt" ]]; then
            SLETTALLEDIKT="1"
        fi
        key="0"
    elif [[ "$key" == "0" ]]; then
        if [[ "$prev" == "epostadresse" ]]; then
            HTTP_EPOSTADRESSE="$var"
        elif [[ "$prev" == "passord" ]]; then
            HTTP_PASSORD="$var"
        elif [[ "$prev" == "getdikt" ]]; then
            DIKTNR="$var"
        elif [[ "$prev" == "nyttdikt" ]]; then
            NYTTDIKTINNHOLD="$var"
        elif [[ "$prev" == "redigerdikt" ]]; then
            REDIGERTDIKT="$var"
        elif [[ "$prev" == "mainmenu" ]]; then
            mainmenu=1
        elif [[ "$prev" == "hvilketdikt" ]]; then
            HVILKETDIKT="$var"
        elif [[ "$prev" == "slettdikt" ]]; then
            HVILKETDIKT="$var"
        fi
        prev=""
        key="1"
    fi
done
IFS=$oifs

######################################################
#                 HTTP HEADER                        #
######################################################
# IF LOGGING IN:                                     #
#   convert form-urlencoded post body to JSON        #
#   send JSON login object to express server         #
#   split response into LOGIN_HEADER and LOGIN_BODY  #
#   extract LOGIN_JSON from LOGIN_BODY               #
#   bounce "Set-Cookie:" header back to browser      #
#           so browser can set session cookie        #
######################################################
if [[ "$LOGIN" == "1" ]]; then
    HTTP_EPOSTADRESSE=$(echo "${HTTP_EPOSTADRESSE}" | sed 's/%40/@/')
    HTTP_JSON="{\"epostadresse\":\"$HTTP_EPOSTADRESSE\",\"passord\":\"$HTTP_PASSORD\"}"
    
    RAW_LOGIN=$(curl --silent --request POST 'http://127.0.0.1:6101/diktdb/session/' \
                     -D - --header 'Content-Type: application/json' \
                     --header 'Connection: close' --data-raw "$HTTP_JSON")
    
    LOGIN_HEADER=$(echo "$RAW_LOGIN" | dos2unix | sed '/^$/q')
    LOGIN_BODY=$(echo "$RAW_LOGIN" | dos2unix | sed '1,/^$/d')
    LOGIN_JSON=$(echo "$LOGIN_BODY" | sed -n '/{/,/}/p')

    # bounce "Set-Cookie:" line back to browser  
    echo "$LOGIN_HEADER" | grep "Set-Cookie:"

# Logging out? Have to DELETE cookie in browser
elif [[ "$LOGOUT" == 1 ]]; then
# Need to bounce the "Set-Cookie:" header back to browser so it can delete session cookie
# eg. echo 'Set-Cookie: restsid=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT'
    COOKIE_VALUE=$(echo "$HTTP_COOKIE" | cut -d\= -f2 )
    
    RAW_LOGOUT=$(curl   --silent --request DELETE "http://127.0.0.1:6101/diktdb/session/$COOKIE_VALUE" \
                        -D - --header 'Connection: close' \
                        --header "Cookie: $HTTP_COOKIE")
    
    LOGOUT_HEADER=$(echo "$RAW_LOGOUT" | dos2unix | sed '/^$/q' )
    LOGOUT_BODY=$(echo "$RAW_LOGOUT" | dos2unix | sed '1,/^$/d' )
    LOGOUT_JSON=$(echo "$LOGOUT_BODY" | sed -n '/{/,/}/p')

    # bounce it back to browser
    echo "$LOGOUT_HEADER" | grep "Set-Cookie:"
fi
##################
# STATIC HEADERS #
##################
echo "Content-type: text/html"
echo "Connection: close"
echo


########################################################
#                      HTTP BODY                       #
########################################################
cat << EOF
<!DOCTYPE HTML>
<html>
<head>
    <title>DiktDB</title>
    <style>
    body{
        background-image: url(https://images.ctfassets.net/3s5io6mnxfqz/3ys8X1VxIfHJWgUY0je6fx/bc3ffc9c76e342fe8d5e14f723739e32/AdobeStock_138628179.jpeg);
    }
    #hentdiktknapp,#slettdiktknapp {
        width: 5.5em;
    }
    #diktliste,#slettdiktliste {
        width: 3em;
    }
    #slettegnedikt,#opprettdikt,#logout {
        width: 10em;
    }
    </style>
</head>
<body>
    <h1>DiktDB</h1>
EOF
########################################################
#                   SESSION INFO                       #
########################################################
# Is user currently logged in?                         #
# grab session STATUS from /session/:SessionID         #
# SessionID is "value" part of cookie (restsid=value)  #
########################################################
if [[ "$HTTP_COOKIE" == "" ]]; then
    if [[ "$LOGIN" == "1" ]]; then
        COOKIE_VALUE=$(echo "$LOGIN_JSON" | jq -r '.restsid' )
    else
        COOKIE_VALUE="NON-EXISTANT-SESSIONID"
    fi
else
    COOKIE_VALUE=$(echo "$HTTP_COOKIE" | cut -d\= -f2 )     
fi

STATUS_JSON=$(curl  --silent --request GET "http://127.0.0.1:6101/diktdb/session/$COOKIE_VALUE" \
                    --header 'Connection: close' \
                    --header "Cookie: $HTTP_COOKIE" )

STATUS=$(echo "$STATUS_JSON" | jq -r '.status')
FULLNAME=$(echo "$STATUS_JSON" | jq -r '.name')
EMAIL=$(echo "$STATUS_JSON" | jq -r '.epostadresse' | tr '[A-Z]' '[a-z]')

if [[ "$STATUS" == "Logged out" ]]; then
echo "<h2>${STATUS}</h2>"
##############
# LOGIN form #
##############
cat << EOF
    <form method='post' action='http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi'>
        <input type=text name='epostadresse' placeholder='user@email.address'><br>
        <input type=password name='passord' placeholder='password'>
        <input type=submit value='Log in'>
    </form>
    <br>
EOF
fi
########################
#    IF LOGGED IN      #
########################
if [[ "$STATUS" == "Logged in" ]]; then
echo "<h2>${FULLNAME} (${EMAIL})</h2>"
    ######################
    # "Log out" <button> #
    ######################

    echo '<form method="post" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">'
    echo '   <button id="logout" type="submit" name="logout" value="1">Log out</button>'
    echo '</form>'

    ###########################
    # "Opprett dikt" <button> #
    ###########################
    if [[ "$OPPRETT" == "0" ]]; then
        echo '<form method="post" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">'
        echo '   <button id="opprettdikt" type="submit" name="opprett" value="1">Opprett dikt</button>'
        echo '</form><br>'
    fi

    ###############################
    # "SLETT EGNE" DIKT <button>  #
    ###############################
        echo '<form method="post" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">'
        echo '   <button id="slettegnedikt" type="submit" name="slettalledikt" value="1">SLETT EGNE</button>'
        echo '</form>'     

    echo "<br>"
fi

##################################
# "SLETT EGNE" button TRIGGERED  #
##################################
if [[ "$SLETTALLEDIKT" == "1" ]]; then
    SLETTALLEDIKT_RES=$(curl --silent \
                             --request DELETE "http://127.0.0.1:6101/diktdb/dikt/" \
                             --header "Cookie: $HTTP_COOKIE" \
                             --header 'Connection: close')

    #################################
    # Display Success/error message #
    #################################
    echo "$SLETTALLEDIKT_RES" | jq -r '.message'
    echo "<br>"
fi

#####################################
#  "SLETT DIKT" button TRIGGERED    #
#####################################
if [[ "$SLETTDIKT" == "1" ]]; then
    SLETTDIKT_RES=$(curl --silent \
                         --request DELETE "http://127.0.0.1:6101/diktdb/dikt/${HVILKETDIKT}" \
                         --header "Cookie: $HTTP_COOKIE" \
                         --header 'Connection: close')
    
    #################################
    # Display Success/error message #
    #################################
    echo "$SLETTDIKT_RES" | jq -r '.message'
    echo "<br>"
fi

########################################
##  "Opprett dikt" button TRIGGERED   ##
########################################
if [[ "$NYTTDIKT" == "1" ]]; then
# dikt contents are POSTed to BODY
# then extracted and dumped in NYTTDIKTINNHOLD
# contents are form-urlencoded
# Currently using 'sed' to decode (needs more work!)
    NYTTDIKTINNHOLD=$(echo "$NYTTDIKTINNHOLD" \
                            | sed 's/+/ /g' \
                            | sed 's/%0D%0A/\&#013;\&#010;/g' \
                            | sed 's/%F8/\&oslash;/g' \
                            | sed 's/%21/!/g' \
                            | sed 's/%E6/\&aelig;/g' \
                            | sed 's/%E5/\&aring;/g' | sed 's/%2C/\,/g' \
                            | sed 's/%3F/?/g' \
                            | sed 's/%27/\&#39;/g')
                                    
    DIKTJSON="{\"dikt\":\"$NYTTDIKTINNHOLD\"}"

    NYTTDIKT_RES=$(curl  --silent \
                         --request POST "http://127.0.0.1:6101/diktdb/dikt/" \
                         --header "Cookie: $HTTP_COOKIE" \
                         --header 'Content-Type: application/json' \
                         --header 'Connection: close' \
                         --data-raw "$DIKTJSON" )
    
    #####################################
    # Display Success/Error message     #
    #         after "OK" Opprett dikt   #
    #####################################
    echo "$NYTTDIKT_RES" | jq -r '.message'
    echo " (diktID "; echo "$NYTTDIKT_RES" | jq -r '.diktID' ; echo ")<br>"
fi

##################################
#     "Save" button TRIGGERED    #
##################################
if [[ "$REDIGERDIKT" == "1" ]]; then
    
    REDIGERTDIKT=$(echo "$REDIGERTDIKT" \
                        | sed 's/+/ /g' \
                        | sed 's/%0D%0A/\&#013;\&#010;/g' \
                        | sed 's/%F8/\&oslash;/g' \
                        | sed 's/%21/!/g' \
                        | sed 's/%E6/\&aelig;/g' \
                        | sed 's/%E5/\&aring;/g' | sed 's/%2C/\,/g' \
                        | sed 's/%3F/?/g' \
                        | sed 's/%27/\&#39;/g')
    
    DIKTJSON="{\"dikt\":\"${REDIGERTDIKT}\"}"

    REDIGERT_RES=$(curl  --silent \
                         --request PUT "http://127.0.0.1:6101/diktdb/dikt/${HVILKETDIKT}" \
                         --header "Cookie: $HTTP_COOKIE" \
                         --header 'Content-Type: application/json' \
                         --header 'Connection: close' \
                         --data-raw "$DIKTJSON" )                    

    #################################
    # Display RESPONSE from EXPRESS #
    #################################
    echo "$REDIGERT_RES" | jq -r '.message'
    echo "<br>"
fi


############################################
# Display SUCCESS or ERROR message         #
#         after LOGIN (attempt/completion) #
############################################
if [[ "$LOGIN" == "1" ]]; then
    ERROR_STATUS=$(echo "$LOGIN_BODY" | sed -n '/{/,/}/p' | jq -r '.message' )
    if [[ "$ERROR_STATUS" == "Unauthorized" ]]; then
        echo "Invalid LOGIN credentials." 
    else
        echo "Successfully logged in."
    fi
    echo "<br><br>"    
fi
######################################
# LOGOUT message from express server #
######################################
if [[ "$LOGOUT" == "1" ]]; then
    echo "$LOGOUT_BODY" | sed -n '/{/,/}/p' | jq -r '.aux'
    echo "<br><br>"    
fi

##########################
# BACK / CANCEL messages #
##########################
if [[ "$CANCELDIKT" == "1" ]]; then
    echo "Opprett dikt canceled<br>"
fi

if [[ "$mainmenu" == "1" ]]; then
    echo "MAIN MENU<br>"
fi

#####################################
# Display "OPPRETT dikt" <textarea> #
#####################################
if [[ "$OPPRETT" == "1" ]]; then
    echo "OPPRETT Dikt<br>"
cat << EOF
    <form method="POST" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">
        <textarea autofocus="autofocus" name="nyttdikt" cols=45 rows=5></textarea><br>
        <input type="submit" value="OK">
        <button type="reset" name="reset" value="1">Clear</button>
    </form>
EOF
    ###################
    # CANCEL <button> #
    ###################
    cat << EOF
    <form method="post" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">
        <button type="submit" name="reset" value="0">Cancel</button>
    </form>
EOF

fi

####################################
##  "Hent dikt" button TRIGGERED  ##
####################################
if [[ "$GETDIKT" == "1" ]]; then
    RAW_HENTETTDIKT=$(curl  --silent --header 'Connection: close' \
                            --request GET "http://127.0.0.1:6101/diktdb/dikt/${DIKTNR}")
    
    HENTETTDIKT_DIKT=$(echo "$RAW_HENTETTDIKT" | jq -r '.dikt')
    HENTETTDIKT_EPOST=$(echo "$RAW_HENTETTDIKT" | jq -r '.epostadresse' | tr '[A-Z]' '[a-z]')
    
    echo "Dikt ${DIKTNR} (${HENTETTDIKT_EPOST})<br>"

cat << EOF
    <form method="POST" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">
        
EOF
    echo "<textarea name='redigerdikt' cols=45 rows=5>"
    echo "$HENTETTDIKT_DIKT"
    echo "</textarea><br>"
    #################################
    # If current user "owns" dikt   #
    #   Display "Save" button       #
    #################################
    if [[ "$EMAIL" == "$HENTETTDIKT_EPOST" ]]; then
            echo "<input type=\"hidden\" id=\"hvilketdikt\" name=\"hvilketdikt\" value=\"${DIKTNR}\" readonly>"
            echo '<input type="submit" value="Save">'
    fi
    echo "</form>"

    ###################
    # CANCEL <button> #
    ###################
cat << EOF
    <form method="post" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">
        <button type="submit" name="mainmenu" value="0">Back</button>
    </form>
EOF

fi

###################################
#  SE EN LISTE OVER ALLE DIKTENE  #
###################################
echo "<br>"
DIKTARRAY=$(curl    --silent --request GET 'http://127.0.0.1:6101/diktdb/dikt/' \
                    --header "Connection: close")

# space seperated list of diktID
DIKTLISTE=$(echo "$DIKTARRAY" | jq -r '.[].diktID' | tr '\n' ' ')

echo '<form method="POST" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">'
echo '<select name="getdikt" id="diktliste">'
INDEX="0"
for DIKTID in ${DIKTLISTE}
do
    DIKTINNHOLD=$(echo "$DIKTARRAY" | jq -r ".[$INDEX].dikt")
    EPOSTADRESSE=$(echo "$DIKTARRAY" | jq -r ".[$INDEX].epostadresse")
    echo "<option value="$DIKTID">$DIKTID</option>"
    INDEX=$(($INDEX+1))  
done
echo "</select>"
cat << EOF
        <input id="hentdiktknapp" type="submit" value="Hent dikt">
    </form>
EOF

#####################################
# SLETTE EGET DIKT VED VALG I LISTE #
#   BYGGER EN LISTE OVER EGNE DIKT  #
#      BARE DERSOM LOGGET INN       #
#####################################
if [[ "$STATUS" == "Logged in" ]]; then
    INDEX="0"
    DELETE_LIST=""
    for DIKTID in ${DIKTLISTE}
    do
        EPOSTADRESSE=$(echo "$DIKTARRAY" | jq -r ".[$INDEX].epostadresse" | tr '[A-Z]' '[a-z]')
        if [[ "$EMAIL" == "$EPOSTADRESSE" ]]; then
            DELETE_LIST="$DELETE_LIST $DIKTID"
        fi
        INDEX=$(($INDEX+1))  
    done
    DELETE_LIST=$(echo "$DELETE_LIST" | sed 's/ //')
    #####################################
    #   DROP-DOWN LISTE MED EGNE DIKT   #
    #####################################
    echo '<form method="POST" action="http://prosjekt.asuscomm.com:81/cgi-bin/diktdb.cgi">'
    echo '<select name="slettdikt" id="slettdiktliste">'
    for DIKTID in ${DELETE_LIST}
    do
        echo "<option value="$DIKTID">$DIKTID</option>"
    done
    echo "</select>"
cat << EOF
        <input id="slettdiktknapp" type="submit" value="Slett dikt">
    </form>
EOF
fi

cat << EOF
</body>
</html>
EOF