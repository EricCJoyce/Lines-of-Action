//////////////////////////////////////////////////////////////////////
//   A J A X (has to be in the head)

//////////////////////////////////////////////////////////////////////
//   L I N E S o f A C T I O N : Request a new game

function reqSess()
  {
    var ReqXML = new XMLHttpRequest();                              //  IE 7+, Firefox, Chrome, Opera, Safari
    var params = 'sendRequest=wannaplay';

    ReqXML.open("POST", 'obj/sess/reqsess.php', true);
    ReqXML.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    ReqXML.onreadystatechange = function()
      {
        if(ReqXML.readyState == 4 && ReqXML.status == 200)
          {
            if(ReqXML.responseText == "")                           //  Null return: unknown error
              {
                switch(currentLang)
                  {
                    case "Spanish": alert(alertStringScrub("Error"));  break;
                    case "German": alert(alertStringScrub("Fehler"));  break;
                    case "Polish": alert(alertStringScrub("B&#322;&#261;d na stronie"));  break;
                    default: alert(alertStringScrub("Error"));
                  }
              }
            else
              {
                var parse = ReqXML.responseText.split('|');
                var arr;
                var i;
                if(parse[0] == 'linesofaction' && parse[1] == 'ok')
                  {
                    parse = parse[2].split(',');                    //  Repurpose "parse".
                    for(i = 0; i < parse.length; i++)               //  Load buffer.
                      {
                        arr = new Uint8Array(1);                    //  Force byte type.
                        arr[0] = parseInt(parse[i]);
                        gameStateBuffer[i] = arr[0];
                      }

                    for(i = _A1; i < _NOTHING; i++)                 //  Set up the board according to the game state encoded in the byte array.
                      {
                        if(gameEngine.instance.exports.isBlack_client(i) == 1)
                          initPiece('B', i, pawnGeometry, true);
                        else if(gameEngine.instance.exports.isWhite_client(i) == 1)
                          initPiece('W', i, pawnGeometry, true);
                      }
                  }
                else                                                //  Error-label or garbage
                  {
                    switch(currentLang)
                      {
                        case "Spanish": alert(alertStringScrub("Error"));  break;
                        case "German": alert(alertStringScrub("Fehler"));  break;
                        case "Polish": alert(alertStringScrub("B&#322;&#261;d na stronie"));  break;
                        default: alert(alertStringScrub("Error"));
                      }
                  }
              }
          }
      };
    ReqXML.send(params);
  }

/* Called by changing the drop-down menu. */
function changeSetup()
  {
    var posSel = document.getElementById('starting-position').value;
    var ReqXML = new XMLHttpRequest();                              //  IE 7+, Firefox, Chrome, Opera, Safari
    var i;
    var params = 'sendRequest=changesetup';
    params += '&pos=' + posSel;                                     //  Specify which set up to deploy.

    if(!gameStarted)                                                //  Don't try to chage the position after play has begun.
      {
        ReqXML.open("POST", 'obj/sess/startpos.php', true);
        ReqXML.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        ReqXML.onreadystatechange = function()
          {
            if(ReqXML.readyState == 4 && ReqXML.status == 200)
              {
                if(ReqXML.responseText == "")                       //  Null return: unknown error
                  {
                    switch(currentLang)
                      {
                        case "Spanish": alert(alertStringScrub("Error"));  break;
                        case "German": alert(alertStringScrub("Fehler"));  break;
                        case "Polish": alert(alertStringScrub("B&#322;&#261;d na stronie"));  break;
                        default: alert(alertStringScrub("Error"));
                      }
                  }
                else
                  {
                    var parse = ReqXML.responseText.split('|');
                    var arr;
                    var i;
                    var newConfig;
                    var blackClock = document.getElementById('black-clock');
                    var whiteClock = document.getElementById('white-clock');

                    if(parse[0] == 'linesofaction' && parse[1] == 'ok')
                      {
                        newConfig = parse[2];                       //  Save the set-up name.
                        parse = parse[3].split(',');                //  Repurpose "parse".
                        for(i = 0; i < parse.length; i++)           //  Load buffer.
                          {
                            arr = new Uint8Array(1);                //  Force byte type.
                            arr[0] = parseInt(parse[i]);
                            gameStateBuffer[i] = arr[0];
                          }

                        if(newConfig == 'scrambledeggs')
                          {
                            variantSetup = 'scrambledeggs';
                            blackClock.classList.replace('black-loa', 'black-scrambledeggs');
                            whiteClock.classList.replace('white-loa', 'white-scrambledeggs');

                            for(i = _A1; i < _NOTHING; i++)
                              {
                                normalMaterials[i] = normalMaterials_ScrambledEggs[i];
                                selectedMaterials[i] = selectedMaterials_ScrambledEggs[i];
                                targetedMaterials[i] = targetedMaterials_ScrambledEggs[i];
                              }
                          }
                        else
                          {
                            variantSetup = 'loa';
                            blackClock.classList.replace('black-scrambledeggs', 'black-loa');
                            whiteClock.classList.replace('white-scrambledeggs', 'white-loa');

                            for(i = _A1; i < _NOTHING; i++)
                              {
                                normalMaterials[i] = normalMaterials_LOA[i];
                                selectedMaterials[i] = selectedMaterials_LOA[i];
                                targetedMaterials[i] = targetedMaterials_LOA[i];
                              }
                          }

                        normalSq();                                 //  Set all squares.

                        for(i = _A1; i < _NOTHING; i++)             //  Set up the board according to the game state encoded in the byte array.
                          {
                            if(gameEngine.instance.exports.isBlack_client(i) == 1)
                              initPiece('B', i, pawnGeometry, (newConfig != 'scrambledeggs'));
                            else if(gameEngine.instance.exports.isWhite_client(i) == 1)
                              initPiece('W', i, pawnGeometry, (newConfig != 'scrambledeggs'));
                          }
                      }
                    else                                            //  Error-label or garbage
                      {
                        switch(currentLang)
                          {
                            case "Spanish": alert(alertStringScrub("Error"));  break;
                            case "German": alert(alertStringScrub("Fehler"));  break;
                            case "Polish": alert(alertStringScrub("B&#322;&#261;d na stronie"));  break;
                            default: alert(alertStringScrub("Error"));
                          }
                      }
                  }
              }
          };

        for(i = 0; i < gamePieces.length; i++)                      //  Remove all pieces from the scene.
          scene.remove(gamePieces[i]);

        gamePieces = [];                                            //  Remove all pieces from "gamePieces" array.

        setup_mp3.play();                                           //  Play the sound.

        ReqXML.send(params);
      }
  }

//////////////////////////////////////////////////////////////////////
//   B E F O R E    U N L O A D
//  User is closing, leaving or reloading the page. Destroy the cookie!
window.onbeforeunload = function(e)
  {
/*
    var RecXML = new XMLHttpRequest();                              //  IE 7+, Firefox, Chrome, Opera, Safari
    RecXML.open("POST", 'obj/ai/killsess.php', true);
    RecXML.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    RecXML.send();
*/
  };

//////////////////////////////////////////////////////////////////////
//   S P E C I A L    C H A R A C T E R    P R O C E S S I N G
function alertStringScrub(str)
  {
    str = str.replace(/&#193;/g, '\xC1');                           //  Uppercase A-acute
    str = str.replace(/&#225;/g, '\xE1');                           //  Lowercase A-acute
    str = str.replace(/&#196;/g, '\xC4');                           //  Uppercase A-umlaut
    str = str.replace(/&#228;/g, '\xE4');                           //  Lowercase A-umlaut
    str = str.replace(/&#260;/g, '\u0104');                         //  Uppercase A-ogonek
    str = str.replace(/&#261;/g, '\u0105');                         //  Lowercase A-ogonek

    str = str.replace(/&#262;/g, '\u0106');                         //  Uppercase C-acute
    str = str.replace(/&#263;/g, '\u0107');                         //  Lowercase C-acute

    str = str.replace(/&#201;/g, '\xC9');                           //  Uppercase E-acute
    str = str.replace(/&#233;/g, '\xE9');                           //  Lowercase E-acute
    str = str.replace(/&#203;/g, '\xCB');                           //  Uppercase E-umlaut
    str = str.replace(/&#235;/g, '\xEB');                           //  Lowercase E-umlaut
    str = str.replace(/&#280;/g, '\u0118');                         //  Uppercase E-ogonek
    str = str.replace(/&#281;/g, '\u0119');                         //  Lowercase E-ogonek

    str = str.replace(/&#205;/g, '\xCD');                           //  Uppercase I-acute
    str = str.replace(/&#237;/g, '\xED');                           //  Lowercase I-acute
    str = str.replace(/&#207;/g, '\xCF');                           //  Uppercase I-umlaut
    str = str.replace(/&#239;/g, '\xEF');                           //  Lowercase I-umlaut

    str = str.replace(/&#321;/g, '\u0141');                         //  Uppercase L-stroke
    str = str.replace(/&#322;/g, '\u0142');                         //  Lowercase L-stroke

    str = str.replace(/&#323;/g, '\u0143');                         //  Uppercase N-acute
    str = str.replace(/&#324;/g, '\u0144');                         //  Lowercase N-acute
    str = str.replace(/&#209;/g, '\xD1');                           //  Uppercase N-tilde
    str = str.replace(/&#241;/g, '\xF1');                           //  Lowercase N-tilde

    str = str.replace(/&#211;/g, '\xD3');                           //  Uppercase O-acute
    str = str.replace(/&#243;/g, '\xF3');                           //  Lowercase O-acute
    str = str.replace(/&#214;/g, '\xD6');                           //  Uppercase O-umlaut
    str = str.replace(/&#246;/g, '\xF6');                           //  Lowercase O-umlaut

    str = str.replace(/&#346;/g, '\u015A');                         //  Uppercase S-acute
    str = str.replace(/&#347;/g, '\u015B');                         //  Lowercase S-acute

    str = str.replace(/&#223;/g, '\xDF');                           //  SZ ligature

    str = str.replace(/&#218;/g, '\xDA');                           //  Uppercase U-acute
    str = str.replace(/&#250;/g, '\xFA');                           //  Lowercase U-acute
    str = str.replace(/&#220;/g, '\xDC');                           //  Uppercase U-umlaut
    str = str.replace(/&#252;/g, '\xFC');                           //  Lowercase U-umlaut

    str = str.replace(/&#377;/g, '\u0179');                         //  Uppercase Z-acute
    str = str.replace(/&#378;/g, '\u017A');                         //  Lowercase Z-acute
    str = str.replace(/&#379;/g, '\u017B');                         //  Uppercase Z-dot
    str = str.replace(/&#380;/g, '\u017C');                         //  Lowercase Z-dot

    str = str.replace(/&#161;/g, '\xA1');                           //  Inverse exclamation point
    str = str.replace(/&#191;/g, '\xBF');                           //  Inverse question mark

    return str;
  }
