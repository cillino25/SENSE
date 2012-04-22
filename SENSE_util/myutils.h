#ifndef MYUTILS_H
 #define MYUTILS_H

 #define _CONCAT(a,b) a##b			// ## is the token-concatenating operator
 #define PORT(x) _CONCAT(PORT,x)
 #define PIN(x) _CONCAT(PIN,x)
 #define DDR(x) _CONCAT(DDR,x)

#endif
