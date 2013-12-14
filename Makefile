# Compiling accelerator	
all : Accelerator.o FileHelper.o
	g++ -Wall -O2 -o Accelerator Accelerator.o `pkg-config fuse --cflags --libs` FileHelper.o

Accelerator.o : Accelerator.cpp
	g++ -Wall -O2 -c Accelerator.cpp `pkg-config fuse --cflags --libs` -o Accelerator.o

# Compiling other classes

FileHelper: FileHelper.o

FileHelper.o : FileHelper.cpp
	g++ -O2 -c FileHelper.cpp -o FileHelper.o
	
# Cleaning compilation
clean :
	rm -rf *.o *~
cleanall :
	rm -rf accelerator *.o *~
