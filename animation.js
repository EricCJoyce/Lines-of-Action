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
                                 if(Object.hasOwn(animationInstruction, 'promo'))
                                   PromotionTarget = animationInstruction.promo;
                                 else
                                   PromotionTarget = _NO_PROMO;
                                 move(animationInstruction.a, animationInstruction.b);
                                 break;
            case 'die':          Select_A = animationInstruction.a;
                                 Select_B = animationInstruction.b;
                                 if(Object.hasOwn(animationInstruction, 'promo'))
                                   PromotionTarget = animationInstruction.promo;
                                 else
                                   PromotionTarget = _NO_PROMO;
                                 commit_mp3.play();
                                 die(animationInstruction.b);
                                 break;
          }
      }
  }

function move(a, b)
  {
    animationTarget = 0;
    while(animationTarget < gamePieces.length && gamePieces[animationTarget].chessposition != a)
      animationTarget++;

    if(animationTarget < gamePieces.length)
      {
        gamePieces[animationTarget].chessposition = b;              //  Update internal position.

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
                                                                    //  Update the game state.
            gameEngine.instance.exports.makeMove_client(a, b, _NO_PROMO);
            swapTurns();
          });

        tweenHead.chain(tweenTail);
        tweenHead.start();
      }
  }

function die(a)
  {
    animationTarget = 0;
    while(animationTarget < gamePieces.length && gamePieces[animationTarget].chessposition != a)
      animationTarget++;

    if(animationTarget < gamePieces.length)
      {
        gamePieces[animationTarget].chessposition = _NOTHING;       //  Update internal position.

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
        if(gamePieces[i].chessposition == _NOTHING)
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
            i = 0;                                                  //  Highlight the checkmated king.
            while(!(gameEngine.instance.exports.isBlack_client(i) && gameEngine.instance.exports.isKing_client(i)))
              i++;
            selectedSq(i);

            switch(currentLang)
              {
                case 'Spanish': alert(alertStringScrub('\xA1Jaque mate!'));  break;
                case 'German': alert(alertStringScrub('Schachmatt!'));  break;
                case 'Polish': alert(alertStringScrub('Mat!'));  break;
                default: alert(alertStringScrub('Checkmate!'));
              }
          }
        else if(winFlag == GAME_OVER_WHITE_WINS)
          {
            i = 0;                                                  //  Highlight the checkmated king.
            while(!(gameEngine.instance.exports.isWhite_client(i) && gameEngine.instance.exports.isKing_client(i)))
              i++;
            selectedSq(i);

            switch(currentLang)
              {
                case 'Spanish': alert(alertStringScrub('\xA1Jaque mate!'));  break;
                case 'German': alert(alertStringScrub('Schachmatt!'));  break;
                case 'Polish': alert(alertStringScrub('Mat!'));  break;
                default: alert(alertStringScrub('Checkmate!'));
              }
          }
        else
          {
            switch(currentLang)
              {
                case 'Spanish': alert(alertStringScrub('\xA1Estancamiento!'));  break;
                case 'German': alert(alertStringScrub('Patt!'));  break;
                case 'Polish': alert(alertStringScrub('Pat!'));  break;
                default: alert(alertStringScrub('Stalemate!'));
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
      }

    return;
  }
