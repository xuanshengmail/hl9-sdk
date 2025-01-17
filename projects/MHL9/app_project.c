/*******************************************************************************
 * @file    app_project.c
 * @brief   Project application interface
 *
 * @version 1.0.0
 *******************************************************************************
 * @license Refer License or other description Docs
 * @author  Felix
 ******************************************************************************/
#include "app.h"
#include "app_at.h"
#include "app_mac.h"
#include "at/at_config.h"
#include "radio/sx127x/sx127x_common.h"

#define TASK_PERIOD_MS      100U    /* unit ms */

/****
Global Variables
****/

/* Code Version */
char *gCodeVers = "1020";
volatile bool   gEnableRadioRx  = true;

/****
Local Variables
****/

/****
Local Functions
****/

/**
 * @brief  Initialize all tasks
 *
 * @return  true if success else false
 */
static bool AppTaskInit(void)
{
    bool result = true;

    /* start MAC task */
    APP_FeedDog();
    result = AppMacTask();

    /**
     * we should opearte AT command when other task is OK,
     * so we start AT command task at the end.
     */
    if(result){
        APP_FeedDog();
        result = AppATTask();
    }

    return result;
}

static void AppTaskManager(void)
{
    BSP_OS_MutexLock(&gParam.mutex, OS_ALWAYS_DELAY);
    UserCheckAT();
    BSP_OS_MutexUnLock(&gParam.mutex);
#if 0
    static int count = 0;
    stc_rtc_time_t stcTime;
    if(count++%10 == 0){
        BSP_RTC_GetDateTime(&stcTime);
        printk("%d-%d-%d %d:%d:%d\r\n", stcTime.u8Year + 2000,stcTime.u8Month,stcTime.u8Day,
            stcTime.u8Hour,stcTime.u8Minute,stcTime.u8Second);
    }
#endif
}

/****
Global Functions
****/

/**
 * @brief  Create the App task
 */
bool AppTaskCreate(void)
{
    bool success = false;

    /* watchdog timeout 6s refer MCU datasheet */
    if(RJ_ERR_OK != PlatformInit(6)){
        return false;
    }

    /* Low Energy Timer and DeepSleep init */
    if(false == BSP_LPowerInit(false)){
        return false;
    }

    DevUserInit();

    success = UserDebugInit(false, gDevFlash.config.prop.bdrate, gDevFlash.config.prop.pari);

    printk("LoRa %s SDK, HAL V%u:%u, XTL:%d, Firmware V%s\r\n", MODULE_NAME,
           RADIO_HAL_VERSION, AT_VER, gParam.dev.extl, gCodeVers);

    if(success) {
        success = AppTaskInit();
    }

    return success;
}

void AppTaskExtends(void)
{
    Dev_GetVol();

    while (1) {
        APP_FeedDog();
        AppTaskManager();
        osDelayMs(TASK_PERIOD_MS);
        if(gDevFlash.config.lcp <= 0){
            LED_OFF(LED_RF_RX);
        }
    }
}
