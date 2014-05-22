chess
=====

A tiny chess AI

Version 0.0.1 (still in alpha, don't expect perfection)

Build
-----

  To build you just need to run make:

    make chess
    make openings 

  The first command builds the chess main program, the second generates the opening book from the game database.

  It has only ever been built on Ubuntu 12.04 64 bit, so... good luck on any other machine.

Usage
-----

  Puzzles may be input in FEN notation:

    ./chess -p "8/8/8/8/8/6K1/5R2/7k w - - 0 0"

  You may play against the computer:

    ./chess -g h c

  The 'h' and 'c' mean human player and computer player for the first
  and second players respectively.
  You must input moves in PGN algebraic notation - capitolization counts.

  The -t parameter may be included for either computer player if you want to specify the number of 
  seconds the AI will think (by default 10)

    ./chess -g h c -t 60

  You can also play a game from any FEN starting possition:

    ./chess -g c h -fen "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"

  You may specify a PGN file to save your game in after it's over.

    ./chess -g c h -pgn file.pgn

  If that pgn file already exists and is an incomplete game (result *), the game will be resumed.

  The opening interface, -o, may be used to:
  
    -c: compile the opening book (can take upwards of an hour),
    
    -p: print the opening book for
    
    -m: minimum number occurences in the source game database,
    
    -r: or generate a random opening..

    ./chess -o -p -m 25 -r
