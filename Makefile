SUBDIRS := $(wildcard */.)

clean:
	rm -rf MBus-Server/build MBus-Client/build

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)