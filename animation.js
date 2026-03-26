const GAME_ONGOING = 0;                                             //  (See C code).
const GAME_OVER_BLACK_WINS = 1;                                     //  (See C code).
const GAME_OVER_WHITE_WINS = 2;                                     //  (See C code).
const GAME_OVER_DRAW = 3;                                           //  (See C code).

function animate()
  {
    animating = true;                                               //  Set the flag: animation is in progress.

    if(animationInstruction != null)
      {
        switch(animationInstruction.action)
          {
            case 'move':         Select_A = animationInstruction.a;
                                 Select_B = animationInstruction.b;
                                 commit_mp3.play();
                                 move(animationInstruction.a, animationInstruction.b);
                                 break;
            case 'die':          Select_A = animationInstruction.a;
                                 Select_B = animationInstruction.b;
                                 die(animationInstruction.b);
                                 break;
            case 'pass':         commit_mp3.play();
                                 switch(currentLang)
                                   {
                                     case 'Spanish': if(variantSetup == 'scrambledeggs')
                                                       {
                                                         if(gropius.team == 'Black')
                                                           alert(alertStringScrub('Chorizo debe pasar.'));
                                                         else
                                                           alert(alertStringScrub('Huevo debe pasar.'));
                                                       }
                                                     else
                                                       {
                                                         if(gropius.team == 'Black')
                                                           alert(alertStringScrub('Negro debe pasar.'));
                                                         else
                                                           alert(alertStringScrub('Blanco debe pasar.'));
                                                       }
                                                     break;
                                     case 'German':  if(variantSetup == 'scrambledeggs')
                                                       {
                                                         if(gropius.team == 'Black')
                                                           alert(alertStringScrub('Wurst muss passen.'));
                                                         else
                                                           alert(alertStringScrub('Ei muss passen.'));
                                                       }
                                                     else
                                                       {
                                                         if(gropius.team == 'Black')
                                                           alert(alertStringScrub('Das schwarze Team muss passen.'));
                                                         else
                                                           alert(alertStringScrub('Das wei\xDFe Team muss passen.'));
                                                       }
                                                     break;
                                     case 'Polish':  if(variantSetup == 'scrambledeggs')
                                                       {
                                                         if(gropius.team == 'Black')
                                                           alert(alertStringScrub('Kie&#322;basa musi zda&#263; kolej.'));
                                                         else
                                                           alert(alertStringScrub('Jajko musi zda&#263; kolej.'));
                                                       }
                                                     else
                                                       {
                                                         if(gropius.team == 'Black')
                                                           alert(alertStringScrub('Czarny zesp&#243;&#322; musi zda&#263; kolej.'));
                                                         else
                                                           alert(alertStringScrub('Bia&#322;y zesp&#243;&#322; musi zda&#263; kolej.'));
                                                       }
                                                     break;
                                     default:        if(variantSetup == 'scrambledeggs')
                                                       {
                                                         if(gropius.team == 'Black')
                                                           alert(alertStringScrub('"Sausage" passes.'));
                                                         else
                                                           alert(alertStringScrub('"Egg" passes.'));
                                                       }
                                                     else
                                                       {
                                                         if(gropius.team == 'Black')
                                                           alert(alertStringScrub('Black passes.'));
                                                         else
                                                           alert(alertStringScrub('White passes.'));
                                                       }
                                   }
                                                                    //  Update the game state.
                                 gameEngine.instance.exports.makeMove_client(a, b);
                                 swapTurns();
                                 break;
          }
      }
  }

function move(a, b)
  {
    animationTarget = 0;
    while(animationTarget < gamePieces.length && gamePieces[animationTarget].boardposition != a)
      animationTarget++;

    if(animationTarget < gamePieces.length)
      {
        gamePieces[animationTarget].boardposition = b;              //  Update internal position.

        var start_x = convIndexToX(a);
        var start_y = convIndexToY(a);

        var end_x = convIndexToX(b);
        var end_y = convIndexToY(b);

        var mid_x = midpoint(start_x, end_x);
        var mid_y = midpoint(start_y, end_y);

        animate_startPos = {x: start_x, y: start_y, z:0};
        animate_midPos   = {x: mid_x,   y: mid_y,   z:GAMEPIECE_MOVEMENT_ZENITH};
        animate_endPos   = {x: end_x,   y: end_y,   z:0};

        var tweenHead = new TWEEN.Tween(animate_startPos).to(animate_midPos, 600);
        tweenHead.easing(TWEEN.Easing.Cubic.Out);
        tweenHead.onUpdate(function()
          {
            gamePieces[animationTarget].position.x = animate_startPos.x;
            gamePieces[animationTarget].position.y = animate_startPos.y;
            gamePieces[animationTarget].position.z = animate_startPos.z;
          });

        var tweenTail = new TWEEN.Tween(animate_midPos).to(animate_endPos, 600);
        tweenTail.easing(TWEEN.Easing.Cubic.In);
        tweenTail.onUpdate(function()
          {
            gamePieces[animationTarget].position.x = animate_midPos.x;
            gamePieces[animationTarget].position.y = animate_midPos.y;
            gamePieces[animationTarget].position.z = animate_midPos.z;
          });
        tweenTail.onComplete(function()
          {
            gameEngine.instance.exports.makeMove_client(a, b);      //  Update the game state.
            swapTurns();
          });

        tweenHead.chain(tweenTail);
        tweenHead.start();
      }
  }

function die(a)
  {
    animationTarget = 0;
    while(animationTarget < gamePieces.length && gamePieces[animationTarget].boardposition != a)
      animationTarget++;

    if(animationTarget < gamePieces.length)
      {
        capture_mp3.play();

        gamePieces[animationTarget].boardposition = _NOTHING;       //  Update internal position.

        animate_startScale = {x: gamePieces[animationTarget].scale.x,
                              y: gamePieces[animationTarget].scale.y,
                              z: gamePieces[animationTarget].scale.z};
        animate_endScale   = {x: 1, y: 1, z: 1};
        var tween = new TWEEN.Tween(animate_startScale).to(animate_endScale, 500);
        tween.onUpdate(function()
          {
            gamePieces[animationTarget].scale.x = animate_startScale.x;
            gamePieces[animationTarget].scale.y = animate_startScale.y;
            gamePieces[animationTarget].scale.z = animate_startScale.z;
          });
        tween.onComplete(function()
          {
            removeDeadMesh();
          });
        tween.start();
      }
  }

function removeDeadMesh()
  {
    var i;
    var markedForDeath = [];
    for(i = 0; i < gamePieces.length; i++)
      {
        if(gamePieces[i].boardposition == _NOTHING)
          markedForDeath.push(i);
      }
    while(markedForDeath.length > 0)
      {
        scene.remove(gamePieces[markedForDeath[0]]);
        gamePieces.splice(markedForDeath[0], 1);
        markedForDeath.shift();
      }
    move(Select_A, Select_B);
  }

function swapTurns()
  {
    var winFlag;
    var i;

    Select_A = _NOTHING;                                            //  Reset.
    Select_B = _NOTHING;

    winFlag = gameEngine.instance.exports.isWin_client();           //  Is the game state now terminal?
    if(winFlag != GAME_ONGOING)                                     //  The game state is now terminal.
      {
        artworkForThinking(false);                                  //  Pull "thinking" artwork.
        nodeCounter(false);                                         //  Pull the node counter.

        gameOver = true;                                            //  Signal that the game is over.
        chime_mp3.play();                                           //  Play the sound.

        if(winFlag == GAME_OVER_BLACK_WINS)
          {
            for(i = _A1; i < _NOTHING; i++)                         //  Highlight the winning team.
              {
                if(gameEngine.instance.exports.isBlack_client(i))
                  targetedSq(i);
              }

            switch(currentLang)
              {
                case 'Spanish': if(variantSetup == 'scrambledeggs')
                                  alert(alertStringScrub('\xA1El chorizo gana!'));
                                else
                                  alert(alertStringScrub('\xA1El negro gana!'));
                                break;
                case 'German':  if(variantSetup == 'scrambledeggs')
                                  alert(alertStringScrub('Die Wurst gewinnt!'));
                                else
                                  alert(alertStringScrub('Schwarz gewinnt!'));
                                break;
                case 'Polish':  if(variantSetup == 'scrambledeggs')
                                  alert(alertStringScrub('Kie&#322;basa wygrywa!'));
                                else
                                  alert(alertStringScrub('Czarny wygrywa!'));
                                break;
                default:        if(variantSetup == 'scrambledeggs')
                                  alert(alertStringScrub('Sausage wins!'));
                                else
                                  alert(alertStringScrub('Black wins!'));
              }
          }
        else if(winFlag == GAME_OVER_WHITE_WINS)
          {
            for(i = _A1; i < _NOTHING; i++)                         //  Highlight the winning team.
              {
                if(gameEngine.instance.exports.isWhite_client(i))
                  targetedSq(i);
              }

            switch(currentLang)
              {
                case 'Spanish': if(variantSetup == 'scrambledeggs')
                                  alert(alertStringScrub('\xA1El huevo gana!'));
                                else
                                  alert(alertStringScrub('\xA1El blanco gana!'));
                                break;
                case 'German':  if(variantSetup == 'scrambledeggs')
                                  alert(alertStringScrub('Das Ei gewinnt!'));
                                else
                                  alert(alertStringScrub('Wei&#223; gewinnt!'));
                                break;
                case 'Polish':  if(variantSetup == 'scrambledeggs')
                                  alert(alertStringScrub('Jajko wygrywa!'));
                                else
                                  alert(alertStringScrub('Bia&#322;y wygrywa!'));
                                break;
                default:        if(variantSetup == 'scrambledeggs')
                                  alert(alertStringScrub('Egg wins!'));
                                else
                                  alert(alertStringScrub('White wins!'));
              }
          }
        else
          {
            switch(currentLang)
              {
                case 'Spanish': alert(alertStringScrub('\xA1Empate!'));     break;
                case 'German':  alert(alertStringScrub('Unentschieden!'));  break;
                case 'Polish':  alert(alertStringScrub('Remis!'));          break;
                default:        alert(alertStringScrub('Draw!'));
              }
          }
      }
    else                                                            //  The game is still on going.
      {
        if(CurrentTurn == 'White')
          {
            CurrentTurn = 'Black';
            if(gropius.team == 'White')
              HumansTurn = true;
            else
              HumansTurn = false;
          }
        else
          {
            CurrentTurn = 'White';
            if(gropius.team == 'Black')
              HumansTurn = true;
            else
              HumansTurn = false;
          }

        animationInstruction = null;                                //  Blank out animation instruction.
        animating = false;                                          //  We are no longer animating.
                                                                    //  Restore control to the human player.
        if((CurrentTurn == 'White' && gropius.team == 'Black') || (CurrentTurn == 'Black' && gropius.team == 'White'))
          {
            artworkForThinking(false);                              //  Pull "thinking" artwork.
            nodeCounter(false);                                     //  Pull the node counter.
            gropius.nodeCtr = 0;                                    //  Reset node counter.
            MasterControl = true;
          }
        else
          {
            artworkForThinking(true);                               //  Show the "thinking" artwork.
            updateNodeCounter(gropius.nodeCtr);                     //  Show the A.I.'s node count.
            nodeCounter(true);                                      //  Show the node counter.
          }

        gameEngine.instance.exports.draw();                         //  Output to the console.
                                                                    //  It is the human's turn, and the human must pass.
        if(HumansTurn && !gameEngine.instance.exports.hasAnyMoves_client())
          {
            switch(currentLang)
              {
                case 'Spanish': if(variantSetup == 'scrambledeggs')
                                  {
                                    if(gropius.team == 'Black')
                                      alert(alertStringScrub('Huevo debe pasar.'));
                                    else
                                      alert(alertStringScrub('Chorizo debe pasar.'));
                                  }
                                else
                                  {
                                    if(gropius.team == 'Black')
                                      alert(alertStringScrub('Blanco debe pasar.'));
                                    else
                                      alert(alertStringScrub('Negro debe pasar.'));
                                  }
                                break;
                case 'German':  if(variantSetup == 'scrambledeggs')
                                  {
                                    if(gropius.team == 'Black')
                                      alert(alertStringScrub('Ei muss passen.'));
                                    else
                                      alert(alertStringScrub('Wurst muss passen.'));
                                  }
                                else
                                  {
                                    if(gropius.team == 'Black')
                                      alert(alertStringScrub('Das wei\xDFe Team muss passen.'));
                                    else
                                      alert(alertStringScrub('Das schwarze Team muss passen.'));
                                  }
                                break;
                case 'Polish':  if(variantSetup == 'scrambledeggs')
                                  {
                                    if(gropius.team == 'Black')
                                      alert(alertStringScrub('Jajko musi zda&#263; kolej.'));
                                    else
                                      alert(alertStringScrub('Kie&#322;basa musi zda&#263; kolej.'));
                                  }
                                else
                                  {
                                    if(gropius.team == 'Black')
                                      alert(alertStringScrub('Bia&#322;y zesp&#243;&#322; musi zda&#263; kolej.'));
                                    else
                                      alert(alertStringScrub('Czarny zesp&#243;&#322; musi zda&#263; kolej.'));
                                  }
                                break;
                default:        if(variantSetup == 'scrambledeggs')
                                  {
                                    if(gropius.team == 'Black')
                                      alert(alertStringScrub('"Egg" passes.'));
                                    else
                                      alert(alertStringScrub('"Sausage" passes.'));
                                  }
                                else
                                  {
                                    if(gropius.team == 'Black')
                                      alert(alertStringScrub('White passes.'));
                                    else
                                      alert(alertStringScrub('Black passes.'));
                                  }
              }
                                                                    //  Update the game state.
            gameEngine.instance.exports.makeMove_client(_FORCED_TO_PASS, _FORCED_TO_PASS);
            swapTurns();
          }
      }

    return;
  }
