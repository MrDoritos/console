# console
Cross platform console graphics development

It's as easy as <br />
```#include "console.h"```

```
#include "console.h"

int main() {
  console::write(0,0,"Hello console!");
  console::write(0,1,"Green on black!", FGREEN | BBLACK);
  console::write(0,2,"Press ESC to exit", FRED | BBLUE);
  
  while (console::readKey() != VK_ESCAPE);
  
  return 0;
}
```

Automatically creates a buffer on both Linux and Windows. Supports 16 colors on Windows and 8 on Linux. Unicode still WIP

If you would like something a little more advanced just do <br />
```#include "advancedConsole.h"```

```
#include "advancedConsole.h"

int main() {    
   adv::setDrawingMode(DRAWINGMODE_COMPARE); //Set only new pixels instead of uploading all of the buffer
   adv::setDoubleWidth(true); //Useful for consoles that lack 1:1 aspect ratio fonts, not very useful for text
   adv::setThreadState(false); //Pause the drawing thread, good for when you would prefer to call the draw function yourself
   
   int key = 0;
   int px = adv::getOffsetX(0.5f, 0), py = adv::getOffsetY(0.5f, 0); //Cursor starts in the center of the console window
   
   do {
    switch (key) {
      case 'w': py--; break;
      case 'a': px--; break;
      case 's': py++; break;
      case 'd': px++; break;
    }
    
    adv::clear();
    adv::circle(px, py, 5, ' ', BWHITE | FBLACK);
    adv::draw();
   
   } while ((key = console::readKey()) != VK_ESCAPE && key != 'q');
   
   return 0;
}
```

I am working on a console UI engine. I've been reinventing the wheel for sure. I've had a program in mind for years and I am only building piece by piece.

Btw it just werks
