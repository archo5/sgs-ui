
include sgscript/core.mk


CXXFLAGS = -DBUILDING_SGS_UI -fno-exceptions -fno-rtti -static-libstdc++ -static-libgcc -fno-unwind-tables -fvisibility=hidden \
	$(call fnIF_RELEASE,-O3,-D_DEBUG -g) $(call fnIF_COMPILER gcc,-static-libgcc,) \
	$(call fnIF_ARCH,x86,-m32,$(call fnIF_ARCH,x64,-m64,)) -Isrc \
	$(call fnIF_OS,windows,,-fPIC -D_FILE_OFFSET_BITS=64)

LINKFLAGS = $(call fnIF_OS,windows,-lkernel32,) -lsgscript -shared -Lsgscript/bin

ifeq ($(mode),release)
	CFLAGS = -O3 $(CXXFLAGS)
else
	mode = debug
	CFLAGS = -D_DEBUG -g $(CXXFLAGS)
endif


# BUILD INFO
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),clean_all)
$(info -------------------)
$(info SGS-UI build info)
$(info -------------------)
$(info OS - $(cOS))
$(info ARCH - $(cARCH))
$(info COMPILER - $(cCOMPILER))
$(info TARGET - $(target_os)/$(target_arch))
$(info MODE - $(call fnIF_RELEASE,release,debug))
$(info OUT.LIB. - $(OUTDIR)/sgs-ui$(LIBEXT))
$(info TODO - $(MAKECMDGOALS))
$(info -------------------)
endif
endif


_DEPS = ui_control.h
DEPS = $(patsubst %,src/%,$(_DEPS))

_OBJ = ui_control.o cppbc_ui_control.o ui_main.o
OBJ = $(patsubst %,obj/%,$(_OBJ))

# the library (default target)
.PHONY: make
make: $(OUTDIR)/sgs-ui$(LIBEXT)

$(OUTDIR)/sgs-ui$(LIBEXT): $(OBJ) $(OUTDIR)/$(LIBPFX)sgscript$(LIBEXT)
	$(CXX) -Wall -o $@ $(OBJ) $(CFLAGS) $(LINKFLAGS)

obj/%.o: src/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(COMPATHS) -I. -Isgscript/src -Isgscript/ext/cppbc $(CFLAGS)

src/cppbc_ui_control.cpp: src/ui_control.h sgscript/bin/sgsvm$(BINEXT)
	sgscript/bin/sgsvm -p sgscript/ext/cppbc/cppbc.sgs src/ui_control.h

# SGScript
$(OUTDIR)/$(LIBPFX)sgscript$(LIBEXT): sgscript/$(OUTDIR)/$(LIBPFX)sgscript$(LIBEXT)
	$(fnCOPY_FILE) $(call fnFIX_PATH,sgscript/$(OUTDIR)/$(LIBPFX)sgscript$(LIBEXT) $(OUTDIR))
sgscript/$(OUTDIR)/$(LIBPFX)sgscript$(LIBEXT):
	make -C sgscript
sgscript/bin/sgsvm$(BINEXT):
	make -C sgscript vm

# clean build data
.PHONY: clean clean_all
clean_all: clean
	-make -C sgscript clean
	-$(fnREMOVE_ALL) $(call fnFIX_PATH,obj/*.o obj/*.a obj/*.lib)
clean:
	-$(fnREMOVE_ALL) $(call fnFIX_PATH,obj/*.o $(OUTDIR)/sgs* $(OUTDIR)/libsgs*)
