all:

	echo 'Compilando Commons'

	cd ../commons/compilar; make;

	echo 'Fin Commons'

	echo 'Compilando PARSER'

	cd ../parser/compilar; make;

	echo 'Fin PARSER'

	echo 'Compilando msgUMV'

	cd ../msgUMV/compilar;make;

	echo 'Fin msgUMV'

	echo 'Compilando protocolo_kernel_cpu'

	cd ../protocolo_kernel_cpu/compilar;make;

	echo 'Fin protocolo_kernel_cpu'

	echo 'Compilando CPU'

	cd ../CPU/compilar;make;

	echo 'Fin CPU'

	echo 'Compilando Kernel'

	cd ../kernel/compilar;make;

	echo 'Fin kernel'

	echo 'Compilado umv'

	cd ../umv/compilar;make;

	echo 'FIn umv'

	echo 'Compilando programa'

	cd ../programa/compilar;make;

	echo 'Fin programa'
	
	sudo mkdir /usr/bin/ansisop;
	sudo mv ../programa/compilar/Programa /usr/bin/ansisop;
	sudo cp ../programa/programa.config /usr/bin/ansisop;

	echo 'Make OK'
	

clean:

	echo 'Compilando Commons'

	cd ../commons/compilar; make clean;

	echo 'Fin Commons'

	echo 'Compilando PARSER'

	cd ../parser/compilar; make clean;

	echo 'Fin PARSER'

	echo 'Compilando msgUMV'

	cd ../msgUMV/compilar;make clean;

	echo 'Fin msgUMV'

	echo 'Compilando protocolo_kernel_cpu'

	cd ../protocolo_kernel_cpu/compilar;make clean;

	echo 'Fin protocolo_kernel_cpu'

	echo 'Compilando CPU'

	cd ../CPU/compilar;make clean;

	echo 'Fin CPU'

	echo 'Compilando Kernel'

	cd ../kernel/compilar;make clean;

	echo 'Fin kernel'

	echo 'Compilado umv'

	cd ../umv/compilar;make clean;

	echo 'FIn umv'

	echo 'Compilando programa'

	cd ../programa/compilar;make clean;

	echo 'Fin programa'
	
	sudo rm -r /usr/bin/ansisop;

	echo 'Clean OK'
