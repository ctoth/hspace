/*-----------------------------------------------------------------
 * Local functions
 *
 * This file is reserved for local functions that you may wish
 * to hack into PennMUSH. Read parse.h for information on adding
 * functions. This file will not be overwritten when you update
 * to a new distribution, so it's preferable to add new functions
 * here and leave the other fun*.c files alone.
 *
 */

/* Here are some includes you're likely to need or want.
 * If your functions are doing math, include <math.h>, too.
 */
#include "copyrite.h"
#include "config.h"
#include <string.h>
#include "conf.h"
#include "externs.h"
#include "parse.h"
#include "confmagic.h"
#include "function.h"

void local_functions(void);

/* Here you can use the new add_function instead of hacking into function.c
 * Example included :)
 */
#ifdef HSPACE
FUNCTION_PROTO(hsf_info);
FUNCTION_PROTO(hsf_console_cmd);
FUNCTION_PROTO(hsf_set);
FUNCTION_PROTO(hsf_get);
FUNCTION_PROTO(hsf_shipsys_set);
FUNCTION_PROTO(hsf_shipsys_get);
FUNCTION_PROTO(hsf_clone);
FUNCTION_PROTO(hsf_systems);
FUNCTION_PROTO(hsf_list);
FUNCTION_PROTO(hsf_placesystem);
FUNCTION_PROTO(hsf_removesystem);
FUNCTION_PROTO(hsf_cargo_transfer);
FUNCTION_PROTO(hsf_cargo_get);
FUNCTION_PROTO(hsf_cargo_set);
FUNCTION_PROTO(hsf_is_object);
FUNCTION_PROTO(hsf_new);
FUNCTION_PROTO(hsf_srep);
#endif

#ifdef EXAMPLE
FUNCTION(local_fun_silly)
{
  safe_format(buff, bp, "Silly%sSilly", args[0]);
}

#endif

void
local_functions(void)
{
#ifdef HSPACE
  function_add("HS_INFO", hsf_info, 0, 0, FN_REG);
  function_add("HS_CONSOLE_CMD", hsf_console_cmd, 1, 5, FN_REG);
  function_add("HS_SET", hsf_set, 4, 4, FN_WIZARD);
  function_add("HS_GET", hsf_get, 3, 3, FN_ADMIN);
  function_add("HS_SHIPSYS_SET", hsf_shipsys_set, 3, 4, FN_WIZARD);
  function_add("HS_SHIPSYS_GET", hsf_shipsys_get, 3, 3, FN_ADMIN);
  function_add("HS_CLONE", hsf_clone, 1, 1, FN_WIZARD);
  function_add("HS_SYSTEMS", hsf_systems, 1, 2, FN_ADMIN);
  function_add("HS_LIST", hsf_list, 1, 2, FN_ADMIN);
  function_add("HS_PLACESYSTEM", hsf_placesystem, 3, 3, FN_WIZARD);
  function_add("HS_REMOVESYSTEM", hsf_removesystem, 2, 2, FN_WIZARD);
  function_add("HS_CARGO_TRANSFER", hsf_cargo_transfer, 4, 4, FN_WIZARD);
  function_add("HS_CARGO_GET", hsf_cargo_get, 2, 2, FN_WIZARD);
  function_add("HS_CARGO_SET", hsf_cargo_set, 3, 3, FN_WIZARD);
  function_add("HS_IS_OBJECT", hsf_is_object, 2, 2, FN_REG);
  function_add("HS_NEW", hsf_new, 1, 3, FN_WIZARD);
  function_add("HS_SREP", hsf_srep, 1, 2, FN_ADMIN);
#endif
#ifdef EXAMPLE
  function_add("SILLY", local_fun_silly, 1, 1, FN_REG);
#endif
}
