MoonSVG
=======

> **Fork Notice:** This is a fork of [NanoSVG](https://github.com/memononen/nanosvg) by Mikko Mononen.  
> The original library has been renamed to MoonSVG and its symbols have been prefixed with `msvg` / `MSVG` to avoid collisions when used alongside the upstream version.  
> All original copyright notices and the zlib license have been preserved.


## Parser

![screenshot of some splines rendered with the sample program](/example/screenshot-1.png?raw=true)

MoonSVG is a simple stupid single-header-file SVG parse. The output of the parser is a list of cubic bezier shapes.

The library suits well for anything from rendering scalable icons in your editor application to prototyping a game.

MoonSVG supports a wide range of SVG features, but something may be missing, feel free to create a pull request!

The shapes in the SVG images are transformed by the viewBox and converted to specified units.
That is, you should get the same looking data as your designed in your favorite app.

MoonSVG can return the paths in few different units. For example if you want to render an image, you may choose
to get the paths in pixels, or if you are feeding the data into a CNC-cutter, you may want to use millimeters. 

The units passed to MoonSVG should be one of: 'px', 'pt', 'pc' 'mm', 'cm', or 'in'.
DPI (dots-per-inch) controls how the unit conversion is done.

If you don't know or care about the units stuff, "px" and 96 should get you going.

## Rasterizer

![screenshot of tiger.svg rendered with MoonSVG rasterizer](/example/screenshot-2.png?raw=true)

The parser library is accompanied with really simpler SVG rasterizer. Currently it only renders flat filled shapes.

The intended usage for the rasterizer is to for example bake icons of different size into a texture. The rasterizer is not particular fast or accurate, but it's small and packed in one header file.


## Example Usage

``` C
// Load
	struct MSVGimage* image;
	image = msvgParseFromFile("test.svg", "px", 96);
printf("size: %f x %f\n", image->width, image->height);
// Use...
for (shape = image->shapes; shape != NULL; shape = shape->next) {
	for (path = shape->paths; path != NULL; path = path->next) {
		for (i = 0; i < path->npts-1; i += 3) {
			float* p = &path->pts[i*2];
			drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
		}
	}
}
// Delete
msvgDelete(image);
```

## Using MoonSVG in your project

In order to use MoonSVG in your own project, just copy moonsvg.h to your project.
In one C/C++ define `MOONSVG_IMPLEMENTATION` before including the library to expand the MoonSVG implementation in that file.
MoonSVG depends on `stdio.h` ,`string.h` and `math.h`, they should be included where the implementation is expanded before including MoonSVG. 

``` C
#include <stdio.h>
#include <string.h>
#include <math.h>
#define MOONSVG_IMPLEMENTATION	// Expands implementation
#include "moonsvg.h"
```

By default, MoonSVG parses only the most common colors. In order to get support for full list of [SVG color keywords](http://www.w3.org/TR/SVG11/types.html#ColorKeywords), define `MOONSVG_ALL_COLOR_KEYWORDS` before expanding the implementation.

``` C
#include <stdio.h>
#include <string.h>
#include <math.h>
#define MOONSVG_ALL_COLOR_KEYWORDS	// Include full list of color keywords.
#define MOONSVG_IMPLEMENTATION		// Expands implementation
#include "moonsvg.h"
```

By default, MoonSVG parses only the most common colors. In order to get support for full list of [SVG color keywords](http://www.w3.org/TR/SVG11/types.html#ColorKeywords), define `MOONSVG_ALL_COLOR_KEYWORDS` before expanding the implementation.

``` C
#include <stdio.h>
#include <string.h>
#include <math.h>
#define MOONSVG_ALL_COLOR_KEYWORDS	// Include full list of color keywords.
#define MOONSVG_IMPLEMENTATION		// Expands implementation
#include "moonsvg.h"
```

Alternatively, you can install the library using CMake and import it into your project using the standard CMake `find_package` command.

```CMake
add_executable(myexe main.c)

find_package(MoonSVG REQUIRED)

target_link_libraries(myexe MoonSVG::moonsvg MoonSVG::moonsvgrast)
```

## Compiling Example Project

In order to compile the demo project, your will need to install [GLFW](http://www.glfw.org/) to compile.

MoonSVG demo project uses [premake4](http://industriousone.com/premake) to build platform specific projects, now is good time to install it if you don't have it already. To build the example, navigate into the root folder in your favorite terminal, then:

- *OS X*: `premake4 xcode4`
- *Windows*: `premake4 vs2010`
- *Linux*: `premake4 gmake`

See premake4 documentation for full list of supported build file types. The projects will be created in `build` folder. An example of building and running the example on OS X:

```bash
$ premake4 gmake
$ cd build/
$ make
$ ./example
```

# License

The library is licensed under [zlib license](LICENSE.txt)
