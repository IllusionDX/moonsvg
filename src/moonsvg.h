/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * The SVG parser is based on Anti-Grain Geometry 2.4 SVG example
 * Copyright (C) 2002-2004 Maxim Shemanarev (McSeem) (http://www.antigrain.com/)
 *
 * Arc calculation code based on canvg (https://code.google.com/p/canvg/)
 *
 * Bounding box calculation based on http://blog.hackers-cafe.net/2009/06/how-to-calculate-bezier-curves-bounding.html
 *
 */

#ifndef MOONSVG_H
#define MOONSVG_H

#ifndef MOONSVG_CPLUSPLUS
#ifdef __cplusplus
extern "C" {
#endif
#endif

// MoonSVG is a simple stupid single-header-file SVG parse. The output of the parser is a list of cubic bezier shapes.
//
// The library suits well for anything from rendering scalable icons in your editor application to prototyping a game.
//
// MoonSVG supports a wide range of SVG features, but something may be missing, feel free to create a pull request!
//
// The shapes in the SVG images are transformed by the viewBox and converted to specified units.
// That is, you should get the same looking data as your designed in your favorite app.
//
// MoonSVG can return the paths in few different units. For example if you want to render an image, you may choose
// to get the paths in pixels, or if you are feeding the data into a CNC-cutter, you may want to use millimeters.
//
// The units passed to MoonSVG should be one of: 'px', 'pt', 'pc' 'mm', 'cm', or 'in'.
// DPI (dots-per-inch) controls how the unit conversion is done.
//
// If you don't know or care about the units stuff, "px" and 96 should get you going.


/* Example Usage:
	// Load SVG
	MSVGimage* image;
	image = msvgParseFromFile("test.svg", "px", 96);
	printf("size: %f x %f\n", image->width, image->height);
	// Use...
	for (MSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
		for (MSVGpath *path = shape->paths; path != NULL; path = path->next) {
			for (int i = 0; i < path->npts-1; i += 3) {
				float* p = &path->pts[i*2];
				drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
			}
		}
	}
	// Delete
	msvgDelete(image);
*/

enum MSVGpaintType {
	MSVG_PAINT_UNDEF = -1,
	MSVG_PAINT_NONE = 0,
	MSVG_PAINT_COLOR = 1,
	MSVG_PAINT_LINEAR_GRADIENT = 2,
	MSVG_PAINT_RADIAL_GRADIENT = 3
};

enum MSVGspreadType {
	MSVG_SPREAD_PAD = 0,
	MSVG_SPREAD_REFLECT = 1,
	MSVG_SPREAD_REPEAT = 2
};

enum MSVGlineJoin {
	MSVG_JOIN_MITER = 0,
	MSVG_JOIN_ROUND = 1,
	MSVG_JOIN_BEVEL = 2
};

enum MSVGlineCap {
	MSVG_CAP_BUTT = 0,
	MSVG_CAP_ROUND = 1,
	MSVG_CAP_SQUARE = 2
};

enum MSVGfillRule {
	MSVG_FILLRULE_NONZERO = 0,
	MSVG_FILLRULE_EVENODD = 1
};

enum MSVGflags {
	MSVG_FLAGS_VISIBLE = 0x01
};

enum MSVGpaintOrder {
	MSVG_PAINT_FILL = 0x00,
	MSVG_PAINT_MARKERS = 0x01,
	MSVG_PAINT_STROKE = 0x02,
};

typedef struct MSVGgradientStop {
	unsigned int color;
	float offset;
} MSVGgradientStop;

typedef struct MSVGgradient {
	float xform[6];
	char spread;
	float fx, fy;
	int nstops;
	MSVGgradientStop stops[1];
} MSVGgradient;

typedef struct MSVGpaint {
	signed char type;
	union {
		unsigned int color;
		MSVGgradient* gradient;
	};
} MSVGpaint;

typedef struct MSVGpath
{
	float* pts;					// Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
	int npts;					// Total number of bezier points.
	char closed;				// Flag indicating if shapes should be treated as closed.
	float bounds[4];			// Tight bounding box of the shape [minx,miny,maxx,maxy].
	struct MSVGpath* next;		// Pointer to next path, or NULL if last element.
} MSVGpath;

#define MSVG__PARSER_MEMPAGE_SIZE 8192
typedef struct MSVGparserPage {
    unsigned char* mem;
    int size;
    int capacity;
    struct MSVGparserPage* next;
} MSVGparserPage;

typedef struct MSVGshape
{
	char id[64];				// Optional 'id' attr of the shape or its group
	MSVGpaint fill;				// Fill paint
	MSVGpaint stroke;			// Stroke paint
	float opacity;				// Opacity of the shape.
	float strokeWidth;			// Stroke width (scaled).
	float strokeDashOffset;		// Stroke dash offset (scaled).
	float strokeDashArray[8];	// Stroke dash array (scaled).
	char strokeDashCount;		// Number of dash values in dash array.
	char strokeLineJoin;		// Stroke join type.
	char strokeLineCap;			// Stroke cap type.
	float miterLimit;			// Miter limit
	char fillRule;				// Fill rule, see MSVGfillRule.
    unsigned char paintOrder;	// Encoded paint order (3×2-bit fields) see MSVGpaintOrder
	unsigned char flags;		// Logical or of MSVG_FLAGS_* flags
	float bounds[4];			// Tight bounding box of the shape [minx,miny,maxx,maxy].
	char fillGradient[64];		// Optional 'id' of fill gradient
	char strokeGradient[64];	// Optional 'id' of stroke gradient
	float xform[6];				// Root transformation for fill/stroke gradient
	MSVGpath* paths;			// Linked list of paths in the image.
	struct MSVGshape* next;		// Pointer to next shape, or NULL if last element.
} MSVGshape;

typedef struct MSVGsymbol
{
	char id[64];				// Symbol identifier
	MSVGshape* shapes;			// Linked list of shapes inside the symbol
	struct MSVGsymbol* next;	// Next symbol in registry
} MSVGsymbol;

typedef struct MSVGstyle
{
	char name[64];				// Class name (without the dot)
	char* properties;			// Pointer to property string inside original buffer
	struct MSVGstyle* next;		// Next style in registry
} MSVGstyle;

typedef struct MSVGimage
{
	float width;				// Width of the image.
	float height;				// Height of the image.
	MSVGshape* shapes;			// Linked list of shapes in the image.
	MSVGparserPage* pages;		// Memory pool pages for parsed data.
	MSVGparserPage* curpage;	// Current pool page in use.
} MSVGimage;

// Parses SVG file from a file, returns SVG image as paths.
MSVGimage* msvgParseFromFile(const char* filename, const char* units, float dpi);

// Parses SVG file from a null terminated string, returns SVG image as paths.
// Important note: changes the string.
MSVGimage* msvgParse(char* input, const char* units, float dpi);

// Duplicates a path.
MSVGpath* msvgDuplicatePath(MSVGpath* p);

// Deletes an image.
void msvgDelete(MSVGimage* image);

#ifndef MOONSVG_CPLUSPLUS
#ifdef __cplusplus
}
#endif
#endif

#ifdef MOONSVG_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MSVG_PI (3.14159265358979323846264338327f)
#define MSVG_KAPPA90 (0.5522847493f)	// Length proportional to radius of a cubic bezier handle for 90deg arcs.

#define MSVG_ALIGN_MIN 0
#define MSVG_ALIGN_MID 1
#define MSVG_ALIGN_MAX 2
#define MSVG_ALIGN_NONE 0
#define MSVG_ALIGN_MEET 1
#define MSVG_ALIGN_SLICE 2

#define MSVG_NOTUSED(v) do { (void)(1 ? (void)0 : ( (void)(v) ) ); } while(0)
#define MSVG_RGB(r, g, b) (((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16))

#ifdef _MSC_VER
	#pragma warning (disable: 4996) // Switch off security warnings
	#pragma warning (disable: 4100) // Switch off unreferenced formal parameter warnings
	#ifdef __cplusplus
	#define MSVG_INLINE inline
	#else
	#define MSVG_INLINE
	#endif
#else
	#define MSVG_INLINE inline
#endif


static int msvg__isspace(char c)
{
	return strchr(" \t\n\v\f\r", c) != 0;
}

static int msvg__isdigit(char c)
{
	return c >= '0' && c <= '9';
}

static MSVG_INLINE float msvg__minf(float a, float b) { return a < b ? a : b; }
static MSVG_INLINE float msvg__maxf(float a, float b) { return a > b ? a : b; }


// Simple XML parser

#define MSVG_XML_TAG 1
#define MSVG_XML_CONTENT 2
#define MSVG_XML_MAX_ATTRIBS 256

static void msvg__parseContent(char* s,
							   void (*contentCb)(void* ud, const char* s),
							   void* ud)
{
	// Trim start white spaces
	while (*s && msvg__isspace(*s)) s++;
	if (!*s) return;

	if (contentCb)
		(*contentCb)(ud, s);
}

static void msvg__parseElement(char* s,
							   void (*startelCb)(void* ud, const char* el, const char** attr),
							   void (*endelCb)(void* ud, const char* el),
							   void* ud)
{
	const char* attr[MSVG_XML_MAX_ATTRIBS];
	int nattr = 0;
	char* name;
	int start = 0;
	int end = 0;
	char quote;

	// Skip white space after the '<'
	while (*s && msvg__isspace(*s)) s++;

	// Check if the tag is end tag
	if (*s == '/') {
		s++;
		end = 1;
	} else {
		start = 1;
	}

	// Skip comments, data and preprocessor stuff.
	if (!*s || *s == '?' || *s == '!')
		return;

	// Get tag name
	name = s;
	while (*s && !msvg__isspace(*s)) s++;
	if (*s) { *s++ = '\0'; }

	// Get attribs
	while (!end && *s && nattr < MSVG_XML_MAX_ATTRIBS-3) {
		char* name = NULL;
		char* value = NULL;

		// Skip white space before the attrib name
		while (*s && msvg__isspace(*s)) s++;
		if (!*s) break;
		if (*s == '/') {
			end = 1;
			break;
		}
		name = s;
		// Find end of the attrib name.
		while (*s && !msvg__isspace(*s) && *s != '=') s++;
		if (*s) { *s++ = '\0'; }
		// Skip until the beginning of the value.
		while (*s && *s != '\"' && *s != '\'') s++;
		if (!*s) break;
		quote = *s;
		s++;
		// Store value and find the end of it.
		value = s;
		while (*s && *s != quote) s++;
		if (*s) { *s++ = '\0'; }

		// Store only well formed attributes
		if (name && value) {
			attr[nattr++] = name;
			attr[nattr++] = value;
		}
	}

	// List terminator
	attr[nattr++] = 0;
	attr[nattr++] = 0;

	// Call callbacks.
	if (start && startelCb)
		(*startelCb)(ud, name, attr);
	if (end && endelCb)
		(*endelCb)(ud, name);
}

int msvg__parseXML(char* input,
				   void (*startelCb)(void* ud, const char* el, const char** attr),
				   void (*endelCb)(void* ud, const char* el),
				   void (*contentCb)(void* ud, const char* s),
				   void* ud)
{
	char* s = input;
	char* mark = s;
	int state = MSVG_XML_CONTENT;
	while (*s) {
		if (*s == '<' && state == MSVG_XML_CONTENT) {
			// Start of a tag
			*s++ = '\0';
			msvg__parseContent(mark, contentCb, ud);
			mark = s;
			state = MSVG_XML_TAG;
		} else if (*s == '>' && state == MSVG_XML_TAG) {
			// Start of a content or new tag.
			*s++ = '\0';
			msvg__parseElement(mark, startelCb, endelCb, ud);
			mark = s;
			state = MSVG_XML_CONTENT;
		} else {
			s++;
		}
	}

	return 1;
}


/* Simple SVG parser. */

#define MSVG_MAX_ATTR 128

enum MSVGgradientUnits {
	MSVG_USER_SPACE = 0,
	MSVG_OBJECT_SPACE = 1
};

#define MSVG_MAX_DASHES 8

enum MSVGunits {
	MSVG_UNITS_USER,
	MSVG_UNITS_PX,
	MSVG_UNITS_PT,
	MSVG_UNITS_PC,
	MSVG_UNITS_MM,
	MSVG_UNITS_CM,
	MSVG_UNITS_IN,
	MSVG_UNITS_PERCENT,
	MSVG_UNITS_EM,
	MSVG_UNITS_EX
};

typedef struct MSVGcoordinate {
	float value;
	int units;
} MSVGcoordinate;

typedef struct MSVGlinearData {
	MSVGcoordinate x1, y1, x2, y2;
} MSVGlinearData;

typedef struct MSVGradialData {
	MSVGcoordinate cx, cy, r, fx, fy;
} MSVGradialData;

typedef struct MSVGgradientData
{
	char id[64];
	char ref[64];
	signed char type;
	union {
		MSVGlinearData linear;
		MSVGradialData radial;
	};
	char spread;
	char units;
	float xform[6];
	int nstops;
	int stopCapacity;
	MSVGgradientStop* stops;
	struct MSVGgradientData* next;
} MSVGgradientData;

typedef struct MSVGattrib
{
	char id[64];
	float xform[6];
	unsigned int fillColor;
	unsigned int strokeColor;
	float opacity;
	float fillOpacity;
	float strokeOpacity;
	char fillGradient[64];
	char strokeGradient[64];
	float strokeWidth;
	float strokeDashOffset;
	float strokeDashArray[MSVG_MAX_DASHES];
	int strokeDashCount;
	char strokeLineJoin;
	char strokeLineCap;
	float miterLimit;
	char fillRule;
	float fontSize;
	unsigned int stopColor;
	float stopOpacity;
	float stopOffset;
	char hasFill;
	char hasStroke;
	char visible;
    unsigned char paintOrder;
} MSVGattrib;

typedef struct MSVGparser
{
	MSVGattrib attr[MSVG_MAX_ATTR];
	int attrHead;
	float* pts;
	int npts;
	int cpts;
	MSVGpath* plist;
	MSVGimage* image;
	MSVGgradientData* gradients;
	MSVGshape* shapesTail;
	MSVGsymbol* symbols;		// Symbol registry for <defs>/<symbol>
	MSVGsymbol* curSymbol;		// Currently open symbol during parsing
	MSVGshape* symbolTail;		// Tail of shapes in current symbol
	MSVGstyle* styles;			// CSS class style registry
	int styleFlag;				// Inside <style> block
	float viewMinx, viewMiny, viewWidth, viewHeight;
	int alignX, alignY, alignType;
	float dpi;
	char pathFlag;
	char defsFlag;
} MSVGparser;

static void msvg__xformIdentity(float* t)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void msvg__xformSetTranslation(float* t, float tx, float ty)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = tx; t[5] = ty;
}

static void msvg__xformSetScale(float* t, float sx, float sy)
{
	t[0] = sx; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = sy;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void msvg__xformSetSkewX(float* t, float a)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = tanf(a); t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void msvg__xformSetSkewY(float* t, float a)
{
	t[0] = 1.0f; t[1] = tanf(a);
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void msvg__xformSetRotation(float* t, float a)
{
	float cs = cosf(a), sn = sinf(a);
	t[0] = cs; t[1] = sn;
	t[2] = -sn; t[3] = cs;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void msvg__xformMultiply(float* t, float* s)
{
	float t0 = t[0] * s[0] + t[1] * s[2];
	float t2 = t[2] * s[0] + t[3] * s[2];
	float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
	t[1] = t[0] * s[1] + t[1] * s[3];
	t[3] = t[2] * s[1] + t[3] * s[3];
	t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
	t[0] = t0;
	t[2] = t2;
	t[4] = t4;
}

static void msvg__xformInverse(float* inv, float* t)
{
	double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
	if (det > -1e-6 && det < 1e-6) {
		msvg__xformIdentity(t);
		return;
	}
	invdet = 1.0 / det;
	inv[0] = (float)(t[3] * invdet);
	inv[2] = (float)(-t[2] * invdet);
	inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
	inv[1] = (float)(-t[1] * invdet);
	inv[3] = (float)(t[0] * invdet);
	inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
}

static void msvg__xformPremultiply(float* t, float* s)
{
	float s2[6];
	memcpy(s2, s, sizeof(float)*6);
	msvg__xformMultiply(s2, t);
	memcpy(t, s2, sizeof(float)*6);
}

static void msvg__xformPoint(float* dx, float* dy, float x, float y, float* t)
{
	*dx = x*t[0] + y*t[2] + t[4];
	*dy = x*t[1] + y*t[3] + t[5];
}

static void msvg__xformVec(float* dx, float* dy, float x, float y, float* t)
{
	*dx = x*t[0] + y*t[2];
	*dy = x*t[1] + y*t[3];
}

#define MSVG_EPSILON (1e-12)

static int msvg__ptInBounds(float* pt, float* bounds)
{
	return pt[0] >= bounds[0] && pt[0] <= bounds[2] && pt[1] >= bounds[1] && pt[1] <= bounds[3];
}


static double msvg__evalBezier(double t, double p0, double p1, double p2, double p3)
{
	double it = 1.0-t;
	return it*it*it*p0 + 3.0*it*it*t*p1 + 3.0*it*t*t*p2 + t*t*t*p3;
}

static void msvg__curveBounds(float* bounds, float* curve)
{
	int i, j, count;
	double roots[2], a, b, c, b2ac, t, v;
	float* v0 = &curve[0];
	float* v1 = &curve[2];
	float* v2 = &curve[4];
	float* v3 = &curve[6];

	// Start the bounding box by end points
	bounds[0] = msvg__minf(v0[0], v3[0]);
	bounds[1] = msvg__minf(v0[1], v3[1]);
	bounds[2] = msvg__maxf(v0[0], v3[0]);
	bounds[3] = msvg__maxf(v0[1], v3[1]);

	// Bezier curve fits inside the convex hull of it's control points.
	// If control points are inside the bounds, we're done.
	if (msvg__ptInBounds(v1, bounds) && msvg__ptInBounds(v2, bounds))
		return;

	// Add bezier curve inflection points in X and Y.
	for (i = 0; i < 2; i++) {
		a = -3.0 * v0[i] + 9.0 * v1[i] - 9.0 * v2[i] + 3.0 * v3[i];
		b = 6.0 * v0[i] - 12.0 * v1[i] + 6.0 * v2[i];
		c = 3.0 * v1[i] - 3.0 * v0[i];
		count = 0;
		if (fabs(a) < MSVG_EPSILON) {
			if (fabs(b) > MSVG_EPSILON) {
				t = -c / b;
				if (t > MSVG_EPSILON && t < 1.0-MSVG_EPSILON)
					roots[count++] = t;
			}
		} else {
			b2ac = b*b - 4.0*c*a;
			if (b2ac > MSVG_EPSILON) {
				t = (-b + sqrt(b2ac)) / (2.0 * a);
				if (t > MSVG_EPSILON && t < 1.0-MSVG_EPSILON)
					roots[count++] = t;
				t = (-b - sqrt(b2ac)) / (2.0 * a);
				if (t > MSVG_EPSILON && t < 1.0-MSVG_EPSILON)
					roots[count++] = t;
			}
		}
		for (j = 0; j < count; j++) {
			v = msvg__evalBezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
			bounds[0+i] = msvg__minf(bounds[0+i], (float)v);
			bounds[2+i] = msvg__maxf(bounds[2+i], (float)v);
		}
	}
}

static unsigned char msvg__encodePaintOrder(enum MSVGpaintOrder a, enum MSVGpaintOrder b, enum MSVGpaintOrder c) {
    return (a & 0x03) | ((b & 0x03) << 2) | ((c & 0x03) << 4);
}

static MSVGparserPage* msvg__nextParserPage(MSVGimage* img, MSVGparserPage* cur, int capacity)
{
    MSVGparserPage* newp;
    if (cur != NULL && cur->next != NULL)
        return cur->next;
    newp = (MSVGparserPage*)malloc(sizeof(MSVGparserPage));
    if (newp == NULL) return NULL;
    memset(newp, 0, sizeof(MSVGparserPage));
    newp->mem = (unsigned char*)malloc(capacity);
    if (newp->mem == NULL) {
        free(newp);
        return NULL;
    }
    newp->capacity = capacity;
    if (cur != NULL)
        cur->next = newp;
    else
        img->pages = newp;
    return newp;
}

static void* msvg__parserAlloc(MSVGimage* img, int size)
{
    unsigned char* buf;
    size = (size + 7) & ~7;
    if (size > MSVG__PARSER_MEMPAGE_SIZE) {
        // Large allocation: create a dedicated page with exact capacity.
        MSVGparserPage* largePage = msvg__nextParserPage(img, img->curpage, size);
        if (largePage == NULL) return NULL;
        largePage->size = size;
        if (img->curpage != NULL && img->curpage->next == largePage)
            img->curpage = largePage;
        return largePage->mem;
    }
    if (img->curpage == NULL || img->curpage->size + size > MSVG__PARSER_MEMPAGE_SIZE) {
        img->curpage = msvg__nextParserPage(img, img->curpage, MSVG__PARSER_MEMPAGE_SIZE);
        if (img->curpage == NULL) return NULL;
    }
    buf = &img->curpage->mem[img->curpage->size];
    img->curpage->size += size;
    return buf;
}

static MSVGparser* msvg__createParser(void)
{
	MSVGparser* p;
	p = (MSVGparser*)malloc(sizeof(MSVGparser));
	if (p == NULL) goto error;
	memset(p, 0, sizeof(MSVGparser));

	p->image = (MSVGimage*)malloc(sizeof(MSVGimage));
	if (p->image == NULL) goto error;
	memset(p->image, 0, sizeof(MSVGimage));

	// Init style
	msvg__xformIdentity(p->attr[0].xform);
	memset(p->attr[0].id, 0, sizeof p->attr[0].id);
	p->attr[0].fillColor = MSVG_RGB(0,0,0);
	p->attr[0].strokeColor = MSVG_RGB(0,0,0);
	p->attr[0].opacity = 1;
	p->attr[0].fillOpacity = 1;
	p->attr[0].strokeOpacity = 1;
	p->attr[0].stopOpacity = 1;
	p->attr[0].strokeWidth = 1;
	p->attr[0].strokeLineJoin = MSVG_JOIN_MITER;
	p->attr[0].strokeLineCap = MSVG_CAP_BUTT;
	p->attr[0].miterLimit = 4;
	p->attr[0].fillRule = MSVG_FILLRULE_NONZERO;
	p->attr[0].hasFill = 1;
	p->attr[0].visible = 1;
    p->attr[0].paintOrder = msvg__encodePaintOrder(MSVG_PAINT_FILL, MSVG_PAINT_STROKE, MSVG_PAINT_MARKERS);

	return p;

error:
	if (p) {
		if (p->image) free(p->image);
		free(p);
	}
	return NULL;
}

static void msvg__deletePaths(MSVGpath* path)
{
    (void)path;
    // Paths are now pool-allocated; no individual cleanup needed.
}

static void msvg__deletePaint(MSVGpaint* paint)
{
	(void)paint;
	// Gradient is now pool-allocated; no individual cleanup needed.
}

static void msvg__deleteGradientData(MSVGgradientData* grad)
{
	MSVGgradientData* next;
	while (grad != NULL) {
		next = grad->next;
		// grad and grad->stops are pool-allocated; no individual free needed.
		grad = next;
	}
}

static void msvg__deleteParser(MSVGparser* p)
{
	if (p != NULL) {
		msvg__deleteGradientData(p->gradients);
		msvgDelete(p->image);
		free(p->pts);
		free(p);
	}
}

static void msvg__resetPath(MSVGparser* p)
{
	p->npts = 0;
}

static void msvg__addPoint(MSVGparser* p, float x, float y)
{
	if (p->npts+1 > p->cpts) {
		p->cpts = p->cpts ? p->cpts*2 : 8;
		p->pts = (float*)realloc(p->pts, p->cpts*2*sizeof(float));
		if (!p->pts) return;
	}
	p->pts[p->npts*2+0] = x;
	p->pts[p->npts*2+1] = y;
	p->npts++;
}

static void msvg__moveTo(MSVGparser* p, float x, float y)
{
	if (p->npts > 0) {
		p->pts[(p->npts-1)*2+0] = x;
		p->pts[(p->npts-1)*2+1] = y;
	} else {
		msvg__addPoint(p, x, y);
	}
}

static void msvg__lineTo(MSVGparser* p, float x, float y)
{
	float px,py, dx,dy;
	if (p->npts > 0) {
		px = p->pts[(p->npts-1)*2+0];
		py = p->pts[(p->npts-1)*2+1];
		dx = x - px;
		dy = y - py;
		msvg__addPoint(p, px + dx/3.0f, py + dy/3.0f);
		msvg__addPoint(p, x - dx/3.0f, y - dy/3.0f);
		msvg__addPoint(p, x, y);
	}
}

static void msvg__cubicBezTo(MSVGparser* p, float cpx1, float cpy1, float cpx2, float cpy2, float x, float y)
{
	if (p->npts > 0) {
		msvg__addPoint(p, cpx1, cpy1);
		msvg__addPoint(p, cpx2, cpy2);
		msvg__addPoint(p, x, y);
	}
}

static MSVGattrib* msvg__getAttr(MSVGparser* p)
{
	return &p->attr[p->attrHead];
}

static void msvg__pushAttr(MSVGparser* p)
{
	if (p->attrHead < MSVG_MAX_ATTR-1) {
		p->attrHead++;
		memcpy(&p->attr[p->attrHead], &p->attr[p->attrHead-1], sizeof(MSVGattrib));
	}
}

static void msvg__popAttr(MSVGparser* p)
{
	if (p->attrHead > 0)
		p->attrHead--;
}

static float msvg__actualOrigX(MSVGparser* p)
{
	return p->viewMinx;
}

static float msvg__actualOrigY(MSVGparser* p)
{
	return p->viewMiny;
}

static float msvg__actualWidth(MSVGparser* p)
{
	return p->viewWidth;
}

static float msvg__actualHeight(MSVGparser* p)
{
	return p->viewHeight;
}

static float msvg__actualLength(MSVGparser* p)
{
	float w = msvg__actualWidth(p), h = msvg__actualHeight(p);
	return sqrtf(w*w + h*h) / sqrtf(2.0f);
}

static float msvg__convertToPixels(MSVGparser* p, MSVGcoordinate c, float orig, float length)
{
	MSVGattrib* attr = msvg__getAttr(p);
	switch (c.units) {
		case MSVG_UNITS_USER:		return c.value;
		case MSVG_UNITS_PX:			return c.value;
		case MSVG_UNITS_PT:			return c.value / 72.0f * p->dpi;
		case MSVG_UNITS_PC:			return c.value / 6.0f * p->dpi;
		case MSVG_UNITS_MM:			return c.value / 25.4f * p->dpi;
		case MSVG_UNITS_CM:			return c.value / 2.54f * p->dpi;
		case MSVG_UNITS_IN:			return c.value * p->dpi;
		case MSVG_UNITS_EM:			return c.value * attr->fontSize;
		case MSVG_UNITS_EX:			return c.value * attr->fontSize * 0.52f; // x-height of Helvetica.
		case MSVG_UNITS_PERCENT:	return orig + c.value / 100.0f * length;
		default:					return c.value;
	}
	return c.value;
}

static MSVGgradientData* msvg__findGradientData(MSVGparser* p, const char* id)
{
	MSVGgradientData* grad = p->gradients;
	if (id == NULL || *id == '\0')
		return NULL;
	while (grad != NULL) {
		if (strcmp(grad->id, id) == 0)
			return grad;
		grad = grad->next;
	}
	return NULL;
}

static MSVGgradient* msvg__createGradient(MSVGparser* p, const char* id, const float* localBounds, float *xform, signed char* paintType)
{
	MSVGgradientData* data = NULL;
	MSVGgradientData* ref = NULL;
	MSVGgradientStop* stops = NULL;
	MSVGgradient* grad;
	float ox, oy, sw, sh, sl;
	int nstops = 0;
	int refIter;

	data = msvg__findGradientData(p, id);
	if (data == NULL) return NULL;

	// TODO: use ref to fill in all unset values too.
	ref = data;
	refIter = 0;
	while (ref != NULL) {
		MSVGgradientData* nextRef = NULL;
		if (stops == NULL && ref->stops != NULL) {
			stops = ref->stops;
			nstops = ref->nstops;
			break;
		}
		nextRef = msvg__findGradientData(p, ref->ref);
		if (nextRef == ref) break; // prevent infite loops on malformed data
		ref = nextRef;
		refIter++;
		if (refIter > 32) break; // prevent infite loops on malformed data
	}
	if (stops == NULL) return NULL;

	grad = (MSVGgradient*)msvg__parserAlloc(p->image, sizeof(MSVGgradient) + sizeof(MSVGgradientStop)*(nstops-1));
	if (grad == NULL) return NULL;

	// The shape width and height.
	if (data->units == MSVG_OBJECT_SPACE) {
		ox = localBounds[0];
		oy = localBounds[1];
		sw = localBounds[2] - localBounds[0];
		sh = localBounds[3] - localBounds[1];
	} else {
		ox = msvg__actualOrigX(p);
		oy = msvg__actualOrigY(p);
		sw = msvg__actualWidth(p);
		sh = msvg__actualHeight(p);
	}
	sl = sqrtf(sw*sw + sh*sh) / sqrtf(2.0f);

	if (data->type == MSVG_PAINT_LINEAR_GRADIENT) {
		float x1, y1, x2, y2, dx, dy;
		x1 = msvg__convertToPixels(p, data->linear.x1, ox, sw);
		y1 = msvg__convertToPixels(p, data->linear.y1, oy, sh);
		x2 = msvg__convertToPixels(p, data->linear.x2, ox, sw);
		y2 = msvg__convertToPixels(p, data->linear.y2, oy, sh);
		// Calculate transform aligned to the line
		dx = x2 - x1;
		dy = y2 - y1;
		grad->xform[0] = dy; grad->xform[1] = -dx;
		grad->xform[2] = dx; grad->xform[3] = dy;
		grad->xform[4] = x1; grad->xform[5] = y1;
	} else {
		float cx, cy, fx, fy, r;
		cx = msvg__convertToPixels(p, data->radial.cx, ox, sw);
		cy = msvg__convertToPixels(p, data->radial.cy, oy, sh);
		fx = msvg__convertToPixels(p, data->radial.fx, ox, sw);
		fy = msvg__convertToPixels(p, data->radial.fy, oy, sh);
		r = msvg__convertToPixels(p, data->radial.r, 0, sl);
		// Calculate transform aligned to the circle
		grad->xform[0] = r; grad->xform[1] = 0;
		grad->xform[2] = 0; grad->xform[3] = r;
		grad->xform[4] = cx; grad->xform[5] = cy;
		grad->fx = fx / r;
		grad->fy = fy / r;
	}

	msvg__xformMultiply(grad->xform, data->xform);
	msvg__xformMultiply(grad->xform, xform);

	grad->spread = data->spread;
	memcpy(grad->stops, stops, nstops*sizeof(MSVGgradientStop));
	grad->nstops = nstops;

	*paintType = data->type;

	return grad;
}

static float msvg__getAverageScale(float* t)
{
	float sx = sqrtf(t[0]*t[0] + t[2]*t[2]);
	float sy = sqrtf(t[1]*t[1] + t[3]*t[3]);
	return (sx + sy) * 0.5f;
}

static void msvg__getLocalBounds(float* bounds, MSVGshape *shape, float* xform)
{
	MSVGpath* path;
	float curve[4*2], curveBounds[4];
	int i, first = 1;
	for (path = shape->paths; path != NULL; path = path->next) {
		msvg__xformPoint(&curve[0], &curve[1], path->pts[0], path->pts[1], xform);
		for (i = 0; i < path->npts-1; i += 3) {
			msvg__xformPoint(&curve[2], &curve[3], path->pts[(i+1)*2], path->pts[(i+1)*2+1], xform);
			msvg__xformPoint(&curve[4], &curve[5], path->pts[(i+2)*2], path->pts[(i+2)*2+1], xform);
			msvg__xformPoint(&curve[6], &curve[7], path->pts[(i+3)*2], path->pts[(i+3)*2+1], xform);
			msvg__curveBounds(curveBounds, curve);
			if (first) {
				bounds[0] = curveBounds[0];
				bounds[1] = curveBounds[1];
				bounds[2] = curveBounds[2];
				bounds[3] = curveBounds[3];
				first = 0;
			} else {
				bounds[0] = msvg__minf(bounds[0], curveBounds[0]);
				bounds[1] = msvg__minf(bounds[1], curveBounds[1]);
				bounds[2] = msvg__maxf(bounds[2], curveBounds[2]);
				bounds[3] = msvg__maxf(bounds[3], curveBounds[3]);
			}
			curve[0] = curve[6];
			curve[1] = curve[7];
		}
	}
}

static void msvg__addShape(MSVGparser* p)
{
	MSVGattrib* attr = msvg__getAttr(p);
	float scale = 1.0f;
	MSVGshape* shape;
	MSVGpath* path;
	int i;

	if (p->plist == NULL)
		return;

	shape = (MSVGshape*)msvg__parserAlloc(p->image, sizeof(MSVGshape));
	if (shape == NULL) return;
	memset(shape, 0, sizeof(MSVGshape));

	memcpy(shape->id, attr->id, sizeof shape->id);
	memcpy(shape->fillGradient, attr->fillGradient, sizeof shape->fillGradient);
	memcpy(shape->strokeGradient, attr->strokeGradient, sizeof shape->strokeGradient);
	memcpy(shape->xform, attr->xform, sizeof shape->xform);
	scale = msvg__getAverageScale(attr->xform);
	shape->strokeWidth = attr->strokeWidth * scale;
	shape->strokeDashOffset = attr->strokeDashOffset * scale;
	shape->strokeDashCount = (char)attr->strokeDashCount;
	for (i = 0; i < attr->strokeDashCount; i++)
		shape->strokeDashArray[i] = attr->strokeDashArray[i] * scale;
	shape->strokeLineJoin = attr->strokeLineJoin;
	shape->strokeLineCap = attr->strokeLineCap;
	shape->miterLimit = attr->miterLimit;
	shape->fillRule = attr->fillRule;
	shape->opacity = attr->opacity;
    shape->paintOrder = attr->paintOrder;

	shape->paths = p->plist;
	p->plist = NULL;

	// Calculate shape bounds
	shape->bounds[0] = shape->paths->bounds[0];
	shape->bounds[1] = shape->paths->bounds[1];
	shape->bounds[2] = shape->paths->bounds[2];
	shape->bounds[3] = shape->paths->bounds[3];
	for (path = shape->paths->next; path != NULL; path = path->next) {
		shape->bounds[0] = msvg__minf(shape->bounds[0], path->bounds[0]);
		shape->bounds[1] = msvg__minf(shape->bounds[1], path->bounds[1]);
		shape->bounds[2] = msvg__maxf(shape->bounds[2], path->bounds[2]);
		shape->bounds[3] = msvg__maxf(shape->bounds[3], path->bounds[3]);
	}

	// Set fill
	if (attr->hasFill == 0) {
		shape->fill.type = MSVG_PAINT_NONE;
	} else if (attr->hasFill == 1) {
		shape->fill.type = MSVG_PAINT_COLOR;
		shape->fill.color = attr->fillColor;
		shape->fill.color |= (unsigned int)(attr->fillOpacity*255) << 24;
	} else if (attr->hasFill == 2) {
		shape->fill.type = MSVG_PAINT_UNDEF;
	}

	// Set stroke
	if (attr->hasStroke == 0) {
		shape->stroke.type = MSVG_PAINT_NONE;
	} else if (attr->hasStroke == 1) {
		shape->stroke.type = MSVG_PAINT_COLOR;
		shape->stroke.color = attr->strokeColor;
		shape->stroke.color |= (unsigned int)(attr->strokeOpacity*255) << 24;
	} else if (attr->hasStroke == 2) {
		shape->stroke.type = MSVG_PAINT_UNDEF;
	}

	// Set flags
	shape->flags = (attr->visible ? MSVG_FLAGS_VISIBLE : 0x00);

	// Add to tail (image shapes or current symbol)
	if (p->curSymbol) {
		if (p->curSymbol->shapes == NULL)
			p->curSymbol->shapes = shape;
		else
			p->symbolTail->next = shape;
		p->symbolTail = shape;
	} else {
		if (p->image->shapes == NULL)
			p->image->shapes = shape;
		else
			p->shapesTail->next = shape;
		p->shapesTail = shape;
	}
}

static MSVGsymbol* msvg__findSymbol(MSVGparser* p, const char* id)
{
	MSVGsymbol* sym;
	for (sym = p->symbols; sym != NULL; sym = sym->next) {
		if (strcmp(sym->id, id) == 0)
			return sym;
	}
	return NULL;
}

static void msvg__deepCopyShape(MSVGparser* p, MSVGshape* src, float tx, float ty)
{
	MSVGshape* shape;
	MSVGpath* srcPath, * dstPath, * pathTail = NULL;
	int i;

	shape = (MSVGshape*)msvg__parserAlloc(p->image, sizeof(MSVGshape));
	if (shape == NULL) return;
	memset(shape, 0, sizeof(MSVGshape));

	memcpy(shape->id, src->id, sizeof shape->id);
	memcpy(shape->fillGradient, src->fillGradient, sizeof shape->fillGradient);
	memcpy(shape->strokeGradient, src->strokeGradient, sizeof shape->strokeGradient);
	memcpy(shape->xform, src->xform, sizeof shape->xform);
	shape->strokeWidth = src->strokeWidth;
	shape->strokeDashOffset = src->strokeDashOffset;
	shape->strokeDashCount = src->strokeDashCount;
	for (i = 0; i < src->strokeDashCount; i++)
		shape->strokeDashArray[i] = src->strokeDashArray[i];
	shape->strokeLineJoin = src->strokeLineJoin;
	shape->strokeLineCap = src->strokeLineCap;
	shape->miterLimit = src->miterLimit;
	shape->fillRule = src->fillRule;
	shape->opacity = src->opacity;
	shape->paintOrder = src->paintOrder;
	shape->fill = src->fill;
	shape->stroke = src->stroke;
	shape->flags = src->flags;

	// Deep-copy paths with x/y translation
	for (srcPath = src->paths; srcPath != NULL; srcPath = srcPath->next) {
		dstPath = (MSVGpath*)msvg__parserAlloc(p->image, sizeof(MSVGpath));
		if (dstPath == NULL) continue;
		memset(dstPath, 0, sizeof(MSVGpath));

		dstPath->npts = srcPath->npts;
		dstPath->closed = srcPath->closed;
		dstPath->pts = (float*)msvg__parserAlloc(p->image, srcPath->npts * 2 * sizeof(float));
		if (dstPath->pts == NULL) continue;

		for (i = 0; i < srcPath->npts; i++) {
			dstPath->pts[i*2] = srcPath->pts[i*2] + tx;
			dstPath->pts[i*2+1] = srcPath->pts[i*2+1] + ty;
		}

		// Recalculate bounds after translation
		dstPath->bounds[0] = srcPath->bounds[0] + tx;
		dstPath->bounds[1] = srcPath->bounds[1] + ty;
		dstPath->bounds[2] = srcPath->bounds[2] + tx;
		dstPath->bounds[3] = srcPath->bounds[3] + ty;

		if (shape->paths == NULL)
			shape->paths = dstPath;
		else
			pathTail->next = dstPath;
		pathTail = dstPath;
	}

	// Calculate shape bounds from translated paths
	if (shape->paths) {
		shape->bounds[0] = shape->paths->bounds[0];
		shape->bounds[1] = shape->paths->bounds[1];
		shape->bounds[2] = shape->paths->bounds[2];
		shape->bounds[3] = shape->paths->bounds[3];
		for (dstPath = shape->paths->next; dstPath != NULL; dstPath = dstPath->next) {
			shape->bounds[0] = msvg__minf(shape->bounds[0], dstPath->bounds[0]);
			shape->bounds[1] = msvg__minf(shape->bounds[1], dstPath->bounds[1]);
			shape->bounds[2] = msvg__maxf(shape->bounds[2], dstPath->bounds[2]);
			shape->bounds[3] = msvg__maxf(shape->bounds[3], dstPath->bounds[3]);
		}
	}

	// Add to image tail
	if (p->image->shapes == NULL)
		p->image->shapes = shape;
	else
		p->shapesTail->next = shape;
	p->shapesTail = shape;
}

static void msvg__addPath(MSVGparser* p, char closed)
{
	MSVGattrib* attr = msvg__getAttr(p);
	MSVGpath* path = NULL;
	float bounds[4];
	float* curve;
	int i;

	if (p->npts < 4)
		return;

	if (closed)
		msvg__lineTo(p, p->pts[0], p->pts[1]);

	// Expect 1 + N*3 points (N = number of cubic bezier segments).
	if ((p->npts % 3) != 1)
		return;

	path = (MSVGpath*)msvg__parserAlloc(p->image, sizeof(MSVGpath));
	if (path == NULL) return;
	memset(path, 0, sizeof(MSVGpath));

	path->pts = (float*)msvg__parserAlloc(p->image, p->npts * 2 * sizeof(float));
	if (path->pts == NULL) return;
	path->closed = closed;
	path->npts = p->npts;

	// Transform path.
	for (i = 0; i < p->npts; ++i)
		msvg__xformPoint(&path->pts[i*2], &path->pts[i*2+1], p->pts[i*2], p->pts[i*2+1], attr->xform);

	// Find bounds
	for (i = 0; i < path->npts-1; i += 3) {
		curve = &path->pts[i*2];
		msvg__curveBounds(bounds, curve);
		if (i == 0) {
			path->bounds[0] = bounds[0];
			path->bounds[1] = bounds[1];
			path->bounds[2] = bounds[2];
			path->bounds[3] = bounds[3];
		} else {
			path->bounds[0] = msvg__minf(path->bounds[0], bounds[0]);
			path->bounds[1] = msvg__minf(path->bounds[1], bounds[1]);
			path->bounds[2] = msvg__maxf(path->bounds[2], bounds[2]);
			path->bounds[3] = msvg__maxf(path->bounds[3], bounds[3]);
		}
	}

	path->next = p->plist;
	p->plist = path;
}

// We roll our own string to float because the std library one uses locale and messes things up.
static double msvg__atof(const char* s)
{
	char* cur = (char*)s;
	char* end = NULL;
	double res = 0.0, sign = 1.0;
	long long intPart = 0, fracPart = 0;
	char hasIntPart = 0, hasFracPart = 0;

	// Parse optional sign
	if (*cur == '+') {
		cur++;
	} else if (*cur == '-') {
		sign = -1;
		cur++;
	}

	// Parse integer part
	if (msvg__isdigit(*cur)) {
		// Parse digit sequence
		intPart = strtoll(cur, &end, 10);
		if (cur != end) {
			res = (double)intPart;
			hasIntPart = 1;
			cur = end;
		}
	}

	// Parse fractional part.
	if (*cur == '.') {
		cur++; // Skip '.'
		if (msvg__isdigit(*cur)) {
			// Parse digit sequence
			fracPart = strtoll(cur, &end, 10);
			if (cur != end) {
				res += (double)fracPart / pow(10.0, (double)(end - cur));
				hasFracPart = 1;
				cur = end;
			}
		}
	}

	// A valid number should have integer or fractional part.
	if (!hasIntPart && !hasFracPart)
		return 0.0;

	// Parse optional exponent
	if (*cur == 'e' || *cur == 'E') {
		long expPart = 0;
		cur++; // skip 'E'
		expPart = strtol(cur, &end, 10); // Parse digit sequence with sign
		if (cur != end) {
			res *= pow(10.0, (double)expPart);
		}
	}

	return res * sign;
}

static void msvg__parseUse(MSVGparser* p, const char** attr)
{
	MSVGsymbol* sym = NULL;
	const char* href = NULL;
	float x = 0.0f, y = 0.0f;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "xlink:href") == 0 || strcmp(attr[i], "href") == 0) {
			href = attr[i + 1];
		} else if (strcmp(attr[i], "x") == 0) {
			x = (float)msvg__atof(attr[i + 1]);
		} else if (strcmp(attr[i], "y") == 0) {
			y = (float)msvg__atof(attr[i + 1]);
		}
	}

	if (href && href[0] == '#') {
		sym = msvg__findSymbol(p, href + 1);
	}

	if (sym && sym->shapes) {
		MSVGshape* src;
		for (src = sym->shapes; src != NULL; src = src->next) {
			msvg__deepCopyShape(p, src, x, y);
		}
	}
}

static const char* msvg__parseNumber(const char* s, char* it, const int size)
{
	const int last = size-1;
	int i = 0;

	// sign
	if (*s == '-' || *s == '+') {
		if (i < last) it[i++] = *s;
		s++;
	}
	// integer part
	while (*s && msvg__isdigit(*s)) {
		if (i < last) it[i++] = *s;
		s++;
	}
	if (*s == '.') {
		// decimal point
		if (i < last) it[i++] = *s;
		s++;
		// fraction part
		while (*s && msvg__isdigit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	// exponent
	if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x')) {
		if (i < last) it[i++] = *s;
		s++;
		if (*s == '-' || *s == '+') {
			if (i < last) it[i++] = *s;
			s++;
		}
		while (*s && msvg__isdigit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	it[i] = '\0';

	return s;
}

static const char* msvg__getNextPathItemWhenArcFlag(const char* s, char* it)
{
	it[0] = '\0';
	while (*s && (msvg__isspace(*s) || *s == ',')) s++;
	if (!*s) return s;
	if (*s == '0' || *s == '1') {
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}
	return s;
}

static const char* msvg__getNextPathItem(const char* s, char* it)
{
	it[0] = '\0';
	// Skip white spaces and commas
	while (*s && (msvg__isspace(*s) || *s == ',')) s++;
	if (!*s) return s;
	if (*s == '-' || *s == '+' || *s == '.' || msvg__isdigit(*s)) {
		s = msvg__parseNumber(s, it, 64);
	} else {
		// Parse command
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}

	return s;
}

static unsigned int msvg__parseColorHex(const char* str)
{
	unsigned int r=0, g=0, b=0;
	if (sscanf(str, "#%2x%2x%2x", &r, &g, &b) == 3 )		// 2 digit hex
		return MSVG_RGB(r, g, b);
	if (sscanf(str, "#%1x%1x%1x", &r, &g, &b) == 3 )		// 1 digit hex, e.g. #abc -> 0xccbbaa
		return MSVG_RGB(r*17, g*17, b*17);			// same effect as (r<<4|r), (g<<4|g), ..
	return MSVG_RGB(128, 128, 128);
}

// Parse rgb color. The pointer 'str' must point at "rgb(" (4+ characters).
// This function returns gray (rgb(128, 128, 128) == '#808080') on parse errors
// for backwards compatibility. Note: other image viewers return black instead.

static unsigned int msvg__parseColorRGB(const char* str)
{
	int i;
	unsigned int rgbi[3];
	float rgbf[3];
	// try decimal integers first
	if (sscanf(str, "rgb(%u, %u, %u)", &rgbi[0], &rgbi[1], &rgbi[2]) != 3) {
		// integers failed, try percent values (float, locale independent)
		const char delimiter[3] = {',', ',', ')'};
		str += 4; // skip "rgb("
		for (i = 0; i < 3; i++) {
			while (*str && (msvg__isspace(*str))) str++; 	// skip leading spaces
			if (*str == '+') str++;				// skip '+' (don't allow '-')
			if (!*str) break;
			rgbf[i] = msvg__atof(str);

			// Note 1: it would be great if msvg__atof() returned how many
			// bytes it consumed but it doesn't. We need to skip the number,
			// the '%' character, spaces, and the delimiter ',' or ')'.

			// Note 2: The following code does not allow values like "33.%",
			// i.e. a decimal point w/o fractional part, but this is consistent
			// with other image viewers, e.g. firefox, chrome, eog, gimp.

			while (*str && msvg__isdigit(*str)) str++;		// skip integer part
			if (*str == '.') {
				str++;
				if (!msvg__isdigit(*str)) break;		// error: no digit after '.'
				while (*str && msvg__isdigit(*str)) str++;	// skip fractional part
			}
			if (*str == '%') str++; else break;
			while (*str && msvg__isspace(*str)) str++;
			if (*str == delimiter[i]) str++;
			else break;
		}
		if (i == 3) {
			rgbi[0] = roundf(rgbf[0] * 2.55f);
			rgbi[1] = roundf(rgbf[1] * 2.55f);
			rgbi[2] = roundf(rgbf[2] * 2.55f);
		} else {
			rgbi[0] = rgbi[1] = rgbi[2] = 128;
		}
	}
	// clip values as the CSS spec requires
	for (i = 0; i < 3; i++) {
		if (rgbi[i] > 255) rgbi[i] = 255;
	}
	return MSVG_RGB(rgbi[0], rgbi[1], rgbi[2]);
}

typedef struct MSVGNamedColor {
	const char* name;
	unsigned int color;
} MSVGNamedColor;

MSVGNamedColor msvg__colors[] = {

	{ "red", MSVG_RGB(255, 0, 0) },
	{ "green", MSVG_RGB( 0, 128, 0) },
	{ "blue", MSVG_RGB( 0, 0, 255) },
	{ "yellow", MSVG_RGB(255, 255, 0) },
	{ "cyan", MSVG_RGB( 0, 255, 255) },
	{ "magenta", MSVG_RGB(255, 0, 255) },
	{ "black", MSVG_RGB( 0, 0, 0) },
	{ "grey", MSVG_RGB(128, 128, 128) },
	{ "gray", MSVG_RGB(128, 128, 128) },
	{ "white", MSVG_RGB(255, 255, 255) },

#ifdef MOONSVG_ALL_COLOR_KEYWORDS
	{ "aliceblue", MSVG_RGB(240, 248, 255) },
	{ "antiquewhite", MSVG_RGB(250, 235, 215) },
	{ "aqua", MSVG_RGB( 0, 255, 255) },
	{ "aquamarine", MSVG_RGB(127, 255, 212) },
	{ "azure", MSVG_RGB(240, 255, 255) },
	{ "beige", MSVG_RGB(245, 245, 220) },
	{ "bisque", MSVG_RGB(255, 228, 196) },
	{ "blanchedalmond", MSVG_RGB(255, 235, 205) },
	{ "blueviolet", MSVG_RGB(138, 43, 226) },
	{ "brown", MSVG_RGB(165, 42, 42) },
	{ "burlywood", MSVG_RGB(222, 184, 135) },
	{ "cadetblue", MSVG_RGB( 95, 158, 160) },
	{ "chartreuse", MSVG_RGB(127, 255, 0) },
	{ "chocolate", MSVG_RGB(210, 105, 30) },
	{ "coral", MSVG_RGB(255, 127, 80) },
	{ "cornflowerblue", MSVG_RGB(100, 149, 237) },
	{ "cornsilk", MSVG_RGB(255, 248, 220) },
	{ "crimson", MSVG_RGB(220, 20, 60) },
	{ "darkblue", MSVG_RGB( 0, 0, 139) },
	{ "darkcyan", MSVG_RGB( 0, 139, 139) },
	{ "darkgoldenrod", MSVG_RGB(184, 134, 11) },
	{ "darkgray", MSVG_RGB(169, 169, 169) },
	{ "darkgreen", MSVG_RGB( 0, 100, 0) },
	{ "darkgrey", MSVG_RGB(169, 169, 169) },
	{ "darkkhaki", MSVG_RGB(189, 183, 107) },
	{ "darkmagenta", MSVG_RGB(139, 0, 139) },
	{ "darkolivegreen", MSVG_RGB( 85, 107, 47) },
	{ "darkorange", MSVG_RGB(255, 140, 0) },
	{ "darkorchid", MSVG_RGB(153, 50, 204) },
	{ "darkred", MSVG_RGB(139, 0, 0) },
	{ "darksalmon", MSVG_RGB(233, 150, 122) },
	{ "darkseagreen", MSVG_RGB(143, 188, 143) },
	{ "darkslateblue", MSVG_RGB( 72, 61, 139) },
	{ "darkslategray", MSVG_RGB( 47, 79, 79) },
	{ "darkslategrey", MSVG_RGB( 47, 79, 79) },
	{ "darkturquoise", MSVG_RGB( 0, 206, 209) },
	{ "darkviolet", MSVG_RGB(148, 0, 211) },
	{ "deeppink", MSVG_RGB(255, 20, 147) },
	{ "deepskyblue", MSVG_RGB( 0, 191, 255) },
	{ "dimgray", MSVG_RGB(105, 105, 105) },
	{ "dimgrey", MSVG_RGB(105, 105, 105) },
	{ "dodgerblue", MSVG_RGB( 30, 144, 255) },
	{ "firebrick", MSVG_RGB(178, 34, 34) },
	{ "floralwhite", MSVG_RGB(255, 250, 240) },
	{ "forestgreen", MSVG_RGB( 34, 139, 34) },
	{ "fuchsia", MSVG_RGB(255, 0, 255) },
	{ "gainsboro", MSVG_RGB(220, 220, 220) },
	{ "ghostwhite", MSVG_RGB(248, 248, 255) },
	{ "gold", MSVG_RGB(255, 215, 0) },
	{ "goldenrod", MSVG_RGB(218, 165, 32) },
	{ "greenyellow", MSVG_RGB(173, 255, 47) },
	{ "honeydew", MSVG_RGB(240, 255, 240) },
	{ "hotpink", MSVG_RGB(255, 105, 180) },
	{ "indianred", MSVG_RGB(205, 92, 92) },
	{ "indigo", MSVG_RGB( 75, 0, 130) },
	{ "ivory", MSVG_RGB(255, 255, 240) },
	{ "khaki", MSVG_RGB(240, 230, 140) },
	{ "lavender", MSVG_RGB(230, 230, 250) },
	{ "lavenderblush", MSVG_RGB(255, 240, 245) },
	{ "lawngreen", MSVG_RGB(124, 252, 0) },
	{ "lemonchiffon", MSVG_RGB(255, 250, 205) },
	{ "lightblue", MSVG_RGB(173, 216, 230) },
	{ "lightcoral", MSVG_RGB(240, 128, 128) },
	{ "lightcyan", MSVG_RGB(224, 255, 255) },
	{ "lightgoldenrodyellow", MSVG_RGB(250, 250, 210) },
	{ "lightgray", MSVG_RGB(211, 211, 211) },
	{ "lightgreen", MSVG_RGB(144, 238, 144) },
	{ "lightgrey", MSVG_RGB(211, 211, 211) },
	{ "lightpink", MSVG_RGB(255, 182, 193) },
	{ "lightsalmon", MSVG_RGB(255, 160, 122) },
	{ "lightseagreen", MSVG_RGB( 32, 178, 170) },
	{ "lightskyblue", MSVG_RGB(135, 206, 250) },
	{ "lightslategray", MSVG_RGB(119, 136, 153) },
	{ "lightslategrey", MSVG_RGB(119, 136, 153) },
	{ "lightsteelblue", MSVG_RGB(176, 196, 222) },
	{ "lightyellow", MSVG_RGB(255, 255, 224) },
	{ "lime", MSVG_RGB( 0, 255, 0) },
	{ "limegreen", MSVG_RGB( 50, 205, 50) },
	{ "linen", MSVG_RGB(250, 240, 230) },
	{ "maroon", MSVG_RGB(128, 0, 0) },
	{ "mediumaquamarine", MSVG_RGB(102, 205, 170) },
	{ "mediumblue", MSVG_RGB( 0, 0, 205) },
	{ "mediumorchid", MSVG_RGB(186, 85, 211) },
	{ "mediumpurple", MSVG_RGB(147, 112, 219) },
	{ "mediumseagreen", MSVG_RGB( 60, 179, 113) },
	{ "mediumslateblue", MSVG_RGB(123, 104, 238) },
	{ "mediumspringgreen", MSVG_RGB( 0, 250, 154) },
	{ "mediumturquoise", MSVG_RGB( 72, 209, 204) },
	{ "mediumvioletred", MSVG_RGB(199, 21, 133) },
	{ "midnightblue", MSVG_RGB( 25, 25, 112) },
	{ "mintcream", MSVG_RGB(245, 255, 250) },
	{ "mistyrose", MSVG_RGB(255, 228, 225) },
	{ "moccasin", MSVG_RGB(255, 228, 181) },
	{ "navajowhite", MSVG_RGB(255, 222, 173) },
	{ "navy", MSVG_RGB( 0, 0, 128) },
	{ "oldlace", MSVG_RGB(253, 245, 230) },
	{ "olive", MSVG_RGB(128, 128, 0) },
	{ "olivedrab", MSVG_RGB(107, 142, 35) },
	{ "orange", MSVG_RGB(255, 165, 0) },
	{ "orangered", MSVG_RGB(255, 69, 0) },
	{ "orchid", MSVG_RGB(218, 112, 214) },
	{ "palegoldenrod", MSVG_RGB(238, 232, 170) },
	{ "palegreen", MSVG_RGB(152, 251, 152) },
	{ "paleturquoise", MSVG_RGB(175, 238, 238) },
	{ "palevioletred", MSVG_RGB(219, 112, 147) },
	{ "papayawhip", MSVG_RGB(255, 239, 213) },
	{ "peachpuff", MSVG_RGB(255, 218, 185) },
	{ "peru", MSVG_RGB(205, 133, 63) },
	{ "pink", MSVG_RGB(255, 192, 203) },
	{ "plum", MSVG_RGB(221, 160, 221) },
	{ "powderblue", MSVG_RGB(176, 224, 230) },
	{ "purple", MSVG_RGB(128, 0, 128) },
	{ "rosybrown", MSVG_RGB(188, 143, 143) },
	{ "royalblue", MSVG_RGB( 65, 105, 225) },
	{ "saddlebrown", MSVG_RGB(139, 69, 19) },
	{ "salmon", MSVG_RGB(250, 128, 114) },
	{ "sandybrown", MSVG_RGB(244, 164, 96) },
	{ "seagreen", MSVG_RGB( 46, 139, 87) },
	{ "seashell", MSVG_RGB(255, 245, 238) },
	{ "sienna", MSVG_RGB(160, 82, 45) },
	{ "silver", MSVG_RGB(192, 192, 192) },
	{ "skyblue", MSVG_RGB(135, 206, 235) },
	{ "slateblue", MSVG_RGB(106, 90, 205) },
	{ "slategray", MSVG_RGB(112, 128, 144) },
	{ "slategrey", MSVG_RGB(112, 128, 144) },
	{ "snow", MSVG_RGB(255, 250, 250) },
	{ "springgreen", MSVG_RGB( 0, 255, 127) },
	{ "steelblue", MSVG_RGB( 70, 130, 180) },
	{ "tan", MSVG_RGB(210, 180, 140) },
	{ "teal", MSVG_RGB( 0, 128, 128) },
	{ "thistle", MSVG_RGB(216, 191, 216) },
	{ "tomato", MSVG_RGB(255, 99, 71) },
	{ "turquoise", MSVG_RGB( 64, 224, 208) },
	{ "violet", MSVG_RGB(238, 130, 238) },
	{ "wheat", MSVG_RGB(245, 222, 179) },
	{ "whitesmoke", MSVG_RGB(245, 245, 245) },
	{ "yellowgreen", MSVG_RGB(154, 205, 50) },
#endif
};

static unsigned int msvg__parseColorName(const char* str)
{
	int i, ncolors = sizeof(msvg__colors) / sizeof(MSVGNamedColor);

	for (i = 0; i < ncolors; i++) {
		if (strcmp(msvg__colors[i].name, str) == 0) {
			return msvg__colors[i].color;
		}
	}

	return MSVG_RGB(128, 128, 128);
}

static unsigned int msvg__parseColor(const char* str)
{
	size_t len = 0;
	while(*str == ' ') ++str;
	len = strlen(str);
	if (len >= 1 && *str == '#')
		return msvg__parseColorHex(str);
	else if (len >= 12 && strcmp(str, "currentColor") == 0)
		return MSVG_RGB(255, 255, 255);
	else if (len >= 4 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == '(')
		return msvg__parseColorRGB(str);
	return msvg__parseColorName(str);
}

static float msvg__parseOpacity(const char* str)
{
	float val = msvg__atof(str);
	if (val < 0.0f) val = 0.0f;
	if (val > 1.0f) val = 1.0f;
	return val;
}

static float msvg__parseMiterLimit(const char* str)
{
	float val = msvg__atof(str);
	if (val < 0.0f) val = 0.0f;
	return val;
}

static int msvg__parseUnits(const char* units)
{
	if (units[0] == 'p' && units[1] == 'x')
		return MSVG_UNITS_PX;
	else if (units[0] == 'p' && units[1] == 't')
		return MSVG_UNITS_PT;
	else if (units[0] == 'p' && units[1] == 'c')
		return MSVG_UNITS_PC;
	else if (units[0] == 'm' && units[1] == 'm')
		return MSVG_UNITS_MM;
	else if (units[0] == 'c' && units[1] == 'm')
		return MSVG_UNITS_CM;
	else if (units[0] == 'i' && units[1] == 'n')
		return MSVG_UNITS_IN;
	else if (units[0] == '%')
		return MSVG_UNITS_PERCENT;
	else if (units[0] == 'e' && units[1] == 'm')
		return MSVG_UNITS_EM;
	else if (units[0] == 'e' && units[1] == 'x')
		return MSVG_UNITS_EX;
	return MSVG_UNITS_USER;
}

static int msvg__isCoordinate(const char* s)
{
	// optional sign
	if (*s == '-' || *s == '+')
		s++;
	// must have at least one digit, or start by a dot
	return (msvg__isdigit(*s) || *s == '.');
}

static MSVGcoordinate msvg__parseCoordinateRaw(const char* str)
{
	MSVGcoordinate coord = {0, MSVG_UNITS_USER};
	char buf[64];
	coord.units = msvg__parseUnits(msvg__parseNumber(str, buf, 64));
	coord.value = msvg__atof(buf);
	return coord;
}

static MSVGcoordinate msvg__coord(float v, int units)
{
	MSVGcoordinate coord = {v, units};
	return coord;
}

static float msvg__parseCoordinate(MSVGparser* p, const char* str, float orig, float length)
{
	MSVGcoordinate coord = msvg__parseCoordinateRaw(str);
	return msvg__convertToPixels(p, coord, orig, length);
}

static int msvg__parseTransformArgs(const char* str, float* args, int maxNa, int* na)
{
	const char* end;
	const char* ptr;
	char it[64];

	*na = 0;
	ptr = str;
	while (*ptr && *ptr != '(') ++ptr;
	if (*ptr == 0)
		return 1;
	end = ptr;
	while (*end && *end != ')') ++end;
	if (*end == 0)
		return 1;

	while (ptr < end) {
		if (*ptr == '-' || *ptr == '+' || *ptr == '.' || msvg__isdigit(*ptr)) {
			if (*na >= maxNa) return 0;
			ptr = msvg__parseNumber(ptr, it, 64);
			args[(*na)++] = (float)msvg__atof(it);
		} else {
			++ptr;
		}
	}
	return (int)(end - str);
}


static int msvg__parseMatrix(float* xform, const char* str)
{
	float t[6];
	int na = 0;
	int len = msvg__parseTransformArgs(str, t, 6, &na);
	if (na != 6) return len;
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int msvg__parseTranslate(float* xform, const char* str)
{
	float args[2];
	float t[6];
	int na = 0;
	int len = msvg__parseTransformArgs(str, args, 2, &na);
	if (na == 1) args[1] = 0.0;

	msvg__xformSetTranslation(t, args[0], args[1]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int msvg__parseScale(float* xform, const char* str)
{
	float args[2];
	int na = 0;
	float t[6];
	int len = msvg__parseTransformArgs(str, args, 2, &na);
	if (na == 1) args[1] = args[0];
	msvg__xformSetScale(t, args[0], args[1]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int msvg__parseSkewX(float* xform, const char* str)
{
	float args[1];
	int na = 0;
	float t[6];
	int len = msvg__parseTransformArgs(str, args, 1, &na);
	msvg__xformSetSkewX(t, args[0]/180.0f*MSVG_PI);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int msvg__parseSkewY(float* xform, const char* str)
{
	float args[1];
	int na = 0;
	float t[6];
	int len = msvg__parseTransformArgs(str, args, 1, &na);
	msvg__xformSetSkewY(t, args[0]/180.0f*MSVG_PI);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int msvg__parseRotate(float* xform, const char* str)
{
	float args[3];
	int na = 0;
	float m[6];
	float t[6];
	int len = msvg__parseTransformArgs(str, args, 3, &na);
	if (na == 1)
		args[1] = args[2] = 0.0f;
	msvg__xformIdentity(m);

	if (na > 1) {
		msvg__xformSetTranslation(t, -args[1], -args[2]);
		msvg__xformMultiply(m, t);
	}

	msvg__xformSetRotation(t, args[0]/180.0f*MSVG_PI);
	msvg__xformMultiply(m, t);

	if (na > 1) {
		msvg__xformSetTranslation(t, args[1], args[2]);
		msvg__xformMultiply(m, t);
	}

	memcpy(xform, m, sizeof(float)*6);

	return len;
}

static void msvg__parseTransform(float* xform, const char* str)
{
	float t[6];
	int len;
	msvg__xformIdentity(xform);
	while (*str)
	{
		if (strncmp(str, "matrix", 6) == 0)
			len = msvg__parseMatrix(t, str);
		else if (strncmp(str, "translate", 9) == 0)
			len = msvg__parseTranslate(t, str);
		else if (strncmp(str, "scale", 5) == 0)
			len = msvg__parseScale(t, str);
		else if (strncmp(str, "rotate", 6) == 0)
			len = msvg__parseRotate(t, str);
		else if (strncmp(str, "skewX", 5) == 0)
			len = msvg__parseSkewX(t, str);
		else if (strncmp(str, "skewY", 5) == 0)
			len = msvg__parseSkewY(t, str);
		else{
			++str;
			continue;
		}
		if (len != 0) {
			str += len;
		} else {
			++str;
			continue;
		}

		msvg__xformPremultiply(xform, t);
	}
}

static void msvg__parseUrl(char* id, const char* str)
{
	int i = 0;
	str += 4; // "url(";
	if (*str && *str == '#')
		str++;
	while (i < 63 && *str && *str != ')') {
		id[i] = *str++;
		i++;
	}
	id[i] = '\0';
}

static char msvg__parseLineCap(const char* str)
{
	if (strcmp(str, "butt") == 0)
		return MSVG_CAP_BUTT;
	else if (strcmp(str, "round") == 0)
		return MSVG_CAP_ROUND;
	else if (strcmp(str, "square") == 0)
		return MSVG_CAP_SQUARE;
	// TODO: handle inherit.
	return MSVG_CAP_BUTT;
}

static char msvg__parseLineJoin(const char* str)
{
	if (strcmp(str, "miter") == 0)
		return MSVG_JOIN_MITER;
	else if (strcmp(str, "round") == 0)
		return MSVG_JOIN_ROUND;
	else if (strcmp(str, "bevel") == 0)
		return MSVG_JOIN_BEVEL;
	// TODO: handle inherit.
	return MSVG_JOIN_MITER;
}

static char msvg__parseFillRule(const char* str)
{
	if (strcmp(str, "nonzero") == 0)
		return MSVG_FILLRULE_NONZERO;
	else if (strcmp(str, "evenodd") == 0)
		return MSVG_FILLRULE_EVENODD;
	// TODO: handle inherit.
	return MSVG_FILLRULE_NONZERO;
}

static unsigned char msvg__parsePaintOrder(const char* str)
{
	if (strcmp(str, "normal") == 0 || strcmp(str, "fill stroke markers") == 0)
		return msvg__encodePaintOrder(MSVG_PAINT_FILL, MSVG_PAINT_STROKE, MSVG_PAINT_MARKERS);
	else if (strcmp(str, "fill markers stroke") == 0)
		return msvg__encodePaintOrder(MSVG_PAINT_FILL, MSVG_PAINT_MARKERS, MSVG_PAINT_STROKE);
	else if (strcmp(str, "markers fill stroke") == 0)
		return msvg__encodePaintOrder(MSVG_PAINT_MARKERS, MSVG_PAINT_FILL, MSVG_PAINT_STROKE);
	else if (strcmp(str, "markers stroke fill") == 0)
		return msvg__encodePaintOrder(MSVG_PAINT_MARKERS, MSVG_PAINT_STROKE, MSVG_PAINT_FILL);
	else if (strcmp(str, "stroke fill markers") == 0)
		return msvg__encodePaintOrder(MSVG_PAINT_STROKE, MSVG_PAINT_FILL, MSVG_PAINT_MARKERS);
	else if (strcmp(str, "stroke markers fill") == 0)
		return msvg__encodePaintOrder(MSVG_PAINT_STROKE, MSVG_PAINT_MARKERS, MSVG_PAINT_FILL);
	// TODO: handle inherit.
	return msvg__encodePaintOrder(MSVG_PAINT_FILL, MSVG_PAINT_STROKE, MSVG_PAINT_MARKERS);
}

static const char* msvg__getNextDashItem(const char* s, char* it)
{
	int n = 0;
	it[0] = '\0';
	// Skip white spaces and commas
	while (*s && (msvg__isspace(*s) || *s == ',')) s++;
	// Advance until whitespace, comma or end.
	while (*s && (!msvg__isspace(*s) && *s != ',')) {
		if (n < 63)
			it[n++] = *s;
		s++;
	}
	it[n++] = '\0';
	return s;
}

static int msvg__parseStrokeDashArray(MSVGparser* p, const char* str, float* strokeDashArray)
{
	char item[64];
	int count = 0, i;
	float sum = 0.0f;

	// Handle "none"
	if (str[0] == 'n')
		return 0;

	// Parse dashes
	while (*str) {
		str = msvg__getNextDashItem(str, item);
		if (!*item) break;
		if (count < MSVG_MAX_DASHES)
			strokeDashArray[count++] = fabsf(msvg__parseCoordinate(p, item, 0.0f, msvg__actualLength(p)));
	}

	for (i = 0; i < count; i++)
		sum += strokeDashArray[i];
	if (sum <= 1e-6f)
		count = 0;

	return count;
}

static void msvg__parseStyle(MSVGparser* p, const char* str);

static int msvg__parseAttr(MSVGparser* p, const char* name, const char* value)
{
	float xform[6];
	MSVGattrib* attr = msvg__getAttr(p);
	if (!attr) return 0;

	if (strcmp(name, "style") == 0) {
		msvg__parseStyle(p, value);
	} else if (strcmp(name, "display") == 0) {
		if (strcmp(value, "none") == 0)
			attr->visible = 0;
		// Don't reset ->visible on display:inline, one display:none hides the whole subtree

	} else if (strcmp(name, "fill") == 0) {
		if (strcmp(value, "none") == 0) {
			attr->hasFill = 0;
		} else if (strncmp(value, "url(", 4) == 0) {
			attr->hasFill = 2;
			msvg__parseUrl(attr->fillGradient, value);
		} else {
			attr->hasFill = 1;
			attr->fillColor = msvg__parseColor(value);
		}
	} else if (strcmp(name, "opacity") == 0) {
		attr->opacity = msvg__parseOpacity(value);
	} else if (strcmp(name, "fill-opacity") == 0) {
		attr->fillOpacity = msvg__parseOpacity(value);
	} else if (strcmp(name, "stroke") == 0) {
		if (strcmp(value, "none") == 0) {
			attr->hasStroke = 0;
		} else if (strncmp(value, "url(", 4) == 0) {
			attr->hasStroke = 2;
			msvg__parseUrl(attr->strokeGradient, value);
		} else {
			attr->hasStroke = 1;
			attr->strokeColor = msvg__parseColor(value);
		}
	} else if (strcmp(name, "stroke-width") == 0) {
		attr->strokeWidth = msvg__parseCoordinate(p, value, 0.0f, msvg__actualLength(p));
	} else if (strcmp(name, "stroke-dasharray") == 0) {
		attr->strokeDashCount = msvg__parseStrokeDashArray(p, value, attr->strokeDashArray);
	} else if (strcmp(name, "stroke-dashoffset") == 0) {
		attr->strokeDashOffset = msvg__parseCoordinate(p, value, 0.0f, msvg__actualLength(p));
	} else if (strcmp(name, "stroke-opacity") == 0) {
		attr->strokeOpacity = msvg__parseOpacity(value);
	} else if (strcmp(name, "stroke-linecap") == 0) {
		attr->strokeLineCap = msvg__parseLineCap(value);
	} else if (strcmp(name, "stroke-linejoin") == 0) {
		attr->strokeLineJoin = msvg__parseLineJoin(value);
	} else if (strcmp(name, "stroke-miterlimit") == 0) {
		attr->miterLimit = msvg__parseMiterLimit(value);
	} else if (strcmp(name, "fill-rule") == 0) {
		attr->fillRule = msvg__parseFillRule(value);
	} else if (strcmp(name, "font-size") == 0) {
		attr->fontSize = msvg__parseCoordinate(p, value, 0.0f, msvg__actualLength(p));
	} else if (strcmp(name, "transform") == 0) {
		msvg__parseTransform(xform, value);
		msvg__xformPremultiply(attr->xform, xform);
	} else if (strcmp(name, "stop-color") == 0) {
		attr->stopColor = msvg__parseColor(value);
	} else if (strcmp(name, "stop-opacity") == 0) {
		attr->stopOpacity = msvg__parseOpacity(value);
	} else if (strcmp(name, "offset") == 0) {
		attr->stopOffset = msvg__parseCoordinate(p, value, 0.0f, 1.0f);
	} else if (strcmp(name, "paint-order") == 0) {
		attr->paintOrder = msvg__parsePaintOrder(value);
	} else if (strcmp(name, "id") == 0) {
		strncpy(attr->id, value, 63);
		attr->id[63] = '\0';
	} else {
		return 0;
	}
	return 1;
}

static int msvg__parseNameValue(MSVGparser* p, const char* start, const char* end)
{
	const char* str;
	const char* val;
	char name[512];
	char value[512];
	int n;

	str = start;
	while (str < end && *str != ':') ++str;

	val = str;

	// Right Trim
	while (str > start &&  (*str == ':' || msvg__isspace(*str))) --str;
	++str;

	n = (int)(str - start);
	if (n > 511) n = 511;
	if (n) memcpy(name, start, n);
	name[n] = 0;

	while (val < end && (*val == ':' || msvg__isspace(*val))) ++val;

	n = (int)(end - val);
	if (n > 511) n = 511;
	if (n) memcpy(value, val, n);
	value[n] = 0;

	return msvg__parseAttr(p, name, value);
}

static void msvg__parseStyle(MSVGparser* p, const char* str)
{
	const char* start;
	const char* end;

	while (*str) {
		// Left Trim
		while(*str && msvg__isspace(*str)) ++str;
		start = str;
		while(*str && *str != ';') ++str;
		end = str;

		// Right Trim
		while (end > start &&  (*end == ';' || msvg__isspace(*end))) --end;
		++end;

		msvg__parseNameValue(p, start, end);
		if (*str) ++str;
	}
}

static MSVGstyle* msvg__findStyle(MSVGparser* p, const char* name)
{
	MSVGstyle* st;
	for (st = p->styles; st != NULL; st = st->next) {
		if (strcmp(st->name, name) == 0)
			return st;
	}
	return NULL;
}

static void msvg__parseCSSStyles(MSVGparser* p, char* str)
{
	char* s = str;
	while (*s) {
		// Skip whitespace and comments
		while (*s && msvg__isspace(*s)) s++;
		if (!*s) break;

		// Look for class selector: .name
		if (*s != '.') {
			// Skip until next '.' or end
			while (*s && *s != '.') s++;
			continue;
		}
		s++; // skip '.'

		// Extract class name
		char* nameStart = s;
		while (*s && !msvg__isspace(*s) && *s != '{' && *s != '.') s++;
		int nameLen = (int)(s - nameStart);
		if (nameLen <= 0 || nameLen >= 64) {
			while (*s && *s != '{' && *s != '.') s++;
			continue;
		}

		// Skip whitespace before {
		while (*s && msvg__isspace(*s)) s++;
		if (*s != '{') continue;
		s++; // skip '{'

		// Find closing }
		char* propsStart = s;
		int depth = 1;
		while (*s && depth > 0) {
			if (*s == '{') depth++;
			else if (*s == '}') depth--;
			if (depth > 0) s++;
		}
		if (depth != 0) break; // unbalanced braces

		// s now points to '}', terminate properties string
		*s = '\0';
		s++; // move past '}'

		// Allocate and fill style entry from pool
		MSVGstyle* st = (MSVGstyle*)msvg__parserAlloc(p->image, sizeof(MSVGstyle));
		if (st == NULL) continue;
		memset(st, 0, sizeof(MSVGstyle));
		memcpy(st->name, nameStart, nameLen);
		st->name[nameLen] = '\0';
		st->properties = propsStart;
		st->next = p->styles;
		p->styles = st;
	}
}

static void msvg__applyClassStyles(MSVGparser* p, const char* classNames)
{
	char buf[256];
	int i, n;
	const char* s = classNames;

	while (*s) {
		// Skip leading whitespace
		while (*s && msvg__isspace(*s)) s++;
		if (!*s) break;

		// Extract one class name
		n = 0;
		while (*s && !msvg__isspace(*s) && n < 255) {
			buf[n++] = *s++;
		}
		buf[n] = '\0';

		if (n > 0) {
			MSVGstyle* st = msvg__findStyle(p, buf);
			if (st && st->properties) {
				msvg__parseStyle(p, st->properties);
			}
		}
	}
}

static void msvg__parseAttribs(MSVGparser* p, const char** attr)
{
	int i;

	// Pass 1: Apply class styles first (so inline can override)
	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "class") == 0) {
			msvg__applyClassStyles(p, attr[i + 1]);
		}
	}

	// Pass 2: Process all attributes except class (inline overrides)
	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "class") == 0)
			continue;
		if (strcmp(attr[i], "style") == 0)
			msvg__parseStyle(p, attr[i + 1]);
		else
			msvg__parseAttr(p, attr[i], attr[i + 1]);
	}
}

static int msvg__getArgsPerElement(char cmd)
{
	switch (cmd) {
		case 'v':
		case 'V':
		case 'h':
		case 'H':
			return 1;
		case 'm':
		case 'M':
		case 'l':
		case 'L':
		case 't':
		case 'T':
			return 2;
		case 'q':
		case 'Q':
		case 's':
		case 'S':
			return 4;
		case 'c':
		case 'C':
			return 6;
		case 'a':
		case 'A':
			return 7;
		case 'z':
		case 'Z':
			return 0;
	}
	return -1;
}

static void msvg__pathMoveTo(MSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	msvg__moveTo(p, *cpx, *cpy);
}

static void msvg__pathLineTo(MSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	msvg__lineTo(p, *cpx, *cpy);
}

static void msvg__pathHLineTo(MSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpx += args[0];
	else
		*cpx = args[0];
	msvg__lineTo(p, *cpx, *cpy);
}

static void msvg__pathVLineTo(MSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpy += args[0];
	else
		*cpy = args[0];
	msvg__lineTo(p, *cpx, *cpy);
}

static void msvg__pathCubicBezTo(MSVGparser* p, float* cpx, float* cpy,
								 float* cpx2, float* cpy2, float* args, int rel)
{
	float x2, y2, cx1, cy1, cx2, cy2;

	if (rel) {
		cx1 = *cpx + args[0];
		cy1 = *cpy + args[1];
		cx2 = *cpx + args[2];
		cy2 = *cpy + args[3];
		x2 = *cpx + args[4];
		y2 = *cpy + args[5];
	} else {
		cx1 = args[0];
		cy1 = args[1];
		cx2 = args[2];
		cy2 = args[3];
		x2 = args[4];
		y2 = args[5];
	}

	msvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void msvg__pathCubicBezShortTo(MSVGparser* p, float* cpx, float* cpy,
									  float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx2 = *cpx + args[0];
		cy2 = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx2 = args[0];
		cy2 = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	cx1 = 2*x1 - *cpx2;
	cy1 = 2*y1 - *cpy2;

	msvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void msvg__pathQuadBezTo(MSVGparser* p, float* cpx, float* cpy,
								float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx = *cpx + args[0];
		cy = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx = args[0];
		cy = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	// Convert to cubic bezier
	cx1 = x1 + 2.0f/3.0f*(cx - x1);
	cy1 = y1 + 2.0f/3.0f*(cy - y1);
	cx2 = x2 + 2.0f/3.0f*(cx - x2);
	cy2 = y2 + 2.0f/3.0f*(cy - y2);

	msvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static void msvg__pathQuadBezShortTo(MSVGparser* p, float* cpx, float* cpy,
									 float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		x2 = *cpx + args[0];
		y2 = *cpy + args[1];
	} else {
		x2 = args[0];
		y2 = args[1];
	}

	cx = 2*x1 - *cpx2;
	cy = 2*y1 - *cpy2;

	// Convert to cubix bezier
	cx1 = x1 + 2.0f/3.0f*(cx - x1);
	cy1 = y1 + 2.0f/3.0f*(cy - y1);
	cx2 = x2 + 2.0f/3.0f*(cx - x2);
	cy2 = y2 + 2.0f/3.0f*(cy - y2);

	msvg__cubicBezTo(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static float msvg__sqr(float x) { return x*x; }
static float msvg__vmag(float x, float y) { return sqrtf(x*x + y*y); }

static float msvg__vecrat(float ux, float uy, float vx, float vy)
{
	return (ux*vx + uy*vy) / (msvg__vmag(ux,uy) * msvg__vmag(vx,vy));
}

static float msvg__vecang(float ux, float uy, float vx, float vy)
{
	float r = msvg__vecrat(ux,uy, vx,vy);
	if (r < -1.0f) r = -1.0f;
	if (r > 1.0f) r = 1.0f;
	return ((ux*vy < uy*vx) ? -1.0f : 1.0f) * acosf(r);
}

static double msvg__vecang_d(double ux, double uy, double vx, double vy)
{
	double r = (ux * vx + uy * vy) / (sqrt(ux * ux + uy * uy) * sqrt(vx * vx + vy * vy));
	if (r < -1.0) r = -1.0;
	if (r > 1.0) r = 1.0;
	return ((ux * vy < uy * vx) ? -1.0 : 1.0) * acos(r);
}

static void msvg__pathArcTo(MSVGparser* p, float* cpx, float* cpy, float* args, int rel)
{
	// Ported from canvg (https://code.google.com/p/canvg/)
	double rx, ry, rotx;
	double x1, y1, x2, y2, cx, cy, dx, dy, d;
	double x1p, y1p, cxp, cyp, s, sa, sb;
	double ux, uy, vx, vy, a1, da;
	double x, y, tanx, tany, a, px = 0.0, py = 0.0, ptanx = 0.0, ptany = 0.0, t[6];
	double sinrx, cosrx;
	int fa, fs;
	int i, ndivs;
	double hda, kappa;
	static const double pi_d = 3.14159265358979323846;

	rx = fabs(args[0]);				// y radius
	ry = fabs(args[1]);				// x radius
	rotx = args[2] / 180.0 * pi_d;		// x rotation angle
	fa = fabs(args[3]) > 1e-6 ? 1 : 0;	// Large arc
	fs = fabs(args[4]) > 1e-6 ? 1 : 0;	// Sweep direction
	x1 = *cpx;						// start point
	y1 = *cpy;
	if (rel) {						// end point
		x2 = *cpx + args[5];
		y2 = *cpy + args[6];
	} else {
		x2 = args[5];
		y2 = args[6];
	}

	dx = x1 - x2;
	dy = y1 - y2;
	d = sqrt(dx * dx + dy * dy);
	if (d < 1e-6 || rx < 1e-6 || ry < 1e-6) {
		// The arc degenerates to a line
		msvg__lineTo(p, (float)x2, (float)y2);
		*cpx = (float)x2;
		*cpy = (float)y2;
		return;
	}

	sinrx = sin(rotx);
	cosrx = cos(rotx);

	// Convert to center point parameterization.
	// http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
	// 1) Compute x1', y1'
	x1p = cosrx * dx / 2.0 + sinrx * dy / 2.0;
	y1p = -sinrx * dx / 2.0 + cosrx * dy / 2.0;
	d = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
	if (d > 1) {
		d = sqrt(d);
		rx *= d;
		ry *= d;
	}
	// 2) Compute cx', cy'
	s = 0.0;
	sa = (rx * rx) * (ry * ry) - (rx * rx) * (y1p * y1p) - (ry * ry) * (x1p * x1p);
	sb = (rx * rx) * (y1p * y1p) + (ry * ry) * (x1p * x1p);
	if (sa < 0.0) sa = 0.0;
	if (sb > 0.0)
		s = sqrt(sa / sb);
	if (fa == fs)
		s = -s;
	cxp = s * rx * y1p / ry;
	cyp = s * -ry * x1p / rx;

	// 3) Compute cx,cy from cx',cy'
	cx = (x1 + x2) / 2.0 + cosrx * cxp - sinrx * cyp;
	cy = (y1 + y2) / 2.0 + sinrx * cxp + cosrx * cyp;

	// 4) Calculate theta1, and delta theta.
	ux = (x1p - cxp) / rx;
	uy = (y1p - cyp) / ry;
	vx = (-x1p - cxp) / rx;
	vy = (-y1p - cyp) / ry;
	a1 = msvg__vecang_d(1.0, 0.0, ux, uy);	// Initial angle
	da = msvg__vecang_d(ux, uy, vx, vy);		// Delta angle

//	if (vecrat(ux,uy,vx,vy) <= -1.0) da = pi_d;
//	if (vecrat(ux,uy,vx,vy) >= 1.0) da = 0;

	if (fs == 0 && da > 0)
		da -= 2 * pi_d;
	else if (fs == 1 && da < 0)
		da += 2 * pi_d;

	// Approximate the arc using cubic spline segments.
	t[0] = cosrx; t[1] = sinrx;
	t[2] = -sinrx; t[3] = cosrx;
	t[4] = cx; t[5] = cy;

	// Split arc into max 90 degree segments.
	// The loop assumes an iteration per end point (including start and end), this +1.
	ndivs = (int)(fabs(da) / (pi_d * 0.5) + 1.0);
	hda = (da / (double)ndivs) / 2.0;
	// Fix for ticket #179: division by 0: avoid cotangens around 0 (infinite)
	if ((hda < 1e-3) && (hda > -1e-3))
		hda *= 0.5;
	else
		hda = (1.0 - cos(hda)) / sin(hda);
	kappa = fabs(4.0 / 3.0 * hda);
	if (da < 0.0)
		kappa = -kappa;

	for (i = 0; i <= ndivs; i++) {
		a = a1 + da * ((double)i / (double)ndivs);
		dx = cos(a);
		dy = sin(a);

		// Inline double-precision matrix transform (position: xformPoint equivalent)
		double pxi = dx * rx * t[0] + dy * ry * t[2] + t[4];
		double pyi = dx * rx * t[1] + dy * ry * t[3] + t[5];
		// Inline double-precision matrix transform (tangent: xformVec equivalent)
		double tanxi = (-dy * rx * kappa) * t[0] + (dx * ry * kappa) * t[2];
		double tanyi = (-dy * rx * kappa) * t[1] + (dx * ry * kappa) * t[3];

		if (i > 0)
			msvg__cubicBezTo(p, (float)(px + ptanx), (float)(py + ptany),
			                    (float)(pxi - tanxi), (float)(pyi - tanyi),
			                    (float)pxi, (float)pyi);
		px = pxi;
		py = pyi;
		ptanx = tanxi;
		ptany = tanyi;
	}

	*cpx = (float)x2;
	*cpy = (float)y2;
}

static void msvg__parsePath(MSVGparser* p, const char** attr)
{
	const char* s = NULL;
	char cmd = '\0';
	float args[10];
	int nargs;
	int rargs = 0;
	char initPoint;
	float cpx, cpy, cpx2, cpy2;
	char closedFlag;
	int i;
	char item[64];

	msvg__parseAttribs(p, attr);

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "d") == 0) {
			s = attr[i + 1];
		}
	}

	if (s) {
		msvg__resetPath(p);
		cpx = 0; cpy = 0;
		cpx2 = 0; cpy2 = 0;
		initPoint = 0;
		closedFlag = 0;
		nargs = 0;

		while (*s) {
			item[0] = '\0';
			if ((cmd == 'A' || cmd == 'a') && (nargs == 3 || nargs == 4))
				s = msvg__getNextPathItemWhenArcFlag(s, item);
			if (!*item)
				s = msvg__getNextPathItem(s, item);
			if (!*item) break;
			if (cmd != '\0' && msvg__isCoordinate(item)) {
				if (nargs < 10)
					args[nargs++] = (float)msvg__atof(item);
				if (nargs >= rargs) {
					switch (cmd) {
						case 'm':
						case 'M':
							msvg__pathMoveTo(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
							// Moveto can be followed by multiple coordinate pairs,
							// which should be treated as linetos.
							cmd = (cmd == 'm') ? 'l' : 'L';
							rargs = msvg__getArgsPerElement(cmd);
							cpx2 = cpx; cpy2 = cpy;
							initPoint = 1;
							break;
						case 'l':
						case 'L':
							msvg__pathLineTo(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'H':
						case 'h':
							msvg__pathHLineTo(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'V':
						case 'v':
							msvg__pathVLineTo(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'C':
						case 'c':
							msvg__pathCubicBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
							break;
						case 'S':
						case 's':
							msvg__pathCubicBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
							break;
						case 'Q':
						case 'q':
							msvg__pathQuadBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
							break;
						case 'T':
						case 't':
							msvg__pathQuadBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 't' ? 1 : 0);
							break;
						case 'A':
						case 'a':
							msvg__pathArcTo(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						default:
							if (nargs >= 2) {
								cpx = args[nargs-2];
								cpy = args[nargs-1];
								cpx2 = cpx; cpy2 = cpy;
							}
							break;
					}
					nargs = 0;
				}
			} else {
				cmd = item[0];
				if (cmd == 'M' || cmd == 'm') {
					// Commit path.
					if (p->npts > 0)
						msvg__addPath(p, closedFlag);
					// Start new subpath.
					msvg__resetPath(p);
					closedFlag = 0;
					nargs = 0;
				} else if (initPoint == 0) {
					// Do not allow other commands until initial point has been set (moveTo called once).
					cmd = '\0';
				}
				if (cmd == 'Z' || cmd == 'z') {
					closedFlag = 1;
					// Commit path.
					if (p->npts > 0) {
						// Move current point to first point
						cpx = p->pts[0];
						cpy = p->pts[1];
						cpx2 = cpx; cpy2 = cpy;
						msvg__addPath(p, closedFlag);
					}
					// Start new subpath.
					msvg__resetPath(p);
					msvg__moveTo(p, cpx, cpy);
					closedFlag = 0;
					nargs = 0;
				}
				rargs = msvg__getArgsPerElement(cmd);
				if (rargs == -1) {
					// Command not recognized
					cmd = '\0';
					rargs = 0;
				}
			}
		}
		// Commit path.
		if (p->npts)
			msvg__addPath(p, closedFlag);
	}

	msvg__addShape(p);
}

static void msvg__parseRect(MSVGparser* p, const char** attr)
{
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;
	float rx = -1.0f; // marks not set
	float ry = -1.0f;
	int i;

	msvg__parseAttribs(p, attr);

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "x") == 0) x = msvg__parseCoordinate(p, attr[i+1], msvg__actualOrigX(p), msvg__actualWidth(p));
		else if (strcmp(attr[i], "y") == 0) y = msvg__parseCoordinate(p, attr[i+1], msvg__actualOrigY(p), msvg__actualHeight(p));
		else if (strcmp(attr[i], "width") == 0) w = msvg__parseCoordinate(p, attr[i+1], 0.0f, msvg__actualWidth(p));
		else if (strcmp(attr[i], "height") == 0) h = msvg__parseCoordinate(p, attr[i+1], 0.0f, msvg__actualHeight(p));
		else if (strcmp(attr[i], "rx") == 0) rx = fabsf(msvg__parseCoordinate(p, attr[i+1], 0.0f, msvg__actualWidth(p)));
		else if (strcmp(attr[i], "ry") == 0) ry = fabsf(msvg__parseCoordinate(p, attr[i+1], 0.0f, msvg__actualHeight(p)));
	}

	if (rx < 0.0f && ry > 0.0f) rx = ry;
	if (ry < 0.0f && rx > 0.0f) ry = rx;
	if (rx < 0.0f) rx = 0.0f;
	if (ry < 0.0f) ry = 0.0f;
	if (rx > w/2.0f) rx = w/2.0f;
	if (ry > h/2.0f) ry = h/2.0f;

	if (w != 0.0f && h != 0.0f) {
		msvg__resetPath(p);

		if (rx < 0.00001f || ry < 0.0001f) {
			msvg__moveTo(p, x, y);
			msvg__lineTo(p, x+w, y);
			msvg__lineTo(p, x+w, y+h);
			msvg__lineTo(p, x, y+h);
		} else {
			// Rounded rectangle
			msvg__moveTo(p, x+rx, y);
			msvg__lineTo(p, x+w-rx, y);
			msvg__cubicBezTo(p, x+w-rx*(1-MSVG_KAPPA90), y, x+w, y+ry*(1-MSVG_KAPPA90), x+w, y+ry);
			msvg__lineTo(p, x+w, y+h-ry);
			msvg__cubicBezTo(p, x+w, y+h-ry*(1-MSVG_KAPPA90), x+w-rx*(1-MSVG_KAPPA90), y+h, x+w-rx, y+h);
			msvg__lineTo(p, x+rx, y+h);
			msvg__cubicBezTo(p, x+rx*(1-MSVG_KAPPA90), y+h, x, y+h-ry*(1-MSVG_KAPPA90), x, y+h-ry);
			msvg__lineTo(p, x, y+ry);
			msvg__cubicBezTo(p, x, y+ry*(1-MSVG_KAPPA90), x+rx*(1-MSVG_KAPPA90), y, x+rx, y);
		}

		msvg__addPath(p, 1);

		msvg__addShape(p);
	}
}

static void msvg__parseCircle(MSVGparser* p, const char** attr)
{
	float cx = 0.0f;
	float cy = 0.0f;
	float r = 0.0f;
	int i;

	msvg__parseAttribs(p, attr);

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "cx") == 0) cx = msvg__parseCoordinate(p, attr[i+1], msvg__actualOrigX(p), msvg__actualWidth(p));
		else if (strcmp(attr[i], "cy") == 0) cy = msvg__parseCoordinate(p, attr[i+1], msvg__actualOrigY(p), msvg__actualHeight(p));
		else if (strcmp(attr[i], "r") == 0) r = fabsf(msvg__parseCoordinate(p, attr[i+1], 0.0f, msvg__actualLength(p)));
	}

	if (r > 0.0f) {
		msvg__resetPath(p);

		msvg__moveTo(p, cx+r, cy);
		msvg__cubicBezTo(p, cx+r, cy+r*MSVG_KAPPA90, cx+r*MSVG_KAPPA90, cy+r, cx, cy+r);
		msvg__cubicBezTo(p, cx-r*MSVG_KAPPA90, cy+r, cx-r, cy+r*MSVG_KAPPA90, cx-r, cy);
		msvg__cubicBezTo(p, cx-r, cy-r*MSVG_KAPPA90, cx-r*MSVG_KAPPA90, cy-r, cx, cy-r);
		msvg__cubicBezTo(p, cx+r*MSVG_KAPPA90, cy-r, cx+r, cy-r*MSVG_KAPPA90, cx+r, cy);

		msvg__addPath(p, 1);

		msvg__addShape(p);
	}
}

static void msvg__parseEllipse(MSVGparser* p, const char** attr)
{
	float cx = 0.0f;
	float cy = 0.0f;
	float rx = 0.0f;
	float ry = 0.0f;
	int i;

	msvg__parseAttribs(p, attr);

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "cx") == 0) cx = msvg__parseCoordinate(p, attr[i+1], msvg__actualOrigX(p), msvg__actualWidth(p));
		else if (strcmp(attr[i], "cy") == 0) cy = msvg__parseCoordinate(p, attr[i+1], msvg__actualOrigY(p), msvg__actualHeight(p));
		else if (strcmp(attr[i], "rx") == 0) rx = fabsf(msvg__parseCoordinate(p, attr[i+1], 0.0f, msvg__actualWidth(p)));
		else if (strcmp(attr[i], "ry") == 0) ry = fabsf(msvg__parseCoordinate(p, attr[i+1], 0.0f, msvg__actualHeight(p)));
	}

	if (rx > 0.0f && ry > 0.0f) {

		msvg__resetPath(p);

		msvg__moveTo(p, cx+rx, cy);
		msvg__cubicBezTo(p, cx+rx, cy+ry*MSVG_KAPPA90, cx+rx*MSVG_KAPPA90, cy+ry, cx, cy+ry);
		msvg__cubicBezTo(p, cx-rx*MSVG_KAPPA90, cy+ry, cx-rx, cy+ry*MSVG_KAPPA90, cx-rx, cy);
		msvg__cubicBezTo(p, cx-rx, cy-ry*MSVG_KAPPA90, cx-rx*MSVG_KAPPA90, cy-ry, cx, cy-ry);
		msvg__cubicBezTo(p, cx+rx*MSVG_KAPPA90, cy-ry, cx+rx, cy-ry*MSVG_KAPPA90, cx+rx, cy);

		msvg__addPath(p, 1);

		msvg__addShape(p);
	}
}

static void msvg__parseLine(MSVGparser* p, const char** attr)
{
	float x1 = 0.0;
	float y1 = 0.0;
	float x2 = 0.0;
	float y2 = 0.0;
	int i;

	msvg__parseAttribs(p, attr);

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "x1") == 0) x1 = msvg__parseCoordinate(p, attr[i + 1], msvg__actualOrigX(p), msvg__actualWidth(p));
		else if (strcmp(attr[i], "y1") == 0) y1 = msvg__parseCoordinate(p, attr[i + 1], msvg__actualOrigY(p), msvg__actualHeight(p));
		else if (strcmp(attr[i], "x2") == 0) x2 = msvg__parseCoordinate(p, attr[i + 1], msvg__actualOrigX(p), msvg__actualWidth(p));
		else if (strcmp(attr[i], "y2") == 0) y2 = msvg__parseCoordinate(p, attr[i + 1], msvg__actualOrigY(p), msvg__actualHeight(p));
	}

	msvg__resetPath(p);

	msvg__moveTo(p, x1, y1);
	msvg__lineTo(p, x2, y2);

	msvg__addPath(p, 0);

	msvg__addShape(p);
}

static void msvg__parsePoly(MSVGparser* p, const char** attr, int closeFlag)
{
	int i;
	const char* s;
	float args[2];
	int nargs, npts = 0;
	char item[64];

	msvg__parseAttribs(p, attr);

	msvg__resetPath(p);

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "points") == 0) {
			s = attr[i + 1];
			nargs = 0;
			while (*s) {
				s = msvg__getNextPathItem(s, item);
				args[nargs++] = (float)msvg__atof(item);
				if (nargs >= 2) {
					if (npts == 0)
						msvg__moveTo(p, args[0], args[1]);
					else
						msvg__lineTo(p, args[0], args[1]);
					nargs = 0;
					npts++;
				}
			}
		}
	}

	msvg__addPath(p, (char)closeFlag);

	msvg__addShape(p);
}

static void msvg__parseSVG(MSVGparser* p, const char** attr)
{
	int i;
	for (i = 0; attr[i]; i += 2) {
		if (!msvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "width") == 0) {
				p->image->width = msvg__parseCoordinate(p, attr[i + 1], 0.0f, 0.0f);
			} else if (strcmp(attr[i], "height") == 0) {
				p->image->height = msvg__parseCoordinate(p, attr[i + 1], 0.0f, 0.0f);
			} else if (strcmp(attr[i], "viewBox") == 0) {
				const char *s = attr[i + 1];
				char buf[64];
				s = msvg__parseNumber(s, buf, 64);
				p->viewMinx = msvg__atof(buf);
				while (*s && (msvg__isspace(*s) || *s == '%' || *s == ',')) s++;
				if (!*s) return;
				s = msvg__parseNumber(s, buf, 64);
				p->viewMiny = msvg__atof(buf);
				while (*s && (msvg__isspace(*s) || *s == '%' || *s == ',')) s++;
				if (!*s) return;
				s = msvg__parseNumber(s, buf, 64);
				p->viewWidth = msvg__atof(buf);
				while (*s && (msvg__isspace(*s) || *s == '%' || *s == ',')) s++;
				if (!*s) return;
				s = msvg__parseNumber(s, buf, 64);
				p->viewHeight = msvg__atof(buf);
			} else if (strcmp(attr[i], "preserveAspectRatio") == 0) {
				if (strstr(attr[i + 1], "none") != 0) {
					// No uniform scaling
					p->alignType = MSVG_ALIGN_NONE;
				} else {
					// Parse X align
					if (strstr(attr[i + 1], "xMin") != 0)
						p->alignX = MSVG_ALIGN_MIN;
					else if (strstr(attr[i + 1], "xMid") != 0)
						p->alignX = MSVG_ALIGN_MID;
					else if (strstr(attr[i + 1], "xMax") != 0)
						p->alignX = MSVG_ALIGN_MAX;
					// Parse X align
					if (strstr(attr[i + 1], "yMin") != 0)
						p->alignY = MSVG_ALIGN_MIN;
					else if (strstr(attr[i + 1], "yMid") != 0)
						p->alignY = MSVG_ALIGN_MID;
					else if (strstr(attr[i + 1], "yMax") != 0)
						p->alignY = MSVG_ALIGN_MAX;
					// Parse meet/slice
					p->alignType = MSVG_ALIGN_MEET;
					if (strstr(attr[i + 1], "slice") != 0)
						p->alignType = MSVG_ALIGN_SLICE;
				}
			}
		}
	}
}

static void msvg__parseGradient(MSVGparser* p, const char** attr, signed char type)
{
	int i;
	MSVGgradientData* grad = (MSVGgradientData*)msvg__parserAlloc(p->image, sizeof(MSVGgradientData));
	if (grad == NULL) return;
	memset(grad, 0, sizeof(MSVGgradientData));
	grad->units = MSVG_OBJECT_SPACE;
	grad->type = type;
	if (grad->type == MSVG_PAINT_LINEAR_GRADIENT) {
		grad->linear.x1 = msvg__coord(0.0f, MSVG_UNITS_PERCENT);
		grad->linear.y1 = msvg__coord(0.0f, MSVG_UNITS_PERCENT);
		grad->linear.x2 = msvg__coord(100.0f, MSVG_UNITS_PERCENT);
		grad->linear.y2 = msvg__coord(0.0f, MSVG_UNITS_PERCENT);
	} else if (grad->type == MSVG_PAINT_RADIAL_GRADIENT) {
		grad->radial.cx = msvg__coord(50.0f, MSVG_UNITS_PERCENT);
		grad->radial.cy = msvg__coord(50.0f, MSVG_UNITS_PERCENT);
		grad->radial.r = msvg__coord(50.0f, MSVG_UNITS_PERCENT);
	}

	msvg__xformIdentity(grad->xform);

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "id") == 0) {
			strncpy(grad->id, attr[i+1], 63);
			grad->id[63] = '\0';
		} else if (!msvg__parseAttr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "gradientUnits") == 0) {
				if (strcmp(attr[i+1], "objectBoundingBox") == 0)
					grad->units = MSVG_OBJECT_SPACE;
				else
					grad->units = MSVG_USER_SPACE;
			} else if (strcmp(attr[i], "gradientTransform") == 0) {
				msvg__parseTransform(grad->xform, attr[i + 1]);
			} else if (strcmp(attr[i], "cx") == 0) {
				grad->radial.cx = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "cy") == 0) {
				grad->radial.cy = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "r") == 0) {
				grad->radial.r = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "fx") == 0) {
				grad->radial.fx = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "fy") == 0) {
				grad->radial.fy = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "x1") == 0) {
				grad->linear.x1 = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "y1") == 0) {
				grad->linear.y1 = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "x2") == 0) {
				grad->linear.x2 = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "y2") == 0) {
				grad->linear.y2 = msvg__parseCoordinateRaw(attr[i + 1]);
			} else if (strcmp(attr[i], "spreadMethod") == 0) {
				if (strcmp(attr[i+1], "pad") == 0)
					grad->spread = MSVG_SPREAD_PAD;
				else if (strcmp(attr[i+1], "reflect") == 0)
					grad->spread = MSVG_SPREAD_REFLECT;
				else if (strcmp(attr[i+1], "repeat") == 0)
					grad->spread = MSVG_SPREAD_REPEAT;
			} else if (strcmp(attr[i], "xlink:href") == 0) {
				const char *href = attr[i+1];
				strncpy(grad->ref, href+1, 62);
				grad->ref[62] = '\0';
			}
		}
	}

	grad->stopCapacity = 8;
	grad->stops = (MSVGgradientStop*)msvg__parserAlloc(p->image, sizeof(MSVGgradientStop) * grad->stopCapacity);
	grad->next = p->gradients;
	p->gradients = grad;
}

static void msvg__parseGradientStop(MSVGparser* p, const char** attr)
{
	MSVGattrib* curAttr = msvg__getAttr(p);
	MSVGgradientData* grad;
	MSVGgradientStop* stop;
	int i, idx;

	curAttr->stopOffset = 0;
	curAttr->stopColor = 0;
	curAttr->stopOpacity = 1.0f;

	for (i = 0; attr[i]; i += 2) {
		msvg__parseAttr(p, attr[i], attr[i + 1]);
	}

	// Add stop to the last gradient.
	grad = p->gradients;
	if (grad == NULL) return;

	if (grad->nstops >= grad->stopCapacity) {
		// Grow stops array from pool.
		int newCapacity = grad->stopCapacity * 2;
		MSVGgradientStop* newStops = (MSVGgradientStop*)msvg__parserAlloc(p->image, sizeof(MSVGgradientStop) * newCapacity);
		if (newStops == NULL) return;
		for (i = 0; i < grad->nstops; i++)
			newStops[i] = grad->stops[i];
		grad->stops = newStops;
		grad->stopCapacity = newCapacity;
	}
	grad->nstops++;

	// Insert
	idx = grad->nstops-1;
	for (i = 0; i < grad->nstops-1; i++) {
		if (curAttr->stopOffset < grad->stops[i].offset) {
			idx = i;
			break;
		}
	}
	if (idx != grad->nstops-1) {
		for (i = grad->nstops-1; i > idx; i--)
			grad->stops[i] = grad->stops[i-1];
	}

	stop = &grad->stops[idx];
	stop->color = curAttr->stopColor;
	stop->color |= (unsigned int)(curAttr->stopOpacity*255) << 24;
	stop->offset = curAttr->stopOffset;
}

static void msvg__startElement(void* ud, const char* el, const char** attr)
{
	MSVGparser* p = (MSVGparser*)ud;

	if (strcmp(el, "style") == 0) {
		p->styleFlag = 1;
		return;
	}

	if (p->defsFlag) {
		// Inside defs: allow gradients, and shapes inside a symbol
		if (strcmp(el, "linearGradient") == 0) {
			msvg__parseGradient(p, attr, MSVG_PAINT_LINEAR_GRADIENT);
		} else if (strcmp(el, "radialGradient") == 0) {
			msvg__parseGradient(p, attr, MSVG_PAINT_RADIAL_GRADIENT);
		} else if (strcmp(el, "stop") == 0) {
			msvg__parseGradientStop(p, attr);
		} else if (strcmp(el, "symbol") == 0) {
			// Start a new symbol entry
			MSVGsymbol* sym = (MSVGsymbol*)msvg__parserAlloc(p->image, sizeof(MSVGsymbol));
			if (sym) {
				memset(sym, 0, sizeof(MSVGsymbol));
				msvg__parseAttribs(p, attr);
				MSVGattrib* curAttr = msvg__getAttr(p);
				memcpy(sym->id, curAttr->id, sizeof sym->id);
				sym->next = p->symbols;
				p->symbols = sym;
				p->curSymbol = sym;
				p->symbolTail = NULL;
			}
		} else if (p->curSymbol && (
			strcmp(el, "path") == 0 || strcmp(el, "rect") == 0 ||
			strcmp(el, "circle") == 0 || strcmp(el, "ellipse") == 0 ||
			strcmp(el, "line") == 0 || strcmp(el, "polyline") == 0 ||
			strcmp(el, "polygon") == 0 || strcmp(el, "g") == 0)) {
			// Parse shapes inside symbol (same as normal parsing)
			if (strcmp(el, "g") == 0) {
				msvg__pushAttr(p);
				msvg__parseAttribs(p, attr);
			} else if (strcmp(el, "path") == 0) {
				if (!p->pathFlag) {
					msvg__pushAttr(p);
					msvg__parsePath(p, attr);
					msvg__popAttr(p);
				}
			} else if (strcmp(el, "rect") == 0) {
				msvg__pushAttr(p);
				msvg__parseRect(p, attr);
				msvg__popAttr(p);
			} else if (strcmp(el, "circle") == 0) {
				msvg__pushAttr(p);
				msvg__parseCircle(p, attr);
				msvg__popAttr(p);
			} else if (strcmp(el, "ellipse") == 0) {
				msvg__pushAttr(p);
				msvg__parseEllipse(p, attr);
				msvg__popAttr(p);
			} else if (strcmp(el, "line") == 0) {
				msvg__pushAttr(p);
				msvg__parseLine(p, attr);
				msvg__popAttr(p);
			} else if (strcmp(el, "polyline") == 0) {
				msvg__pushAttr(p);
				msvg__parsePoly(p, attr, 0);
				msvg__popAttr(p);
			} else if (strcmp(el, "polygon") == 0) {
				msvg__pushAttr(p);
				msvg__parsePoly(p, attr, 1);
				msvg__popAttr(p);
			}
		}
		return;
	}

	if (strcmp(el, "g") == 0) {
		msvg__pushAttr(p);
		msvg__parseAttribs(p, attr);
	} else if (strcmp(el, "path") == 0) {
		if (p->pathFlag)	// Do not allow nested paths.
			return;
		msvg__pushAttr(p);
		msvg__parsePath(p, attr);
		msvg__popAttr(p);
	} else if (strcmp(el, "rect") == 0) {
		msvg__pushAttr(p);
		msvg__parseRect(p, attr);
		msvg__popAttr(p);
	} else if (strcmp(el, "circle") == 0) {
		msvg__pushAttr(p);
		msvg__parseCircle(p, attr);
		msvg__popAttr(p);
	} else if (strcmp(el, "ellipse") == 0) {
		msvg__pushAttr(p);
		msvg__parseEllipse(p, attr);
		msvg__popAttr(p);
	} else if (strcmp(el, "line") == 0)  {
		msvg__pushAttr(p);
		msvg__parseLine(p, attr);
		msvg__popAttr(p);
	} else if (strcmp(el, "polyline") == 0)  {
		msvg__pushAttr(p);
		msvg__parsePoly(p, attr, 0);
		msvg__popAttr(p);
	} else if (strcmp(el, "polygon") == 0)  {
		msvg__pushAttr(p);
		msvg__parsePoly(p, attr, 1);
		msvg__popAttr(p);
	} else  if (strcmp(el, "linearGradient") == 0) {
		msvg__parseGradient(p, attr, MSVG_PAINT_LINEAR_GRADIENT);
	} else if (strcmp(el, "radialGradient") == 0) {
		msvg__parseGradient(p, attr, MSVG_PAINT_RADIAL_GRADIENT);
	} else if (strcmp(el, "stop") == 0) {
		msvg__parseGradientStop(p, attr);
	} else if (strcmp(el, "use") == 0) {
		msvg__parseUse(p, attr);
	} else if (strcmp(el, "defs") == 0) {
		p->defsFlag = 1;
	} else if (strcmp(el, "svg") == 0) {
		msvg__parseSVG(p, attr);
	}
}

static void msvg__endElement(void* ud, const char* el)
{
	MSVGparser* p = (MSVGparser*)ud;

	if (strcmp(el, "g") == 0) {
		msvg__popAttr(p);
	} else if (strcmp(el, "path") == 0) {
		p->pathFlag = 0;
	} else if (strcmp(el, "symbol") == 0) {
		p->curSymbol = NULL;
		p->symbolTail = NULL;
	} else if (strcmp(el, "style") == 0) {
		p->styleFlag = 0;
	} else if (strcmp(el, "defs") == 0) {
		p->defsFlag = 0;
	}
}

static void msvg__content(void* ud, const char* s)
{
	MSVGparser* p = (MSVGparser*)ud;
	if (p->styleFlag) {
		msvg__parseCSSStyles(p, (char*)s);
	}
}

static void msvg__imageBounds(MSVGparser* p, float* bounds)
{
	MSVGshape* shape;
	shape = p->image->shapes;
	if (shape == NULL) {
		bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0;
		return;
	}
	bounds[0] = shape->bounds[0];
	bounds[1] = shape->bounds[1];
	bounds[2] = shape->bounds[2];
	bounds[3] = shape->bounds[3];
	for (shape = shape->next; shape != NULL; shape = shape->next) {
		bounds[0] = msvg__minf(bounds[0], shape->bounds[0]);
		bounds[1] = msvg__minf(bounds[1], shape->bounds[1]);
		bounds[2] = msvg__maxf(bounds[2], shape->bounds[2]);
		bounds[3] = msvg__maxf(bounds[3], shape->bounds[3]);
	}
}

static float msvg__viewAlign(float content, float container, int type)
{
	if (type == MSVG_ALIGN_MIN)
		return 0;
	else if (type == MSVG_ALIGN_MAX)
		return container - content;
	// mid
	return (container - content) * 0.5f;
}

static void msvg__scaleGradient(MSVGgradient* grad, float tx, float ty, float sx, float sy)
{
	float t[6];
	msvg__xformSetTranslation(t, tx, ty);
	msvg__xformMultiply (grad->xform, t);

	msvg__xformSetScale(t, sx, sy);
	msvg__xformMultiply (grad->xform, t);
}

static void msvg__scaleToViewbox(MSVGparser* p, const char* units)
{
	MSVGshape* shape;
	MSVGpath* path;
	float tx, ty, sx, sy, us, bounds[4], t[6], avgs;
	int i;
	float* pt;

	// Guess image size if not set completely.
	msvg__imageBounds(p, bounds);

	if (p->viewWidth == 0) {
		if (p->image->width > 0) {
			p->viewWidth = p->image->width;
		} else {
			p->viewMinx = bounds[0];
			p->viewWidth = bounds[2] - bounds[0];
		}
	}
	if (p->viewHeight == 0) {
		if (p->image->height > 0) {
			p->viewHeight = p->image->height;
		} else {
			p->viewMiny = bounds[1];
			p->viewHeight = bounds[3] - bounds[1];
		}
	}
	if (p->image->width == 0)
		p->image->width = p->viewWidth;
	if (p->image->height == 0)
		p->image->height = p->viewHeight;

	tx = -p->viewMinx;
	ty = -p->viewMiny;
	sx = p->viewWidth > 0 ? p->image->width / p->viewWidth : 0;
	sy = p->viewHeight > 0 ? p->image->height / p->viewHeight : 0;
	// Unit scaling
	us = 1.0f / msvg__convertToPixels(p, msvg__coord(1.0f, msvg__parseUnits(units)), 0.0f, 1.0f);

	// Fix aspect ratio
	if (p->alignType == MSVG_ALIGN_MEET) {
		// fit whole image into viewbox
		sx = sy = msvg__minf(sx, sy);
		tx += msvg__viewAlign(p->viewWidth*sx, p->image->width, p->alignX) / sx;
		ty += msvg__viewAlign(p->viewHeight*sy, p->image->height, p->alignY) / sy;
	} else if (p->alignType == MSVG_ALIGN_SLICE) {
		// fill whole viewbox with image
		sx = sy = msvg__maxf(sx, sy);
		tx += msvg__viewAlign(p->viewWidth*sx, p->image->width, p->alignX) / sx;
		ty += msvg__viewAlign(p->viewHeight*sy, p->image->height, p->alignY) / sy;
	}

	// Transform
	sx *= us;
	sy *= us;
	avgs = (sx+sy) / 2.0f;
	for (shape = p->image->shapes; shape != NULL; shape = shape->next) {
		shape->bounds[0] = (shape->bounds[0] + tx) * sx;
		shape->bounds[1] = (shape->bounds[1] + ty) * sy;
		shape->bounds[2] = (shape->bounds[2] + tx) * sx;
		shape->bounds[3] = (shape->bounds[3] + ty) * sy;
		for (path = shape->paths; path != NULL; path = path->next) {
			path->bounds[0] = (path->bounds[0] + tx) * sx;
			path->bounds[1] = (path->bounds[1] + ty) * sy;
			path->bounds[2] = (path->bounds[2] + tx) * sx;
			path->bounds[3] = (path->bounds[3] + ty) * sy;
			for (i =0; i < path->npts; i++) {
				pt = &path->pts[i*2];
				pt[0] = (pt[0] + tx) * sx;
				pt[1] = (pt[1] + ty) * sy;
			}
		}

		if (shape->fill.type == MSVG_PAINT_LINEAR_GRADIENT || shape->fill.type == MSVG_PAINT_RADIAL_GRADIENT) {
			msvg__scaleGradient(shape->fill.gradient, tx,ty, sx,sy);
			memcpy(t, shape->fill.gradient->xform, sizeof(float)*6);
			msvg__xformInverse(shape->fill.gradient->xform, t);
		}
		if (shape->stroke.type == MSVG_PAINT_LINEAR_GRADIENT || shape->stroke.type == MSVG_PAINT_RADIAL_GRADIENT) {
			msvg__scaleGradient(shape->stroke.gradient, tx,ty, sx,sy);
			memcpy(t, shape->stroke.gradient->xform, sizeof(float)*6);
			msvg__xformInverse(shape->stroke.gradient->xform, t);
		}

		shape->strokeWidth *= avgs;
		shape->strokeDashOffset *= avgs;
		for (i = 0; i < shape->strokeDashCount; i++)
			shape->strokeDashArray[i] *= avgs;
	}
}

static void msvg__createGradients(MSVGparser* p)
{
	MSVGshape* shape;

	for (shape = p->image->shapes; shape != NULL; shape = shape->next) {
		if (shape->fill.type == MSVG_PAINT_UNDEF) {
			if (shape->fillGradient[0] != '\0') {
				float inv[6], localBounds[4];
				msvg__xformInverse(inv, shape->xform);
				msvg__getLocalBounds(localBounds, shape, inv);
				shape->fill.gradient = msvg__createGradient(p, shape->fillGradient, localBounds, shape->xform, &shape->fill.type);
			}
			if (shape->fill.type == MSVG_PAINT_UNDEF) {
				shape->fill.type = MSVG_PAINT_NONE;
			}
		}
		if (shape->stroke.type == MSVG_PAINT_UNDEF) {
			if (shape->strokeGradient[0] != '\0') {
				float inv[6], localBounds[4];
				msvg__xformInverse(inv, shape->xform);
				msvg__getLocalBounds(localBounds, shape, inv);
				shape->stroke.gradient = msvg__createGradient(p, shape->strokeGradient, localBounds, shape->xform, &shape->stroke.type);
			}
			if (shape->stroke.type == MSVG_PAINT_UNDEF) {
				shape->stroke.type = MSVG_PAINT_NONE;
			}
		}
	}
}

MSVGimage* msvgParse(char* input, const char* units, float dpi)
{
	MSVGparser* p;
	MSVGimage* ret = 0;

	p = msvg__createParser();
	if (p == NULL) {
		return NULL;
	}
	p->dpi = dpi;

	msvg__parseXML(input, msvg__startElement, msvg__endElement, msvg__content, p);

	// Create gradients after all definitions have been parsed
	msvg__createGradients(p);

	// Scale to viewBox
	msvg__scaleToViewbox(p, units);

	ret = p->image;
	p->image = NULL;

	msvg__deleteParser(p);

	return ret;
}

MSVGimage* msvgParseFromFile(const char* filename, const char* units, float dpi)
{
	FILE* fp = NULL;
	size_t size;
	char* data = NULL;
	MSVGimage* image = NULL;

	fp = fopen(filename, "rb");
	if (!fp) goto error;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)malloc(size+1);
	if (data == NULL) goto error;
	if (fread(data, 1, size, fp) != size) goto error;
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	image = msvgParse(data, units, dpi);
	free(data);

	return image;

error:
	if (fp) fclose(fp);
	if (data) free(data);
	if (image) msvgDelete(image);
	return NULL;
}

MSVGpath* msvgDuplicatePath(MSVGpath* p)
{
    MSVGpath* res = NULL;

    if (p == NULL)
        return NULL;

    res = (MSVGpath*)malloc(sizeof(MSVGpath));
    if (res == NULL) goto error;
    memset(res, 0, sizeof(MSVGpath));

    res->pts = (float*)malloc(p->npts*2*sizeof(float));
    if (res->pts == NULL) goto error;
    memcpy(res->pts, p->pts, p->npts * sizeof(float) * 2);
    res->npts = p->npts;

    memcpy(res->bounds, p->bounds, sizeof(p->bounds));

    res->closed = p->closed;

    return res;

error:
    if (res != NULL) {
        free(res->pts);
        free(res);
    }
    return NULL;
}

void msvgDelete(MSVGimage* image)
{
	MSVGparserPage* page;
	if (image == NULL) return;
	page = image->pages;
	while (page) {
		MSVGparserPage* next = page->next;
		if (page->mem) free(page->mem);
		free(page);
		page = next;
	}
	free(image);
}

#endif // MOONSVG_IMPLEMENTATION

#endif // MOONSVG_H
