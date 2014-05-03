chess
=====

A tiny chess AI
Version 0.0
(still in alpha, don't expect perfection)

Build
-----

  To build you just need to run make:

    make chess

  It has only ever been built on Ubuntu 12.04 64 bit.

Usage
-----

  Puzzle may be input in FEN notation:

    ./chess -p "8/8/8/8/8/6K1/5R2/7k w - - 0 0"

  You may play against the computer:

    ./chess -g h c

  The 'h' and 'c' mean human player and computer player for the first
  and second players respectively.
  You must input moves in PNG algebraic notation - capitolization counts.
  You can also play a game from any FEN starting possition:

    ./chess -g c h "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
