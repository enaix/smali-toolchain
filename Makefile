C=gcc
CFLAGS = -Wall  #-Werror=return-type

CFLAGS_DEBUG = -g -DENABLE_DEBUG -O2
CFLAGS_RELEASE = -O3 -DNDEBUG

TARGETS = inject
INCLUDE = -I.
LINK = 

.DEFAULT_GOAL := debug

.PHONY: all
all: debug

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
