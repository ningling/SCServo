CC=g++
CFLAGS= -std=c++11  -g
INCLUDE=-I./includes -I./external/tclap-1.2.2/include
OBJDIR=./obj/
LIBDIR=./lib/
LIBSRCDIR=./src/
LIBSRC = $(notdir $(wildcard $(LIBSRCDIR)*.cpp))
LIBOBJ = $(LIBSRC:.cpp=.o)
LIBNAME = lib$(LIBSRC:.cpp=).a

BINDIR=./bin/
BINSRCDIR=./utils/
BINSRC = $(notdir $(wildcard $(BINSRCDIR)*.cpp))
BINNAME = $(BINSRC:.cpp=.out)
#define default action
default:
	@echo "make factor project v1.0"
	@echo "use make all"

all: $(BINNAME) $(LIBNAME)
#all: $(LIBNAME)
$(BINNAME): %.out:$(BINSRCDIR)%.cpp $(LIBNAME)
	$(CC) $< $(CFLAGS) -L$(LIBDIR) -l$(LIBSRC:.cpp=) $(INCLUDE) -o $(BINDIR)$*

$(LIBNAME): $(LIBOBJ)
	ar rcs $(LIBDIR)$(LIBNAME) $(foreach n,$(LIBOBJ),$(OBJDIR)$(n))

$(LIBOBJ): %.o:$(LIBSRCDIR)%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $(OBJDIR)$@
clean:
	-rm $(LIBDIR)*.* $(OBJDIR)*.* $(BINDIR)*
