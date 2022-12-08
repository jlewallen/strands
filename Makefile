FQBN := esp8266:esp8266:d1_mini

default: build-desk


build-desk: build-deps
	arduino-cli compile -b $(FQBN) --build-path target/desk --build-property "build.extra_flags=-DESP8266 -Dleds_desk"

upload-desk: build-desk
	arduino-cli upload -b $(FQBN) -i target/desk/strands.ino.bin --port /dev/ttyUSB0


build-tree-living: build-deps
	arduino-cli compile -b $(FQBN)--build-path target/tree-living --build-property "build.extra_flags=-DESP8266 -Dleds_tree_living"

upload-tree-living: build-tree-living
	arduino-cli upload -b $(FQBN) -i target/tree-living/strands.ino.bin --port /dev/ttyUSB0


build-tree-yard: build-deps
	arduino-cli compile -b $(FQBN) --build-path target/tree-yard --build-property "build.extra_flags=-DESP8266 -Dleds_tree_yard"

upload-tree-yard: build-tree-yard
	arduino-cli upload -b $(FQBN) -i target/tree-yard/strands.ino.bin --port /dev/ttyUSB0


build-house-eaves: build-deps
	arduino-cli compile -b $(FQBN) --build-path target/house-eaves --build-property "build.extra_flags=-DESP8266 -Dleds_house_eaves"

upload-house-eaves: build-house-eaves
	arduino-cli upload -b $(FQBN) -i target/house-eaves/strands.ino.bin --port /dev/ttyUSB0


build-deps: static.g.h

static.g.h: www/index.html www/bundle.js www/style.css
	xxd -i www/index.html > static.g.h
	xxd -i www/bundle.js >> static.g.h
	xxd -i www/style.css >> static.g.h

clean:
	rm -rf target
