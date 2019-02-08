# Copy this file into your working directory and edit it accordingly!

# project name
PROJECT = lvbms
MCU = STM32F413ZHTx # Change this if you use a different MCU

BOARD_INC_DIRS = # Whatever additional directories you'd like to include
BOARD_SRC_DIRS = # Whatever additional directories you'd like to compile
BOARD_SRC_FILES = # Individual files you'd like to compile (but not compile their entire directories)
BOARD_DEFS = -DUSING_CAN2 # Defines for your board, should at least include which CAN bus you're using

# This will build unity tests
include ../../build/unity.mk

# This will run board.mk.
include ../../build/board.mk

