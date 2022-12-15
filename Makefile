FQBN := esp8266:esp8266:d1_mini
TARGET := ../target

default: build-desk

# Right now where's an issue with consecutive builds that use build-path with a
# path in the project directory.
# https://github.com/arduino/arduino-cli/issues/1646

build-desk: build-deps
	arduino-cli compile -b $(FQBN) --build-path $(TARGET)/desk --build-property "build.extra_flags=-DESP8266 -Dleds_desk"

upload-desk: build-desk
	arduino-cli upload -b $(FQBN) -i $(TARGET)/desk/strands.ino.bin --port /dev/ttyUSB0


build-tree-living: build-deps
	arduino-cli compile -b $(FQBN) --build-path $(TARGET)/tree-living --build-property "build.extra_flags=-DESP8266 -Dleds_tree_living"

upload-tree-living: build-tree-living
	arduino-cli upload -b $(FQBN) -i $(TARGET)/tree-living/strands.ino.bin --port /dev/ttyUSB0


build-tree-yard: build-deps
	arduino-cli compile -b $(FQBN) --build-path $(TARGET)/tree-yard --build-property "build.extra_flags=-DESP8266 -Dleds_tree_yard"

upload-tree-yard: build-tree-yard
	arduino-cli upload -b $(FQBN) -i $(TARGET)/tree-yard/strands.ino.bin --port /dev/ttyUSB0


build-house-eaves: build-deps
	arduino-cli compile -b $(FQBN) --build-path $(TARGET)/house-eaves --build-property "build.extra_flags=-DESP8266 -Dleds_house_eaves"

upload-house-eaves: build-house-eaves
	arduino-cli upload -b $(FQBN) -i $(TARGET)/house-eaves/strands.ino.bin --port /dev/ttyUSB0


build-deps: static.g.h

static.g.h: www/index.html www/bundle.js www/style.css www/jquery-3.6.1.min.js.gz
	xxd -i www/index.html > static.g.h
	xxd -i www/bundle.js >> static.g.h
	xxd -i www/style.css >> static.g.h
	xxd -i www/jquery-3.6.1.min.js.gz >> static.g.h
	sed -i "s/unsigned char [^[:space:]]\+\[\]/& PROGMEM/g" static.g.h

clean:
	rm -rf $(TARGET)
