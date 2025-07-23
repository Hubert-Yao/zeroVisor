CONFIG_MODULE_SIG=n

SRC_DIR := src
BUILD_DIR := build

obj-m := zeroVisor.o

KVERSION = $(shell uname -r)
KDIR := /lib/modules/$(KVERSION)/build

PWD := $(shell pwd)

all: symtable modules move_ko clean_tmp

symtable:
	sudo python3 $(SRC_DIR)/make_symtable.py

modules:
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/$(SRC_DIR) EXTRA_CFLAGS="-I$(PWD)/include" modules

move_ko:
	@echo "Moving .ko to $(BUILD_DIR)..."
	@mv $(SRC_DIR)/zeroVisor.ko $(BUILD_DIR)/

clean_tmp:
	@echo "Cleaning temporary files in $(SRC_DIR)..."
	@rm -f $(SRC_DIR)/*.o $(SRC_DIR)/*.mod.* $(SRC_DIR)/*.mod $(SRC_DIR)/*.symvers $(SRC_DIR)/Module.symvers \
			$(SRC_DIR)/modules.order
	@rm -rf $(SRC_DIR)/.tmp_versions
	@find $(SRC_DIR) -name '*.cmd' -delete

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/$(SRC_DIR) clean
	rm -f $(BUILD_DIR)/zeroVisor.ko
	@$(MAKE) clean_tmp

install:
	sudo insmod $(BUILD_DIR)/zeroVisor.ko

uninstall:
	sudo rmmod zeroVisor