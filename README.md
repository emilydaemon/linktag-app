# LinkTag App

TODO: write a description

## Contributing

Contributions are welcome! However, there are some things you should take note of:
- Use the text drawing functions (`draw_text()`, `draw_center_text()`, `draw_title()`, etc) BEFORE drawing ANYTHING ELSE. After you're done drawing everything, call `render_text()`. This will actually draw all the text on the screen.
  - Text is drawn onto a separate buffer, so that we can later resize it for widescreen support. It's not pretty, but it works.
- Please use snake_case for naming variables and functions.
- Please use tabs instead of spaces for indentation.
- Please use the One True Brace indentation style.
  - Example:
```c
int main() {
    for (i = 0; i < 10; i++) {
        printf("%d", i);
    }
    return 1;
}
```

Looking for something to do? To-do can be found here: https://github.com/users/emilydaemon/projects/1

### Compiling

First, install all the required dependencies:

```
git clone https://github.com/GRRLIB/GRRLIB
pacman --sync --needed --noconfirm libfat-ogc ppc-libpng ppc-freetype ppc-libjpeg-turbo
cd GRRLIB/GRRLIB
make clean all install

pacman --sync --needed --noconfirm ppc-jansson wii-winyl
```

Then, compile.

```
git clone https://github.com/emilydaemon/linktag-app
cd linktag-app
make
```

The makefile supports many arguments.

|Argument|Description|
|-|-|
|build|Default argument. Compiles the app.|
|run|Runs the app on your Wii using Wiiload.|
|package|Creates a .zip with boot.dol, icon.png, and meta.xml.|
|package-run|Transfers the packaged .zip to your Wii using Wiiload.|
|clean|Clean up build files.|

A couple examples:

```
make build run              # Compile, run binary on Wii
make package package-run    # Build package, transfer to Wii
```
