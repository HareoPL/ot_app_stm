#ifndef OT_APP_PORT_OPENTHREAD_H_
#define OT_APP_PORT_OPENTHREAD_H_

#ifdef ESP_PLATFORM
    #include "esp_openthread.h"
    #define otapp_port_openthread_get_instance() esp_openthread_get_instance()

#elif defined(STM_PLATFORM)
extern otInstance * PtOpenThreadInstance;
    #define otapp_port_openthread_get_instance() (PtOpenThreadInstance)
#else
    #error "Unsupported platform. Define ESP_PLATFORM or STM_PLATFORM"
#endif

#endif  /* OT_APP_PORT_OPENTHREAD_H_ */
