SRCDIR=src
INCDIR=include
BINDIR=bin
OBJDIR=$(BINDIR)/obj
DBDIR=$(BINDIR)/db
CC=clang
CXX=clang++

INCLUDE_NAME=esc
LIB_NAME=libesc.so
TARGET=$(BINDIR)/$(LIB_NAME)
BINARIES=
SRC=$(SRCDIR)/esc.cpp
OBJ=$(patsubst $(SRCDIR)/%,$(OBJDIR)/%.o,$(SRC))

CFLAGS:=-Os -g -Wall -Wextra -Werror -pedantic -std=c11 -fPIC -pthread
CXXFLAGS:=-Os -g -Wall -Wextra -Werror -pedantic -std=c++17 -fPIC -pthread
LDFLAGS:=
INCLUDE:=-I$(INCDIR) -I/usr/local/include

.PHONY: default all build install clean

default: all

all: $(TARGET) $(BINARIES) compile_commands.json

build: $(BINARIES)

install: $(TARGET)
	rm -rf ~/.local/include/$(INCLUDE_NAME)
	mkdir -p ~/.local/include
	mkdir -p ~/.local/lib64
	cp -r include ~/.local/include/$(INCLUDE_NAME)
	cp $(TARGET) ~/.local/lib64/

$(OBJDIR)/%.cpp.o: $(SRCDIR)/%.cpp $(OBJDIR) $(DBDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MJ $(DBDIR)/$*.cpp.o.json -c -o $@ $<

compile_commands.json: $(OBJ) $(DBDIR)
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(patsubst $(OBJDIR)/%.cpp.o,$(DBDIR)/%.cpp.o.json,$<) > $@

$(TARGET): $(OBJ) $(LIB)
	$(CXX) $(LDFLAGS) -shared -o $@ $^

$(OBJDIR): $(BINDIR)
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(DBDIR):
	mkdir -p $(DBDIR)

clean:
	rm -rf bin/
	rm -rf public/
