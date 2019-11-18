# gtk-example

usage 
main.c:

gcc `pkg-config --libs --cflags gtk+-3.0` main.c -o main
./main

gtk_thread.c:

gcc `pkg-config --libs --cflags gtk+-3.0` gtk_thread.c -o thread
./thread
