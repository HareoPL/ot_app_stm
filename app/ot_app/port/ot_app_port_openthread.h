#ifndef OT_APP_PORT_OPENTHREAD_H_
#define OT_APP_PORT_OPENTHREAD_H_

#ifdef ESP_PLATFORM
    #include "esp_openthread.h"
    #define otapp_port_openthread_get_instance() esp_openthread_get_instance()

#elif defined(STM_PLATFORM)
    #include "app_thread.h"
    #define otapp_port_openthread_get_instance() APP_THREAD_GetInstance()

#else
    #error "Unsupported platform. Define ESP_PLATFORM or STM_PLATFORM"
#endif

#endif  /* OT_APP_PORT_OPENTHREAD_H_ */
