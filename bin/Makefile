CPPFLAGS=- -I/usr/local/include
LDFLAGS= -L/usr/local/lib
LDLIBS=
CPP=clang++
PDFIUM=/Users/nas/code/forks/pdfium
PDFIUM_INC=$(PDFIUM)/fpdfsdk/include
PDFIUM_V8=$(PDFIUM)/v8/include
PDFIUM_BUILD=$(PDFIUM)/xcodebuild/Debug

MUPDF=/Users/nas/code/forks/mupdf-1.5-source
MUPDF_BUILD=$(MUPDF)/build/release

POPPLER_BUILD=$(MUPDF)/build/release

all: mupdf pdfium

mupdf: mupdf.o
	clang++ $(MUPDF_BUILD)/*.a  -lm  -lcrypto mupdf.o  common.o  -lfreeimage -o mupdf 

mupdf.o: mupdf.cc common.o
	clang -std=c++11 -Iinclude -Igenerated -Wall -pipe -O3 -DHAVE_OPENSSL  -o mupdf.o -c mupdf.cc

## Work in progress, not compiling yet
poppler: poppler.o
	clang++ $(POPPLER_BUILD)/*.a  -lm  -lcrypto poppler.o  common.o  -lfreeimage -o poppler 
poppler.o: poppler.cc common.o
	clang -std=c++11 -I/usr/local/include/poppler -Iinclude -Igenerated -Wall -pipe -O3 -DHAVE_OPENSSL  -o poppler.o -c poppler.cc


common.o: common.cc
	clang -std=c++11 -Iinclude -Igenerated -Wall -pipe -O3  -o common.o -c common.cc

clean:
	rm -rf *o pdfium mupdf


# Copied from xcode's build log, with a few cleanups
pdfium: pdfium.o
	clang++ -L$(PDFIUM_BUILD) -F$(PDFIUM_BUILD)  -mmacosx-version-min=10.9 -Wl,-search_paths_first -lfreeimage $(PDFIUM_BUILD)/*.a -framework AppKit -framework CoreFoundation common.o pdfium.o -o pdfium

pdfium.o: pdfium.cc common.o
	clang -x c++ -std=c++11 -arch x86_64 -fmessage-length=0 -fdiagnostics-show-note-include-stack -fmacro-backtrace-limit=0 -Wno-trigraphs -fno-exceptions -fno-rtti -O0 -Wno-missing-field-initializers -Wno-missing-prototypes -Wno-return-type -Wnon-virtual-dtor -Wno-overloaded-virtual -Wno-exit-time-destructors   -Wall -Wendif-labels -W -Wno-unused-parameter -Wno-missing-braces -Wparentheses -Wswitch -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-variable -Wunused-value -Wno-empty-body -Wno-uninitialized -Wno-unknown-pragmas -Wno-shadow -Wno-four-char-constants -Wno-conversion -Wno-constant-conversion -Wno-int-conversion -Wno-bool-conversion -Wno-enum-conversion -Wno-shorten-64-to-32 -Wno-newline-eof -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk -fstrict-aliasing -Wdeprecated-declarations -Winvalid-offsetof -mmacosx-version-min=10.9 -O3 -fvisibility=hidden -fvisibility-inlines-hidden -Wno-sign-conversion -Iinclude -Igenerated  -I$(PDFIUM_INC) -I$(PDFIUM_V8) -I/usr/local/include -c pdfium.cc -o pdfium.o
