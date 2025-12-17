
set(srcs) 
set(public_requires)

file(GLOB_RECURSE srcs
    src/*.c  
)

list(APPEND public_requires "main")
list(APPEND public_requires "driver")
list(APPEND public_requires "openthread")
# list(APPEND public_requires "json")
# list(APPEND public_requires "spiffs")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "inc"
                    REQUIRES ${public_requires})
