# BEANZ Asteroids v1.0 - Audio prototype (GameCube, libogc)
# Adapted in structure from the devkitPro gamecube-examples Makefiles.

.SUFFIXES:
.SECONDARY:

ifeq ($(strip $(DEVKITPPC)),)
$(error DEVKITPPC is not set. Source your devkitPro environment first.)
endif

include $(DEVKITPPC)/gamecube_rules

TARGET   := beanz_asteroids
BUILD    := build
SOURCES  := src
INCLUDES := src

CFLAGS   := -g -O2 -Wall -Wextra $(MACHDEP) $(INCLUDE)
LDFLAGS  := -g $(MACHDEP) -Wl,-Map,$(notdir $@).map
LIBS     := -lasnd -logc -lm

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export LD := $(CC)
export OFILES := $(CFILES:.c=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) -I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC)
export LIBPATHS := -L$(LIBOGC_LIB)

.PHONY: all clean $(BUILD)
all: $(BUILD)

$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol

else

DEPENDS := $(OFILES:.o=.d)
.PHONY: all
all: $(OUTPUT).dol

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
