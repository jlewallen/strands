default: static.g.h

clean:
	echo

static.g.h: www/index.html www/bundle.js www/style.css
	xxd -i www/index.html > static.g.h
	xxd -i www/bundle.js >> static.g.h
	xxd -i www/style.css >> static.g.h
