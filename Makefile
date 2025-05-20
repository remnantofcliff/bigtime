.PHONY := clean

ifeq (${type}, )
type := default
endif

BUILD_DIR := ./build/${type}

BUILD_BIN := ${BUILD_DIR}/bin
BUILD_EMBED := ${BUILD_DIR}/embed
BUILD_OBJECTS := ${BUILD_DIR}/objects

$(shell mkdir -p \
	${BUILD_BIN} \
	${BUILD_EMBED} \
	${BUILD_OBJECTS} \
)

C_SOURCES := $(shell find src -name '*.c')
DEPENDS := $(patsubst src/%.c, ${BUILD_OBJECTS}/%.d, ${C_SOURCES})
OBJECTS := $(patsubst src/%.c, ${BUILD_OBJECTS}/%.o, ${C_SOURCES})

GLSL_SOURCES := $(shell find src -name '*.glsl')
SPIRV := $(patsubst src/%.glsl, ${BUILD_EMBED}/%.spv, ${GLSL_SOURCES})

FLAGS := -Wall \
	-Wextra \
	-Wpedantic \
	-Wzero-as-null-pointer-constant \
	-Wdouble-promotion \
	-Wconversion \
	-Wfloat-equal \
	-Wformat=2 \
	-Wformat-signedness \
	-Wmissing-declarations \
	-Wshadow \
	-Wswitch-default \
	-MMD \
	-MP \
	--embed-dir=${BUILD_EMBED} \
	--embed-dir=data
SHADER_FLAGS :=
LINKER_FLAGS := -lSDL3

REQUIREMENTS := Makefile ${SPIRV}

ifeq (${type}, default)
FLAGS := ${FLAGS} -g -fsanitize=address,undefined -D GU_DEBUG
SHADER_FLAGS := -g
endif
ifeq (${type}, debug)
FLAGS := ${FLAGS} -g -D GU_DEBUG
SHADER_FLAGS := -g
endif
ifeq (${type}, release)
FLAGS := ${FLAGS} -O3 -flto -DNDEBUG
SHADER_FLAGS :=
endif

GAME := ${BUILD_BIN}/bigtime

${GAME}: ${OBJECTS}
	${CC} ${OBJECTS} ${FLAGS} ${LINKER_FLAGS} -o ${@}

${BUILD_OBJECTS}/%.o: src/%.c ${REQUIREMENTS}
	@mkdir -p $(dir ${@})
	${CC} ${<} ${FLAGS} -std=c23 -c -o ${@}

-include ${DEPENDS}

${BUILD_EMBED}/%.spv: src/%.glsl
	glslangValidator -V ${<} -o ${@}
	spirv-opt -O ${@} -o ${@}

clean:
	rm -rf build
