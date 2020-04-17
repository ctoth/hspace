/* ANSI control codes for various neat-o terminal effects

 * Some older versions of Ultrix don't appear to be able to
 * handle these escape sequences. If lowercase 'a's are being
 * stripped from @doings, and/or the output of the ANSI flag
 * is screwed up, you have the Ultrix problem.
 *
 * To fix the ANSI problem, try replacing the '\x1B' with '\033'.
 * To fix the problem with 'a's, replace all occurrences of '\a'
 * in the code with '\07'.
 *
 */

//! @brief ANSI sequences for generating colorized displays

#ifndef __HSANSI_H
#define __HSANSI_H

#ifdef PENNMUSH
// xxx - Penn specific codes, lets think of something smarter!

#define BEEP_CHAR     '\a'
#define ESC_CHAR      '\x1B'

#define ANSI_RAW_NORMAL "\x1B[0m"

#define TAG_START     '\002'
#define TAG_END       '\003'
#define MARKUP_START     "\002"
#define MARKUP_END       "\003"

#define ANSI_HILITE      MARKUP_START "ch" MARKUP_END
#define ANSI_INVERSE     MARKUP_START "ci" MARKUP_END
#define ANSI_BLINK       MARKUP_START "cf" MARKUP_END
#define ANSI_UNDERSCORE  MARKUP_START "cu" MARKUP_END

#define ANSI_INV_BLINK         MARKUP_START "cfi" MARKUP_END
#define ANSI_INV_HILITE        MARKUP_START "chi" MARKUP_END
#define ANSI_BLINK_HILITE      MARKUP_START "cfh" MARKUP_END
#define ANSI_INV_BLINK_HILITE  MARKUP_START "cifh" MARKUP_END

/* Foreground colors */

#define ANSI_PLAIN      MARKUP_START "n" MARKUP_END
#define ANSI_BLACK      MARKUP_START "cx" MARKUP_END
#define ANSI_RED        MARKUP_START "cr" MARKUP_END
#define ANSI_GREEN      MARKUP_START "cg" MARKUP_END
#define ANSI_YELLOW     MARKUP_START "cy" MARKUP_END
#define ANSI_BLUE       MARKUP_START "cb" MARKUP_END
#define ANSI_MAGENTA    MARKUP_START "cm" MARKUP_END
#define ANSI_CYAN       MARKUP_START "cc" MARKUP_END
#define ANSI_WHITE      MARKUP_START "cw" MARKUP_END

#define ANSI_HIBLACK      MARKUP_START "chx" MARKUP_END
#define ANSI_HIRED        MARKUP_START "chr" MARKUP_END
#define ANSI_HIGREEN      MARKUP_START "chg" MARKUP_END
#define ANSI_HIYELLOW     MARKUP_START "chy" MARKUP_END
#define ANSI_HIBLUE       MARKUP_START "chb" MARKUP_END
#define ANSI_HIMAGENTA    MARKUP_START "chm" MARKUP_END
#define ANSI_HICYAN       MARKUP_START "chc" MARKUP_END
#define ANSI_HIWHITE      MARKUP_START "chw" MARKUP_END

/* Background colors */

#define ANSI_BBLACK     MARKUP_START "cX" MARKUP_END
#define ANSI_BRED       MARKUP_START "cR" MARKUP_END
#define ANSI_BGREEN     MARKUP_START "cG" MARKUP_END
#define ANSI_BYELLOW    MARKUP_START "cY" MARKUP_END
#define ANSI_BBLUE      MARKUP_START "cB" MARKUP_END
#define ANSI_BMAGENTA   MARKUP_START "cM" MARKUP_END
#define ANSI_BCYAN      MARKUP_START "cC" MARKUP_END
#define ANSI_BWHITE     MARKUP_START "cW" MARKUP_END

#define ANSI_END        MARKUP_START "c/" MARKUP_END
#define ANSI_ENDALL     MARKUP_START "c/a" MARKUP_END

#define ANSI_NORMAL     ANSI_ENDALL

#else

#define ANSI_BLACK_V    (30)
#define ANSI_RED_V      (31)
#define ANSI_GREEN_V    (32)
#define ANSI_YELLOW_V   (33)
#define ANSI_BLUE_V     (34)
#define ANSI_MAGENTA_V  (35)
#define ANSI_CYAN_V     (36)
#define ANSI_WHITE_V    (37)

#ifndef OLD_ANSI

#define BEEP_CHAR     '\a'
#define ESC_CHAR      '\x1B'

#define ANSI_BEGIN   "\x1B["

#define ANSI_NORMAL   "\x1B[0m"

#define ANSI_HILITE   "\x1B[1m"
#define ANSI_INVERSE  "\x1B[7m"
#define ANSI_BLINK    "\x1B[5m"
#define ANSI_UNDERSCORE "\x1B[4m"

#define ANSI_INV_BLINK         "\x1B[7;5m"
#define ANSI_INV_HILITE        "\x1B[1;7m"
#define ANSI_BLINK_HILITE      "\x1B[1;5m"
#define ANSI_INV_BLINK_HILITE  "\x1B[1;5;7m"

/* Foreground colors */

#define ANSI_BLACK      "\x1B[30m"
#define ANSI_RED        "\x1B[31m"
#define ANSI_GREEN      "\x1B[32m"
#define ANSI_YELLOW     "\x1B[33m"
#define ANSI_BLUE       "\x1B[34m"
#define ANSI_MAGENTA    "\x1B[35m"
#define ANSI_CYAN       "\x1B[36m"
#define ANSI_WHITE      "\x1B[37m"

/* Background colors */

#define ANSI_BBLACK     "\x1B[40m"
#define ANSI_BRED       "\x1B[41m"
#define ANSI_BGREEN     "\x1B[42m"
#define ANSI_BYELLOW    "\x1B[43m"
#define ANSI_BBLUE      "\x1B[44m"
#define ANSI_BMAGENTA   "\x1B[45m"
#define ANSI_BCYAN      "\x1B[46m"
#define ANSI_BWHITE     "\x1B[47m"

#else

#define BEEP_CHAR     '\07'
#define ESC_CHAR      '\033'

#define ANSI_NORMAL   "\033[0m"
#define ANSI_BEGIN    "\033["

#define ANSI_HILITE   "\033[1m"
#define ANSI_INVERSE  "\033[7m"
#define ANSI_BLINK    "\033[5m"
#define ANSI_UNDERSCORE "\033[4m"

#define ANSI_INV_BLINK         "\033[7;5m"
#define ANSI_INV_HILITE        "\033[1;7m"
#define ANSI_BLINK_HILITE      "\033[1;5m"
#define ANSI_INV_BLINK_HILITE  "\033[1;5;7m"

/* Foreground colors */

#define ANSI_BLACK      "\033[30m"
#define ANSI_RED        "\033[31m"
#define ANSI_GREEN      "\033[32m"
#define ANSI_YELLOW     "\033[33m"
#define ANSI_BLUE       "\033[34m"
#define ANSI_MAGENTA    "\033[35m"
#define ANSI_CYAN       "\033[36m"
#define ANSI_WHITE      "\033[37m"

/* Background colors */

#define ANSI_BBLACK     "\033[40m"
#define ANSI_BRED       "\033[41m"
#define ANSI_BGREEN     "\033[42m"
#define ANSI_BYELLOW    "\033[43m"
#define ANSI_BBLUE      "\033[44m"
#define ANSI_BMAGENTA   "\033[45m"
#define ANSI_BCYAN      "\033[46m"
#define ANSI_BWHITE     "\033[47m"

#endif

#define ANSI_END        "m"
#endif

#endif /* __HSANSI_H */
