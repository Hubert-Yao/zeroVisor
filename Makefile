CONFIG_MODULE_SIG=n

SRC_DIR := src
BUILD_DIR := build

obj-m := zv_core.o

KVERSION = $(shell uname -r)
KDIR := /lib/modules/$(KVERSION)/build

PWD := $(shell pwd)

all: modules move_ko clean_tmp

modules:
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/$(SRC_DIR) modules

move_ko:
	@echo "Moving .ko to $(BUILD_DIR)..."
	@mv $(SRC_DIR)/zv_core.ko $(BUILD_DIR)/

clean_tmp:
	@echo "Cleaning temporary files in $(SRC_DIR)..."
	@rm -f $(SRC_DIR)/*.o $(SRC_DIR)/*.mod.* $(SRC_DIR)/*.symvers $(SRC_DIR)/Module.symvers \
			$(SRC_DIR)/modules.order
	@rm -rf $(SRC_DIR)/.tmp_versions
	@find $(SRC_DIR) -name '*.cmd' -delete

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/$(SRC_DIR) clean
	rm -f $(BUILD_DIR)/zv_core.ko

install:
	sudo insmod $(BUILD_DIR)/zv_core.ko

uninstall:
	sudo rmmod zv_core