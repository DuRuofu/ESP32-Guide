GEN_CSRCS += $(notdir $(wildcard $(PRJ_DIR)/generated/images/*.c))

DEPPATH += --dep-path $(PRJ_DIR)/generated/images
VPATH += :$(PRJ_DIR)/generated/images

CFLAGS += "-I$(PRJ_DIR)/generated/images"