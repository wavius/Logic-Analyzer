# Define paths
# Quartus Prime project source directory
SRCPATH = ../src/$(QP_NAME)/.
# Destination directory of the generated and compiled project
DSTPATH = ../out/$(QP_NAME)/.
# Release directory of the generated and compiled project
RELPATH = ../rel/DE1-SoC/$(QP_NAME)/.
# The relative path the common scripts directory as compared to the source directory
S2CPATH  = ../../common/scripts
# The relative path the common scripts directory as compared to the destination directory
D2CPATH  = ../../../common/scripts

include $(S2CPATH)/common.mk

