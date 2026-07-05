.PHONY: all build run data clean

all: run

build:
	cmake -S . -B build
	$(MAKE) -C build pitz

run: build
	cd build && ./pitz ../data/krk-A ../data/krk-T ../data/krk-M ../data/km-A ../data/km-T ../data/polish-trains

clean:
	rm -rf build

data:
	rm -rf data
	mkdir -p data
	# Koleje małopolskie
	wget -O data/km-A.zip https://www.kolejemalopolskie.com.pl/rozklady_jazdy/kml-ska-gtfs.zip
	wget -O data/km-T.zip https://www.kolejemalopolskie.com.pl/rozklady_jazdy/ald-gtfs.zip

	# ZTP Kraków
	wget -O data/krk-A.zip https://gtfs.ztp.krakow.pl/GTFS_KRK_A.zip
	wget -O data/krk-M.zip https://gtfs.ztp.krakow.pl/GTFS_KRK_M.zip
	wget -O data/krk-T.zip https://gtfs.ztp.krakow.pl/GTFS_KRK_T.zip

	# All polish Trains
	wget -O data/polish-trains.zip https://mkuran.pl/gtfs/polish_trains.zip


	@for file in data/*.zip; do \
    		folder="$${file%.zip}"; \
    	  	unzip "$$file" -d "$$folder" && rm "$$file"; \
    	done
