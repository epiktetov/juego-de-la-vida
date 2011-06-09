GtkFLGS := $(shell gtk-config --cflags)
GtkLIBS := $(shell gtk-config --libs)

SRCS     := elpp.cpp elmundo.cpp elmundo-a.cpp elvista.cpp
CC       := g++
CPPFLAGS := -g $(GtkFLGS)
LIBS     :=    $(GtkLIBS)
OFILES   := $(SRCS:%.cpp=%.o)

elpp: $(OFILES)
	$(CC) $(CPPFLAGS) -o elpp $(OFILES) $(LIBS)

clean:
	rm -f elpp *.o

depend: $(SRCS)
	makedepend -Y $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

elpp.o: elpp.h elmundo.h elvista.h
elmundo.o: elpp.h elmundo.h elvista.h
elmundo-a.o: elpp.h elmundo.h elvista.h
elvista.o: elpp.h elvista.h elmundo.h

