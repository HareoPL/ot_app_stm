#ifndef OT_APP_PORT_RTOS_H_
#define OT_APP_PORT_RTOS_H_


#ifdef ESP_PLATFORM
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h" 

#elif defined(STM_PLATFORM)
    #include "FreeRTOS.h"
    #include "task.h"
    #include "queue.h" 
	#include "semphr.h"

#else
    #error "Unsupported platform. Define ESP_PLATFORM or STM_PLATFORM"
#endif

#endif  /* OT_APP_PORT_RTOS_H_ */
