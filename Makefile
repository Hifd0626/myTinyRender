SYSCONF_LINK = g++
CPPFLAGS     = -std=c++17 -fopenmp
LDFLAGS      =
LIBS         = -lm

DESTDIR = ./
TARGET  = photo

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(DESTDIR)$(TARGET) run

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) -Wall $(CPPFLAGS) $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS)

$(OBJECTS): %.o: %.cpp
	$(SYSCONF_LINK) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

run: $(DESTDIR)$(TARGET)
	./$(TARGET).exe 
	magick output.tga output.jpg  

clean:
	del /F /Q $(OBJECTS)
	del /F /Q $(TARGET)
	del /F /Q *.tga

