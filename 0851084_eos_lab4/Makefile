CC = arm-linux-gnueabihf-gcc
CXX = arm-linux-gnueabihf-g++
FLAGS = -g -Wall --static

XLNX_DIR = /home/ncrl/embedded_oper_sys/LAB_2/linux-xlnx
obj-m = mydev.o

all: mydev writer reader

mydev:
	make -C $(XLNX_DIR) M=$(PWD) modules
	cp mydev.ko /home/ncrl/embedded_oper_sys/nfs/

%.o: %.cpp
	$(CXX) -c $< $(FLAGS)

writer: writer.o
	$(CXX) -o $@ $^ $(FLAGS)
	cp writer /home/ncrl/embedded_oper_sys/nfs/

reader: reader.o
	$(CXX) -o $@ $^ $(FLAGS)
	cp reader /home/ncrl/embedded_oper_sys/nfs/

clean:
	make -C $(XLNX_DIR) M=$(PWD) clean
	rm -rf writer writer.o
	rm -rf reader reader.o
	rm -f /home/ncrl/embedded_oper_sys/nfs/writer
	rm -f /home/ncrl/embedded_oper_sys/nfs/reader
	rm -f /home/ncrl/embedded_oper_sys/nfs/mydev.ko

.PYONY: all mydev clean
