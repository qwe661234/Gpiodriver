obj-m += gpiodrv.o  
PWD := $(CURDIR) 

all:
	make -C /lib/modules/5.4.0-1069-raspi/build M=$(PWD) modules 

client:
	gcc client.c -o client

clean: 
	make -C /lib/modules/5.4.0-1069-raspi/build M=$(PWD) clean