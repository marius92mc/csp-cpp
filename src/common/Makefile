/opt/local/bin/g++ -std=c++11 -o dummyTest.o dummyTest.cpp
/opt/local/bin/g++ -std=c++11 -o Utils.o Utils.cpp

OBJECTS = dummyTest.o Utils.o

myprogram : $(OBJECTS)
	/opt/local/bin/g++ -std=c++11 -o myprogram $(OBJECTS)

#main.o : header1.h
Utils.o :  Utils.h

clean :
	rm myprogram \
	$(OBJECTS)

