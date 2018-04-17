
all: 
	cd Caravel && make
	cd FastORE && make
	cd Gateway && make
	cd Middlebox && make
	cd Client && make

remake:
	cd Caravel && make clean && make
	cd FastORE && make clean && make
	cd Gateway && make clean && make
	cd Middlebox && make clean && make
	cd Client && make clean && make

clean:
	cd Caravel && make clean
	cd FastORE && make clean
	cd Gateway && make clean
	cd Middlebox && make clean
	cd Client && make clean


