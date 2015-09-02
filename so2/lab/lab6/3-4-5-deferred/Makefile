all: deploy

deploy:
	cd kernel && make && make deploy
	cd user && make && make deploy
	cd ../../../qemu-vm/ && make

clean:
	cd kernel && make clean
	cd user && make clean
