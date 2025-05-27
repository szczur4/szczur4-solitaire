Needed for everything
1. Install MSYS2 with ncursesw
2. Add <msys2 folder path>\mingw64\bin to PATH

Playing
1. Run pasjans.exe
2. Right-click on the title
3. Click "properties"
4. In "options" tab turn off "quick edit mode"
5. You can adjust the colors in "colors" tab
6. Resize the window so everything fits
7. Instuctions how to play are displayed in the game

Compiling
1. Run cmd in the folder with main.cpp
2. Run the folowing command: gcc -std=c++26 main.cpp -o pasjans.exe -llibstdc++ -lncursesw
