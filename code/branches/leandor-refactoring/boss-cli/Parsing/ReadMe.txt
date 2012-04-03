========================================================================
    STATIC LIBRARY : LibParsing Project Overview
========================================================================

This static library contains the implementation of the userlist parser.

It has been separated into this library to avoid high recompilation times, 
since they use boost::spirit and that library, due to the heavy use of 
templates for meta-programming, raises compilation times considerably. 


Is organized in two layers:
 * The Model which defines the classes requires by the grammar definition
 * And the Grammars 

Also, the library has two 'views':
 * The public: which are the header files required to use the library,
 * The private: which are the files compiled or used by the library internally.

The public part is the "Parsing.h" header and the Model/*.h needed to use this library.
The private part is everything inside the Private/ directory, which is only used while 
compiling the static library. 

