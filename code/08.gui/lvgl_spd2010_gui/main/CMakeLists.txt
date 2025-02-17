file(GLOB_RECURSE srcs *.c
                    ui/custom/*.c
                    ui/generated/*.c
                    ui/generated/guider_customer_fonts/*.c
                    ui/generated/guider_fonts/*.c
                    ui/generated/images/*.c
                    )
 
set(include_dirs 
    . 
    ui/custom 
    ui/generated 
    ui/generated/guider_customer_fonts
    ui/generated/guider_fonts
    ui/generated/images

    )
 
idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES lvgl
    )
