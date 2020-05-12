all:
	arm-unknown-linux-gnu-gcc -o led led.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	arm-unknown-linux-gnu-gcc -o 7_seg 7_seg.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	arm-unknown-linux-gnu-gcc -o keypad keypad.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	arm-unknown-linux-gnu-gcc -o LCD_control LCD_control.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	arm-unknown-linux-gnu-gcc -o calculator calculator.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	arm-unknown-linux-gnu-gcc -o lab3 lab3.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	arm-unknown-linux-gnu-gcc -o hw1 hw1.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	cp led /home/ncrl/embedded_oper_sys/nfs/
	cp 7_seg /home/ncrl/embedded_oper_sys/nfs/
	cp keypad /home/ncrl/embedded_oper_sys/nfs/
	cp LCD_control /home/ncrl/embedded_oper_sys/nfs/
	cp calculator /home/ncrl/embedded_oper_sys/nfs/
	cp lab3 /home/ncrl/embedded_oper_sys/nfs/
	cp hw1 /home/ncrl/embedded_oper_sys/nfs/
	cp inventory.txt /home/ncrl/embedded_oper_sys/nfs/

led:
	arm-unknown-linux-gnu-gcc -o led led.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	cp led /home/ncrl/embedded_oper_sys/nfs/

7_seg:
	arm-unknown-linux-gnu-gcc -o 7_seg 7_seg.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	cp 7_seg /home/ncrl/embedded_oper_sys/nfs/

keypad:
	arm-unknown-linux-gnu-gcc -o keypad keypad.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	cp keypad /home/ncrl/embedded_oper_sys/nfs/

LCD_control:
	arm-unknown-linux-gnu-gcc -o LCD_control LCD_control.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	cp LCD_control /home/ncrl/embedded_oper_sys/nfs/

calculator:
	arm-unknown-linux-gnu-gcc -o calculator calculator.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	cp calculator /home/ncrl/embedded_oper_sys/nfs/

lab3:
	arm-unknown-linux-gnu-gcc -o lab3 lab3.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	cp lab3 /home/ncrl/embedded_oper_sys/nfs/

hw1:
	arm-unknown-linux-gnu-gcc -o hw1 hw1.c -L /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/ -I /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/ -I /home/ncrl/embedded_oper_sys/microtime/linux/include/
	cp hw1 /home/ncrl/embedded_oper_sys/nfs/
	cp inventory.txt /home/ncrl/embedded_oper_sys/nfs/

clean:
	rm -f led
	rm -f 7_seg
	rm -f keypad
	rm -f LCD_control
	rm -f calculator
	rm -f lab3
	rm -f hw1
	rm -f /home/ncrl/embedded_oper_sys/nfs/led
	rm -f /home/ncrl/embedded_oper_sys/nfs/7_seg
	rm -f /home/ncrl/embedded_oper_sys/nfs/keypad
	rm -f /home/ncrl/embedded_oper_sys/nfs/LCD_control
	rm -f /home/ncrl/embedded_oper_sys/nfs/calculator
	rm -f /home/ncrl/embedded_oper_sys/nfs/lab3
	rm -f /home/ncrl/embedded_oper_sys/nfs/hw1
