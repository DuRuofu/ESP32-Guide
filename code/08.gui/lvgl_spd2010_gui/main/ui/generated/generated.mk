# images
include $(PRJ_DIR)/generated/images/images.mk

# GUI Guider fonts
include $(PRJ_DIR)/generated/guider_fonts/guider_fonts.mk

# GUI Guider customer fonts
include $(PRJ_DIR)/generated/guider_customer_fonts/guider_customer_fonts.mk


GEN_CSRCS += $(notdir $(wildcard $(PRJ_DIR)/generated/*.c))

DEPPATH += --dep-path $(PRJ_DIR)/generated
VPATH += :$(PRJ_DIR)/generated

CFLAGS += "-I$(PRJ_DIR)/generated"