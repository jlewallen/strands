default: build

build: static.g.h
	arduino-cli compile -b esp8266:esp8266:d1_mini

upload: build
	arduino-cli upload -b esp8266:esp8266:d1_mini --port /dev/ttyUSB0

static.g.h: www/index.html www/bundle.js www/style.css
	xxd -i www/index.html > static.g.h
	xxd -i www/bundle.js >> static.g.h
	xxd -i www/style.css >> static.g.h

clean:
	echo
