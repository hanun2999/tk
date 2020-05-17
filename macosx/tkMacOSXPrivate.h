/*
 * tkMacOSXPrivate.h --
 *
 *	Macros and declarations that are purely internal & private to TkAqua.
 *
 * Copyright (c) 2005-2009 Daniel A. Steffen <das@users.sourceforge.net>
 * Copyright 2008-2009, Apple Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id$
 */

#ifndef _TKMACPRIV
#define _TKMACPRIV

#if !__OBJC__
#error Objective-C compiler required
#endif

#ifndef __clang__
#define instancetype id
#endif

#define TextStyle MacTextStyle
#import <ApplicationServices/ApplicationServices.h>
#import <Cocoa/Cocoa.h>
#ifndef NO_CARBON_H
#import <Carbon/Carbon.h>
#endif
#undef TextStyle
#import <objc/runtime.h> /* for sel_isEqual() */

#ifndef _TKMACINT
#include "tkMacOSXInt.h"
#endif
#ifndef _TKMACDEFAULT
#include "tkMacOSXDefault.h"
#endif

/* Macros for Mac OS X API availability checking */
#define TK_IF_MAC_OS_X_API(vers, symbol, ...) \
	tk_if_mac_os_x_10_##vers(symbol != NULL, 1, __VA_ARGS__)
#define TK_ELSE_MAC_OS_X(vers, ...) \
	tk_else_mac_os_x_10_##vers(__VA_ARGS__)
#define TK_IF_MAC_OS_X_API_COND(vers, symbol, cond, ...) \
	tk_if_mac_os_x_10_##vers(symbol != NULL, cond, __VA_ARGS__)
#define TK_ELSE(...) \
	} else { __VA_ARGS__
#define TK_ENDIF \
	}
/* Private macros that implement the checking macros above */
#define tk_if_mac_os_x_yes(chk, cond, ...) \
	if (cond) { __VA_ARGS__
#define tk_else_mac_os_x_yes(...) \
	} else {
#define tk_if_mac_os_x_chk(chk, cond, ...) \
	if ((chk) && (cond)) { __VA_ARGS__
#define tk_else_mac_os_x_chk(...) \
	} else { __VA_ARGS__
#define tk_if_mac_os_x_no(chk, cond, ...) \
	if (0) {
#define tk_else_mac_os_x_no(...) \
	} else { __VA_ARGS__

/*
 * Macros for DEBUG_ASSERT_MESSAGE et al from Debugging.h.
 */

#undef kComponentSignatureString
#undef COMPONENT_SIGNATURE
#define kComponentSignatureString "TkMacOSX"
#define COMPONENT_SIGNATURE 'Tk  '

/*
 * Macros abstracting checks only active in a debug build.
 */

#ifdef TK_MAC_DEBUG
#define TKLog(f, ...) NSLog(f, ##__VA_ARGS__)

/*
 * Macro to do debug message output.
 */
#define TkMacOSXDbgMsg(m, ...) \
    do { \
	TKLog(@"%s:%d: %s(): " m, strrchr(__FILE__, '/')+1, \
		__LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

/*
 * Macro to do debug API failure message output.
 */
#define TkMacOSXDbgOSErr(f, err) \
    do { \
	TkMacOSXDbgMsg("%s failed: %d", #f, (int)(err)); \
    } while (0)

/*
 * Macro to do very common check for noErr return from given API and output
 * debug message in case of failure.
 */
#define ChkErr(f, ...) ({ \
	OSStatus err = f(__VA_ARGS__); \
	if (err != noErr) { \
	    TkMacOSXDbgOSErr(f, err); \
	} \
	err;})

#else /* TK_MAC_DEBUG */
#define TKLog(f, ...)
#define TkMacOSXDbgMsg(m, ...)
#define TkMacOSXDbgOSErr(f, err)
#define ChkErr(f, ...) ({f(__VA_ARGS__);})
#endif /* TK_MAC_DEBUG */

/*
 * Macro abstracting use of TkMacOSXGetNamedSymbol to init named symbols.
 */

#define UNINITIALISED_SYMBOL	((void*)(-1L))
#define TkMacOSXInitNamedSymbol(module, ret, symbol, ...) \
    static ret (* symbol)(__VA_ARGS__) = UNINITIALISED_SYMBOL; \
    if (symbol == UNINITIALISED_SYMBOL) { \
	symbol = TkMacOSXGetNamedSymbol(STRINGIFY(module), \
		STRINGIFY(symbol)); \
    }

/*
 *  The structure of a 32-bit XEvent keycode on macOS. It may be viewed as
 *  an unsigned int or as having either two or three bitfields.
 */

typedef struct keycode_v_t {
    unsigned keychar: 22;    /* UCS-32 character */
    unsigned o_s: 2;         /* State of Option and Shift keys. */
    unsigned virtual: 8;     /* 8-bit virtual keycode - identifies a key. */
} keycode_v;

typedef struct keycode_x_t {
    unsigned keychar: 22;     /* UCS-32 character */
    unsigned xvirtual: 10;    /* Combines o_s and virtual. This 10-bit integer
			       * is used as a key for looking up the character
			       * produced when pressing a key with a particular
			       * Shift and Option modifier state. */
} keycode_x;

typedef union MacKeycode_t {
  unsigned int uint;
  keycode_v v;
  keycode_x x;
} MacKeycode;

/*
 * Macros used in tkMacOSXKeyboard.c and tkMacOSXKeyEvent.c.
 * Note that 0x7f is del and 0xF8FF is the Apple Logo character.
 */

#define ON_KEYPAD(virtual) ((virtual >= 0x41) && (virtual <= 0x5C))
#define IS_PRINTABLE(keychar) ((keychar >= 0x20) && (keychar != 0x7f) && \
                               ((keychar < 0xF700) || keychar >= 0xF8FF))

/*
 * An "index" is 2-bit bitfield showing the state of the Option and Shift
 * keys.  It is used as an index when building the keymaps and it
 * is the value of the o_s bitfield of a keycode_v.
 */

#define INDEX_SHIFT 1
#define INDEX_OPTION 2
#define INDEX2STATE(index) ((index & INDEX_SHIFT ? ShiftMask : 0) |	\
			    (index & INDEX_OPTION ? Mod2Mask : 0))
#define INDEX2CARBON(index) ((index & INDEX_SHIFT ? shiftKey : 0) |	\
			     (index & INDEX_OPTION ? optionKey : 0))
#define STATE2INDEX(state) ((state & ShiftMask ? INDEX_SHIFT : 0) |	\
			    (state & Mod2Mask ? INDEX_OPTION : 0))

/*
 * Special values for the virtual bitfield.  Actual virtual keycodes are < 128.
 */

#define NO_VIRTUAL 0xFF /* Not generated by a key or the NSText"InputClient. */
#define REPLACEMENT_VIRTUAL 0x80 /* A BMP char sent by the NSTextInputClient. */
#define NON_BMP_VIRTUAL 0x81 /* A non-BMP char sent by the NSTextInputClient. */

/*
 * A special character is used in the keycode for simulated modifier KeyPress
 * or KeyRelease XEvents.  It is near the end of the private-use range but
 * different from the UniChar 0xF8FF which Apple uses for their logo character.
 * A different special character is used for keys, like the Menu key, which do
 * not appear on Macintosh keyboards.
 */

#define MOD_KEYCHAR 0xF8FE
#define UNKNOWN_KEYCHAR 0xF8FD

/*
 * Structure encapsulating current drawing environment.
 */

typedef struct TkMacOSXDrawingContext {
    CGContextRef context;
    NSView *view;
    HIShapeRef clipRgn;
    CGRect portBounds;
} TkMacOSXDrawingContext;

/*
 * Variables internal to TkAqua.
 */

MODULE_SCOPE long tkMacOSXMacOSXVersion;
MODULE_SCOPE bool tkMacOSXIsHandlingIdleEventsBeforeDrawing;

/*
 * Prototypes for TkMacOSXRegion.c.
 */

MODULE_SCOPE HIShapeRef	TkMacOSXGetNativeRegion(Region r);
MODULE_SCOPE void	TkMacOSXSetWithNativeRegion(Region r,
			    HIShapeRef rgn);
MODULE_SCOPE HIShapeRef	TkMacOSXHIShapeCreateEmpty(void);
MODULE_SCOPE HIMutableShapeRef TkMacOSXHIShapeCreateMutableWithRect(
			    const CGRect *inRect);
MODULE_SCOPE OSStatus	TkMacOSXHIShapeSetWithShape(
			    HIMutableShapeRef inDestShape,
			    HIShapeRef inSrcShape);
#if 0
MODULE_SCOPE OSStatus	TkMacOSXHIShapeSetWithRect(HIMutableShapeRef inShape,
			    const CGRect *inRect);
#endif
MODULE_SCOPE OSStatus	TkMacOSHIShapeDifferenceWithRect(
			    HIMutableShapeRef inShape, const CGRect *inRect);
MODULE_SCOPE OSStatus	TkMacOSHIShapeUnionWithRect(HIMutableShapeRef inShape,
			    const CGRect *inRect);
MODULE_SCOPE OSStatus	TkMacOSHIShapeUnion(HIShapeRef inShape1,
			    HIShapeRef inShape2, HIMutableShapeRef outResult);

/*
 * Prototypes of TkAqua internal procs.
 */

MODULE_SCOPE void *	TkMacOSXGetNamedSymbol(const char *module,
			    const char *symbol);
MODULE_SCOPE void	TkMacOSXDisplayChanged(Display *display);
MODULE_SCOPE CGFloat	TkMacOSXZeroScreenHeight();
MODULE_SCOPE CGFloat	TkMacOSXZeroScreenTop();
MODULE_SCOPE int	TkMacOSXUseAntialiasedText(Tcl_Interp *interp,
			    int enable);
MODULE_SCOPE int	TkMacOSXInitCGDrawing(Tcl_Interp *interp, int enable,
			    int antiAlias);
MODULE_SCOPE int	TkMacOSXGenerateFocusEvent(TkWindow *winPtr,
			    int activeFlag);
MODULE_SCOPE WindowClass TkMacOSXWindowClass(TkWindow *winPtr);
MODULE_SCOPE int	TkMacOSXIsWindowZoomed(TkWindow *winPtr);
MODULE_SCOPE int	TkGenerateButtonEventForXPointer(Window window);
MODULE_SCOPE EventModifiers TkMacOSXModifierState(void);
MODULE_SCOPE NSBitmapImageRep* TkMacOSXBitmapRepFromDrawableRect(Drawable drawable,
			    int x, int y, unsigned int width, unsigned int height);
MODULE_SCOPE CGImageRef TkMacOSXCreateCGImageWithXImage(XImage *image);
MODULE_SCOPE void       TkMacOSXDrawCGImage(Drawable d, GC gc, CGContextRef context,
			    CGImageRef image, unsigned long imageForeground,
			    unsigned long imageBackground, CGRect imageBounds,
			    CGRect srcBounds, CGRect dstBounds);
MODULE_SCOPE int	TkMacOSXSetupDrawingContext(Drawable d, GC gc,
			    int useCG, TkMacOSXDrawingContext *dcPtr);
MODULE_SCOPE void	TkMacOSXRestoreDrawingContext(
			    TkMacOSXDrawingContext *dcPtr);
MODULE_SCOPE void	TkMacOSXSetColorInContext(GC gc, unsigned long pixel,
			    CGContextRef context);
MODULE_SCOPE int	TkMacOSXMakeFullscreen(TkWindow *winPtr,
			    NSWindow *window, int fullscreen,
			    Tcl_Interp *interp);
MODULE_SCOPE void	TkMacOSXEnterExitFullscreen(TkWindow *winPtr,
			    int active);
MODULE_SCOPE NSWindow*	TkMacOSXDrawableWindow(Drawable drawable);
MODULE_SCOPE NSView*	TkMacOSXDrawableView(MacDrawable *macWin,
			    MacDrawable **viewMacWinPtr);
MODULE_SCOPE void	TkMacOSXWinCGBounds(TkWindow *winPtr, CGRect *bounds);
MODULE_SCOPE HIShapeRef	TkMacOSXGetClipRgn(Drawable drawable);
MODULE_SCOPE void	TkMacOSXInvalidateViewRegion(NSView *view,
			    HIShapeRef rgn);
MODULE_SCOPE CGContextRef TkMacOSXGetCGContextForDrawable(Drawable drawable);
MODULE_SCOPE CGImageRef	TkMacOSXCreateCGImageWithDrawable(Drawable drawable);
MODULE_SCOPE NSImage*	TkMacOSXGetNSImageWithTkImage(Display *display,
			    Tk_Image image, int width, int height);
MODULE_SCOPE NSImage*	TkMacOSXGetNSImageWithBitmap(Display *display,
			    Pixmap bitmap, GC gc, int width, int height);
MODULE_SCOPE CGColorRef	TkMacOSXCreateCGColor(GC gc, unsigned long pixel);
MODULE_SCOPE NSColor*	TkMacOSXGetNSColor(GC gc, unsigned long pixel);
MODULE_SCOPE TkWindow*	TkMacOSXGetTkWindow(NSWindow *w);
MODULE_SCOPE NSFont*	TkMacOSXNSFontForFont(Tk_Font tkfont);
MODULE_SCOPE NSDictionary* TkMacOSXNSFontAttributesForFont(Tk_Font tkfont);
MODULE_SCOPE NSModalSession TkMacOSXGetModalSession(void);
MODULE_SCOPE void	TkMacOSXSelDeadWindow(TkWindow *winPtr);
MODULE_SCOPE void	TkMacOSXApplyWindowAttributes(TkWindow *winPtr,
			    NSWindow *macWindow);
MODULE_SCOPE int	TkMacOSXStandardAboutPanelObjCmd(ClientData clientData,
			    Tcl_Interp *interp, int objc,
			    Tcl_Obj *const objv[]);
MODULE_SCOPE int	TkMacOSXIconBitmapObjCmd(ClientData clientData,
			    Tcl_Interp *interp, int objc,
			    Tcl_Obj *const objv[]);
MODULE_SCOPE void       TkMacOSXDrawSolidBorder(Tk_Window tkwin, GC gc,
			    int inset, int thickness);
MODULE_SCOPE int 	TkMacOSXServices_Init(Tcl_Interp *interp);
MODULE_SCOPE int	TkMacOSXRegisterServiceWidgetObjCmd(ClientData clientData,
			    Tcl_Interp *interp, int objc,
			    Tcl_Obj *const objv[]);
MODULE_SCOPE NSString*  TkUtfToNSString(const char *source, size_t numBytes);
MODULE_SCOPE int        TkUtfAtIndex(NSString *string, int index, char *uni,
				      unsigned int *code);
MODULE_SCOPE char*      TkNSStringToUtf(NSString *string, int *numBytes);
MODULE_SCOPE unsigned   TkMacOSXAddVirtual(unsigned int keycode);

#pragma mark Private Objective-C Classes

#define VISIBILITY_HIDDEN __attribute__((visibility("hidden")))

enum { tkMainMenu = 1, tkApplicationMenu, tkWindowsMenu, tkHelpMenu};

VISIBILITY_HIDDEN
@interface TKMenu : NSMenu {
@private
    void *_tkMenu;
    NSUInteger _tkOffset, _tkItemCount, _tkSpecial;
}
- (void)setSpecial:(NSUInteger)special;
- (BOOL)isSpecial:(NSUInteger)special;
@end

@interface TKMenu(TKMenuDelegate) <NSMenuDelegate>
@end

VISIBILITY_HIDDEN
@interface TKApplication : NSApplication {
@private
    Tcl_Interp *_eventInterp;
    NSMenu *_servicesMenu;
    TKMenu *_defaultMainMenu, *_defaultApplicationMenu;
    NSMenuItem *_demoMenuItem;
    NSArray *_defaultApplicationMenuItems, *_defaultWindowsMenuItems;
    NSArray *_defaultHelpMenuItems, *_defaultFileMenuItems;
    NSAutoreleasePool *_mainPool;
#ifdef __i386__
    /* The Objective C runtime used on i386 requires this. */
    int _poolLock;
    int _macMinorVersion;
    Bool _isDrawing;
#endif
}
@property int poolLock;
@property int macMinorVersion;
@property Bool isDrawing;

@end
@interface TKApplication(TKInit)
- (NSString *)tkFrameworkImagePath:(NSString*)image;
- (void)_resetAutoreleasePool;
- (void)_lockAutoreleasePool;
- (void)_unlockAutoreleasePool;
@end
@interface TKApplication(TKKeyboard)
- (void) keyboardChanged: (NSNotification *) notification;
@end
@interface TKApplication(TKWindowEvent) <NSApplicationDelegate>
- (void) _setupWindowNotifications;
@end
@interface TKApplication(TKDialog) <NSOpenSavePanelDelegate>
@end
@interface TKApplication(TKMenu)
- (void)tkSetMainMenu:(TKMenu *)menu;
@end
@interface TKApplication(TKMenus)
- (void) _setupMenus;
@end
@interface NSApplication(TKNotify)
/* We need to declare this hidden method. */
- (void) _modalSession: (NSModalSession) session sendEvent: (NSEvent *) event;
@end
@interface TKApplication(TKEvent)
- (NSEvent *)tkProcessEvent:(NSEvent *)theEvent;
@end
@interface TKApplication(TKMouseEvent)
- (NSEvent *)tkProcessMouseEvent:(NSEvent *)theEvent;
@end
@interface TKApplication(TKKeyEvent)
- (NSEvent *)tkProcessKeyEvent:(NSEvent *)theEvent;
@end
@interface TKApplication(TKClipboard)
- (void)tkProvidePasteboard:(TkDisplay *)dispPtr;
- (void)tkCheckPasteboard;
@end
@interface TKApplication(TKHLEvents)
- (void) terminate: (id) sender;
- (void) preferences: (id) sender;
- (void) handleQuitApplicationEvent:   (NSAppleEventDescriptor *)event
		     withReplyEvent:   (NSAppleEventDescriptor *)replyEvent;
- (void) handleOpenApplicationEvent:   (NSAppleEventDescriptor *)event
		     withReplyEvent:   (NSAppleEventDescriptor *)replyEvent;
- (void) handleReopenApplicationEvent: (NSAppleEventDescriptor *)event
		       withReplyEvent: (NSAppleEventDescriptor *)replyEvent;
- (void) handleShowPreferencesEvent:   (NSAppleEventDescriptor *)event
		     withReplyEvent:   (NSAppleEventDescriptor *)replyEvent;
- (void) handleOpenDocumentsEvent:     (NSAppleEventDescriptor *)event
		   withReplyEvent:     (NSAppleEventDescriptor *)replyEvent;
- (void) handlePrintDocumentsEvent:    (NSAppleEventDescriptor *)event
		   withReplyEvent:     (NSAppleEventDescriptor *)replyEvent;
- (void) handleDoScriptEvent:          (NSAppleEventDescriptor *)event
		   withReplyEvent:     (NSAppleEventDescriptor *)replyEvent;
- (void)handleURLEvent:                (NSAppleEventDescriptor*)event
	           withReplyEvent:     (NSAppleEventDescriptor*)replyEvent;
@end

VISIBILITY_HIDDEN
/*
 * Subclass TKContentView from NSTextInputClient to enable composition and
 * input from the Character Palette.
 */

@interface TKContentView : NSView <NSTextInputClient>
{
@private
    NSString *privateWorkingText;
    Bool _needsRedisplay;
}
@property Bool needsRedisplay;
@end

@interface TKContentView(TKKeyEvent)
- (void) deleteWorkingText;
- (void) cancelComposingText;
@end

@interface TKContentView(TKWindowEvent)
- (void) generateExposeEvents: (HIShapeRef) shape;
- (void) tkToolbarButton: (id) sender;
@end

@interface NSWindow(TKWm)
- (NSPoint) tkConvertPointToScreen:(NSPoint)point;
- (NSPoint) tkConvertPointFromScreen:(NSPoint)point;
@end

VISIBILITY_HIDDEN
@interface TKWindow : NSWindow
@end

@interface TKWindow(TKWm)
- (void)    tkLayoutChanged;
@end

@interface NSDrawerWindow : NSWindow
{
    id _i1, _i2;
}
@end

#pragma mark NSMenu & NSMenuItem Utilities

@interface NSMenu(TKUtils)
+ (id)menuWithTitle:(NSString *)title;
+ (id)menuWithTitle:(NSString *)title menuItems:(NSArray *)items;
+ (id)menuWithTitle:(NSString *)title submenus:(NSArray *)submenus;
- (NSMenuItem *)itemWithSubmenu:(NSMenu *)submenu;
- (NSMenuItem *)itemInSupermenu;
@end

@interface NSMenuItem(TKUtils)
+ (id)itemWithSubmenu:(NSMenu *)submenu;
+ (id)itemWithTitle:(NSString *)title submenu:(NSMenu *)submenu;
+ (id)itemWithTitle:(NSString *)title action:(SEL)action;
+ (id)itemWithTitle:(NSString *)title action:(SEL)action
	target:(id)target;
+ (id)itemWithTitle:(NSString *)title action:(SEL)action
	keyEquivalent:(NSString *)keyEquivalent;
+ (id)itemWithTitle:(NSString *)title action:(SEL)action
	target:(id)target keyEquivalent:(NSString *)keyEquivalent;
+ (id)itemWithTitle:(NSString *)title action:(SEL)action
	keyEquivalent:(NSString *)keyEquivalent
	keyEquivalentModifierMask:(NSUInteger)keyEquivalentModifierMask;
+ (id)itemWithTitle:(NSString *)title action:(SEL)action
	target:(id)target keyEquivalent:(NSString *)keyEquivalent
	keyEquivalentModifierMask:(NSUInteger)keyEquivalentModifierMask;
@end

@interface NSColorPanel(TKDialog)
- (void) _setUseModalAppearance: (BOOL) flag;
@end

@interface NSFont(TKFont)
- (NSFont *) bestMatchingFontForCharacters: (const UTF16Char *) characters
	length: (NSUInteger) length attributes: (NSDictionary *) attributes
	actualCoveredLength: (NSUInteger *) coveredLength;
@end

/*
 * This method of NSApplication is not declared in NSApplication.h so we
 * declare it here to be a method of the TKMenu category.
 */

@interface NSApplication(TKMenu)
- (void) setAppleMenu: (NSMenu *) menu;
@end

/*
 * These methods are exposed because they are needed to prevent zombie windows
 * on systems with a TouchBar.  The TouchBar Key-Value observer holds a
 * reference to the key window, which prevents deallocation of the key window
 * when it is closed.
 */

@interface NSApplication(TkWm)
- (id) _setKeyWindow: (NSWindow *) window;
- (id) _setMainWindow: (NSWindow *) window;
@end


/*
 *---------------------------------------------------------------------------
 *
 * TKNSString --
 *
 * When Tcl is compiled with TCL_UTF_MAX = 3 (the default for 8.6) it cannot
 * deal directly with UTF-8 encoded non-BMP characters, since their UTF-8
 * encoding requires 4 bytes.
 *
 * As a workaround, these versions of Tcl encode non-BMP characters as a string
 * of length 6 in which the high and low UTF-16 surrogates have been encoded
 * using the UTF-8 algorithm.  The UTF-8 encoding does not allow encoding
 * surrogates, so these 6-byte strings are not valid UTF-8, and hence Apple's
 * NString class will refuse to instantiate an NSString from the 6-byte
 * encoding.
 *
 * This subclass of NSString adds a new initialization method which accepts
 * a C string encoded with the scheme described above.
 *
 *---------------------------------------------------------------------------
 */

@interface TKNSString:NSString {
@private
    Tcl_DString _ds;
    NSString *_string;
}
@property const char *UTF8String;
- (instancetype)initWithTclUtfBytes:(const void *)bytes
			     length:(NSUInteger)len;
@end

#endif /* _TKMACPRIV */

/*
 * Local Variables:
 * mode: objc
 * c-basic-offset: 4
 * fill-column: 79
 * coding: utf-8
 * End:
 */
