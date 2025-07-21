C=gcc
CFLAGS = -Wall  #-Werror=return-type

CFLAGS_DEBUG = -g -DENABLE_DEBUG
CFLAGS_RELEASE = -O3 -DNDEBUG

TARGETS = inject
INCLUDE = -I.
LINK = 

.DEFAULT_GOAL := all

.PHONY: all
all: release

prerun:
	$(shell mkdir -p build)

.PHONY: debug
debug: CFLAGS += ${CFLAGS_DEBUG}
debug: prerun $(TARGETS)

.PHONY: release
release: CFLAGS += ${CFLAGS_RELEASE}
release: prerun $(TARGETS)

$(TARGETS): %: %.o
	${C} ${CFLAGS} -o build/$@ build/$^ ${LINK}

%.o : src/%.c
	${C} -c ${CFLAGS} $< ${INCLUDE} -o build/$(notdir $@)

.PHONY: clean
clean: rm build/*
