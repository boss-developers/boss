========================================================================
    STATIC LIBRARY : LibParsing Project Overview
========================================================================

This static library contains the implementation of the masterlist and 
userlist parsers.

They were separated into this library to avoid high recompilation times, 
since they use boost::spirit and that library, due to the heavy use of 
templates for meta-programming, raises compilation times considerably. 

