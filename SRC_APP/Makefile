#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := hello-world

include $(IDF_PATH)/make/project.mk

web-flash: $(APP_BIN)
	@echo "Flashing app to http POST ..."
	curl --data-binary @$(APP_BIN) http://velogen/flash/upload; curl http://velogen/reboot