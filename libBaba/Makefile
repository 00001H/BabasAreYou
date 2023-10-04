COMPOPT := -I"E:/include" -I"C:/GL/include" $(default_gcc_args) -std=c++23 -L"C:/GL/libs" -L"E:/libs"
SOURCES := $(wildcard *.cpp)
HEADERS := $(wildcard *.hpp)
BMH := $(wildcard ../basemod/*.hpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
.PHONY: cleanobj clean
libbaba.a: $(OBJECTS)
	ar rcs libbaba.a $(OBJECTS)
cleanobj:
	del *.o
clean: cleanobj
	del *.exe
%.o: %.cpp $(HEADERS)
	g++ -c $< $(COMPOPT)
../demo.exe: libbaba.a ../demo.cpp $(BMH)
	g++ "../demo.cpp" -o"../demo.exe" $(COMPOPT) -L. -lbaba -lpygame -lglfw3 -lgdi32 -lglad -lfreetype -luila
