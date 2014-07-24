//
//  _debug.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 7. 22..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef hangeul_debug_h
#define hangeul_debug_h

#if DEBUG
    #include <cdebug/debug.h>
    #include "utf8.h"
#else
    #define dprintfnoln(...)
    #define dassertlognoln(COND, ...)
    #define dlognoln(LEVEL, ...)

    #define dprintfln(...)
    #define dassertlogln(COND, ...)
    #define dlogln(LEVEL, ...)

    #define dprintf(...)
    #define dlog(...)
#endif

#endif
