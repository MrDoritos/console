g++ -c -o console.windows.o console.windows.cpp -I. -DDLLEXPORT 
g++ -o console.windows.dll console.windows.o -s -shared 
