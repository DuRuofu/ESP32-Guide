idf_component_register(SRCS "flash_fatfs.c"
                    INCLUDE_DIRS ".")

set(image ../fatfs_image)
fatfs_create_spiflash_image(storage ${image} FLASH_IN_PROJECT)
