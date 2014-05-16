#include <baseBridge.h>

mama_status mamaBaseBridge_additionalMethodTest (int test)
{
    mama_log (MAMA_LOG_LEVEL_NORMAL, 
                "In the base bridge implementation of "
                "additionalMethodTest: %d\n", test);
    return MAMA_STATUS_OK;
}
