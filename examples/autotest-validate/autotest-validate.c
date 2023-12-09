/**
* A simple file to validate your automated test setup for AESD
*/

#include "autotest-validate.h"
#include <stdbool.h>
#include "../../assignment-autotest/test/assignment1/username-from-conf-file.h"

/**
* @return true (as you may have guessed from the name)
*/
bool this_function_returns_true()
{
    return true;
}

/**
* @return false (as you may have guessed from the name)
*/
bool this_function_returns_false()
{
    return false;
}

/**
 * @return a string which contains the username you use for
 * git submissions.  This string should match the string in conf/username.txt
 */
const char *my_username()
{
    return malloc_username_from_conf_file();
}
