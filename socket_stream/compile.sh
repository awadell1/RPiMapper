# Stop any currently runing instances of socketStream
# Note: May need to run as sudo
killall -9 socketStream

# Compile socketStream from source
gcc -std=c11 main.c -o socketStream
