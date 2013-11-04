
ifdef SystemRoot
	RM = del /Q
	CP = copy
	FixPath = $(subst /,\,$1)
	PLATFLAGS = -lkernel32
	LINKPATHS = -Lsgscript/bin
	COMPATHS =
	PLATPOST =
	LIBPFX=
	LIBEXT=.dll
else
	RM = rm -f
	CP = cp
	FixPath = $1
	PLATFLAGS = -lm -Wl,-rpath,'$$ORIGIN' -Wl,-z,origin
	LINKPATHS = -Lsgscript/bin
	COMPATHS =
	PLATPOST =
	LIBPFX=lib
	LIBEXT=.so
endif

SRCDIR=src
OUTDIR=bin
OBJDIR=obj

CXX=g++
ifeq ($(mode),release)
	CFLAGS = -O3 -Wall
else
	mode = debug
	CFLAGS = -D_DEBUG -g -Wall
endif

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -DBUILDING_SGS_UI

_DEPS = ui_control.h
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = ui_control.o cppbc_ui_control.o ui_main.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OUTDIR)/sgs-ui$(LIBEXT): $(OBJ)
	$(MAKE) -C sgscript
	$(CXX) -Wall -o $@ $(OBJ) $(CXXFLAGS) -Lsgscript/bin $(LINKPATHS) $(PLATFLAGS) -lsgscript -shared -static-libgcc -static-libstdc++
	$(PLATPOST)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(COMPATHS) -I. -Isgscript/src -Isgscript/ext/cppbc $(CXXFLAGS)

$(SRCDIR)/cppbc_ui_control.cpp: $(SRCDIR)/ui_control.h
	sgsvm -p sgscript/ext/cppbc/cppbc.sgs $(SRCDIR)/ui_control.h


.PHONY: clean
clean:
	$(RM) $(call FixPath,$(OBJDIR)/*.o $(OUTDIR)/sgs*)

.PHONY: clean_all
clean_all: clean
	$(MAKE) -C sgscript clean

