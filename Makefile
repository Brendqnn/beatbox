CFLAGS = -Ilib/raylib-5.0_win64_mingw-w64/include

LDFLAGS = -Llib/raylib-5.0_win64_mingw-w64/lib -lraylib -lopengl32 -lgdi32 -lwinmm

all:
	gcc $(CFLAGS) -o main.exe src/*.c $(LDFLAGS) && main.exe

