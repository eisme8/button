//g++ -o x11buttons x11buttons.cpp -lX11 && ./x11buttons

// A simple X11 program that creates buttons that send keyboard events to a website game
// Compile with: g++ -o x11buttons x11buttons.cpp -lX11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <iostream>
#include <string>
#include <vector>

// A struct to store the button properties
struct Button {
  int x; // The x coordinate of the top-left corner
  int y; // The y coordinate of the top-left corner
  int width; // The width of the button
  int height; // The height of the button
  std::string label; // The label of the button
  std::vector<KeySym> keys; // The keys to send when the button is pressed
};

// A function to create a window and return its ID
Window createWindow(Display* display, int width, int height) {
  // Get the default screen and root window
  int screen = DefaultScreen(display);
  Window root = DefaultRootWindow(display);

  // Create a window with the given size and position at the center of the screen
  int x = (DisplayWidth(display, screen) - width) / 2;
  int y = (DisplayHeight(display, screen) - height) / 2;
  Window window = XCreateSimpleWindow(display, root, x, y, width, height, 0,
                                      BlackPixel(display, screen), WhitePixel(display, screen));

  // Set the window title and enable closing it with the delete key
  XStoreName(display, window, "X11 Buttons");
  Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wm_delete_window, 1);

  // Map the window to the display and return its ID
  XMapWindow(display, window);
  return window;
}

// A function to create a button and return its ID
Window createButton(Display* display, Window parent, Button button) {
  // Get the default screen and graphics context
  int screen = DefaultScreen(display);
  GC gc = DefaultGC(display, screen);

  // Create a child window with the button properties
  Window window = XCreateSimpleWindow(display, parent, button.x, button.y,
                                      button.width, button.height, 0,
                                      BlackPixel(display, screen), WhitePixel(display, screen));

  // Draw the button label with black text on white background
{ XCharStruct overall;
XTextExtents(XQueryFont(display, XGContextFromGC(gc)),
             button.label.c_str(),
             button.label.length(),
             NULL, NULL, NULL, &overall);
int ascent = overall.ascent;}
  // Map the window to the display and return its ID
  XMapWindow(display, window);
  return window;
}

// A function to send keyboard events to a target window
void sendKeys(Display* display, Window target, std::vector<KeySym> keys) {
  // Get the default screen and root window
  int screen = DefaultScreen(display);
  Window root = DefaultRootWindow(display);

  // Create a fake keyboard event structure
  XKeyEvent event;
  event.display = display;
  event.window = target;
  event.root = root;
  event.subwindow = None;
  event.time = CurrentTime;
  event.x = event.y = event.x_root = event.y_root = 0;
  // Loop through the keys vector and send a press and release event for each key
  for (KeySym key : keys) {
    // Convert the key symbol to a keycode
    event.keycode = XKeysymToKeycode(display, key);

    // Send a press event
    event.type = KeyPress;
    XSendEvent(display, target, True, KeyPressMask, (XEvent*)&event);

    // Send a release event
    event.type = KeyRelease;
    XSendEvent(display, target, True, KeyReleaseMask, (XEvent*)&event);
  }
}

// The main function
int main() {
  // Open a connection to the display
  Display* display = XOpenDisplay(NULL);
  if (display == NULL) {
    std::cerr << "Cannot open display\n";
    return -1;
  }

  // Create a vector of buttons with their properties
  std::vector<Button> buttons = {
    {10, 10, 100, 50, "S", {XK_S}},
    {120, 10, 100, 50, "D", {XK_D}},
    {230, 10, 100, 50, "A", {XK_A}},
    {10, 70, 100, 50, "H", {XK_H}},
    {120, 70, 100, 50, "L", {XK_L}},
    {230, 70, 100, 50, "Combo", {XK_L, XK_H, XK_L, XK_H, XK_L, XK_H}}
  };

  // Create a window with a fixed size of 340x130 pixels
  Window window = createWindow(display, 340, 130);

  // Create a vector of button windows and store their IDs
  std::vector<Window> buttonWindows;
  for (Button button : buttons) {
    buttonWindows.push_back(createButton(display, window, button));
  }

  // Get the ID of the target window that runs the website game
  // You may need to change this depending on how you launch the game
  Window target = XGetSelectionOwner(display,
                                     XInternAtom(display,
                                                 "_NET_ACTIVE_WINDOW",
                                                 False)); // Add this

  // Enter the event loop
  XEvent event;
  while (true) {
    // Wait for the next event
    XNextEvent(display, &event);

    // If the event is a mouse button press
    if (event.type == ButtonPress) {
      // Loop through the button windows and check if the event window matches any of them
      for (int i = 0; i < buttonWindows.size(); i++) {
        if (event.xbutton.window == buttonWindows[i]) {
          // If yes, send the corresponding keys to the target window
          sendKeys(display, target, buttons[i].keys);
          break;
        }
      }
    }

    // If the event is a client message and matches the delete window atom
    else if (event.type == ClientMessage &&
           (long)event.xclient.data.l[0] == (long)XInternAtom(display,
                                                   "WM_DELETE_WINDOW",
                                                   False))
      // If yes, break out of the loop and exit the program
      break;
    }
  }

{ // Close the connection to the display and return
 XCloseDisplay(display)
  return 0;
}
  
