set(srcs) 
set(public_requires)

file(GLOB_RECURSE srcs
    *.c  
)

list(APPEND public_requires "main")


idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    REQUIRES ${public_requires})
