# Makefile  — in repo root (LinuxTaskManager/Makefile)

.PHONY: all core daemon clean

all: core daemon

core:
	$(MAKE) -C c-core

daemon:
	$(MAKE) -C c-daemon

clean:
	$(MAKE) -C c-core clean
	$(MAKE) -C c-daemon clean
