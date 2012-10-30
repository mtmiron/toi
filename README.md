The TOI interpreter: an independent study project
=================================================

A basic rundown in a single sentence
------------------------------------
  * TOI is split up between the core interpreter's source files and the modular "extension" files that implement specific functionality (e.g. string.c [implementing strings], array.c [implementing arrays], etc.); see the source and comments in "toi.h" for a more detailed description of specific structures/files/implementation-details.



Notable features/functionality
------------------------------
  * Exception handling.	[see exception.toi for demonstration]
  * File input/output.	[see file.toi for demonstration]
  * Dynamic class and method definitions.	[see def.toi for demonstration]
  * Informative (and understandable) error messages, including syntax and parsing errors.	[see error.toi for demonstration]
  * A highly extensible and modular design that allows for very simple and easy additions to existing functionality/source code.	[see "extending.txt" for more info]
  * A slew of out-of-the-box, miscellaneous functionality designed to make programming less of a chore and more of a painless, playful amusement (like it [arguably] should be).



The fundamental (i.e. really, REALLY important) core source files
-----------------------------------------------------------------
  * parse.y: this file contains the totality of the parsing/tokenizing code, and is a Bison-compatible file.
  * class.c: the bulk of the code relating to implementation of a fully object-oriented environment is in this file.
  * symbol.c: most of the code that deals with dynamic variable assignment/reference, etc..
  * eval.c: the vast majority of the actual evaluation of the instruction sequence that results from parsing/"compiling" a script file is in here.  Also houses some other code that's integral to the core interpreter.
  * toi.h: the header file containing universal prototypes and definitions; it absolutely must be #include'd in any TOI source file.
