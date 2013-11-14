#define INCL_WIN
#define INCL_GPI
#define INCL_DOSMISC
#define INCL_DOSPROCESS

#include <os2.h>

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

void dumplong (long n);
void dump (char *s);
void Message (char *s, char *title);
int Question (char *s, char *title);

enum {RUBBER_ZERO,RUBBER_START,RUBBER_CANCEL,RUBBER_DONE};

class Time
{	ULONG Seconds;
	public :
	void set ();
	Time () { set(); }
	ULONG seconds () { return Seconds; }
	operator ULONG () { set(); return Seconds; }
};

extern long defaultsize;

class String
{	char *P;
	long Size;
	public :
	String ();
	String (char *text, long size=defaultsize);
	String (int id); // id from Resource
	~String ();
	char *text () { return P; }
	long size () { return Size; }
	void copy (char *text);
	char *filename ();
	void stripfilename ();
	operator char * () { return P; }
	operator PSZ () { return P; }
};

class Rectangle
{	LONG X,Y,W,H;
	public :
	Rectangle (LONG x=0, LONG y=0, LONG w=1, LONG h=1)
	{	X=x; Y=y; W=w; H=h;
	}
	LONG x1 ()
	{	if (W<0) return X+W+1;
		else return X;
	}
	LONG x2 ()
	{	if (W>0) return X+W-1;
		else return X;
	}
	LONG y1 ()
	{	if (H<0) return Y+H+1;
		else return Y;
	}
	LONG y2 ()
	{	if (H>0) return Y+H-1;
		else return Y;
	}
	LONG x () { return X; }
	LONG y () { return Y; }
	LONG w () { return W; }
	LONG h () { return H; }
	void resize (LONG w, LONG h)
	{	W=w; H=h;
	}
	void hrescale (double scale);
	void wrescale (double scale);
	void minsize (LONG wmin, LONG hmin);
};

class Program
{	HAB Hab;
	HMQ Hmq;
	QMSG Qmsg;
	public :
	Program();
	~Program();
	int getmessage();
	void dispatch();
	void loop();
	HAB hab() { return Hab; }
	HMQ hmq() { return Hmq; }
};

extern Program program;

class PS;

const int FCF_NORMAL=
	FCF_TITLEBAR|FCF_SYSMENU|FCF_SIZEBORDER|FCF_MINMAX|
	FCF_SHELLPOSITION|FCF_TASKLIST|FCF_ICON;

class Menu;

enum clicktype
{ 	button1,button2,button3,
	button1double,button2double,button3double,
	button1up,button1down,
	button2up,button2down,
	button3up,button3down,
	mousemove
};

class Window : public Program
{	HWND FrameHandle,Handle;
	int Width,Height,Visible;
	Menu *Windowmenu;
	int Rubber;
	int Id;
	String *Name;
	ULONG Flags;
	public :
	Window (int id,
		char *name="Main Window",
		ULONG flags=FCF_NORMAL);
	~Window ();
	void init ();
	void setmenu (Menu *m) { Windowmenu=m; }
	virtual void redraw (PS &ps);
	virtual void sized () {}
	virtual void clicked (LONG x, LONG y, clicktype click) {}
	int width () { return Width; }
	int height () { return Height; }
	HWND handle () { return Handle; }
	HWND framehandle () { return FrameHandle; }
	void update () { WinInvalidateRect(Handle,NULL,TRUE); }
	void size (LONG w, LONG h);
	void validate () { WinValidateRect(Handle,NULL,TRUE); }
	void top ();
	int rubberbox (LONG x, LONG y, clicktype click,
		Rectangle &R, LONG wmin=0, LONG hmin=0,
		double wscale=0, double hscale=0);
	friend MRESULT EXPENTRY MainWindowProc (HWND hwnd,
			ULONG msg,MPARAM mp1, MPARAM mp2);
	int visible () { return Visible; }
};

typedef void Menuproc (int Command);

class Menuentry
{	ULONG Command;
	Menuproc *Proc;
	Menuentry *Next;
	public :
	Menuentry (ULONG command, Menuproc *proc,
		Menuentry *next=NULL)
	{	Command=command;
		Proc=proc;
		Next=next;
	}
	Menuentry *next () { return Next; }
	void call (int command) { Proc(command); }
	ULONG command () { return Command; }
};

class Menu
{	Menuentry *Mp;
	Window *W;
	public :
	Menu (Window &window)
	{	Mp=NULL;
		W=&window;
		W->setmenu(this);
	}
	~Menu ();
	void add (ULONG command, Menuproc *proc)
	{   Mp=new Menuentry(command,proc,Mp);
	}
	int call (int command);
};

class Rgb
{	LONG Value;
	public :
	Rgb (int red, int green, int blue)
	{	Value=((unsigned long)red<<16)+((unsigned long)green<<8)+blue;
	}
	Rgb (LONG value) { Value=value; }
	operator LONG () { return Value; }
};

class PS
{   POINTL P;
	ULONG Color,Alignment;
	protected :
	HPS Handle;
	SIZEL S;
	public :
	PS ()
	{	Handle=NULLHANDLE;
		Color=CLR_DEFAULT; Alignment=TA_STANDARD_HORIZ;
	}
	PS (HPS hps)
	{	PS(); Handle=hps; GpiQueryPS(Handle,&S);
	}
	HPS handle () { return Handle; }
	void erase () { GpiErase(Handle); }
	LONG width () { return S.cx; }
	LONG height () { return S.cy; }
	void color (ULONG color)
	{	if (Color!=color)
		{	GpiSetColor(Handle,color);
			Color=color;
		}
	}
	void directcolor (int pure=0);
	void setcolor (int index, Rgb rgb, int pure=0);
	void defaultcolors ();
	void alignment (ULONG al)
	{	if (Alignment!=al)
		{	GpiSetTextAlignment(Handle,al,TA_BASE);
			Alignment=al;
		}
	}
	void mix (int mode) { GpiSetMix(Handle,mode); }
	void move (LONG c, LONG r, ULONG color=CLR_DEFAULT);
	void lineto (LONG c, LONG r, ULONG color=CLR_DEFAULT);
	void linerel (LONG w, LONG h, ULONG color=CLR_DEFAULT);
	void point (LONG w, LONG h, ULONG color=CLR_DEFAULT);
	void text (char *s, ULONG color=CLR_DEFAULT,
		ULONG alignment=TA_LEFT);
	void framedarea (Rectangle &R, int r, ULONG col=CLR_DEFAULT);
	void frame (Rectangle &R, int r=0, ULONG color=CLR_DEFAULT);
	void area (Rectangle &R, int r=0, ULONG color=CLR_DEFAULT);
	void framedarea (LONG w, LONG h, int r, ULONG col=CLR_DEFAULT);
	void frame (LONG w, LONG h, int r=0, ULONG color=CLR_DEFAULT);
	void area (LONG w, LONG h, int r=0, ULONG color=CLR_DEFAULT);
};

inline void Window::redraw (PS &ps)
{	ps.erase();
}

class WindowPS : public PS
{   Window *W;
	int getps;
	void set (HPS handle, Window &window)
	{	W=&window;
		Handle=handle;
		S.cx=window.width(); S.cy=window.height();
	}
	public :
	WindowPS (HPS handle, Window &window)
	{	set(handle,window);
		getps=0;
	}
	WindowPS (Window &window)
	{	set(WinGetPS(window.handle()),window);
		getps=1;
	}
	~WindowPS () { if (getps) WinReleasePS(handle()); }
};

class RedrawPS : public WindowPS
{	public :
	RedrawPS (HWND hwnd, Window &window) :
		WindowPS(WinBeginPaint(hwnd,NULLHANDLE,NULL),window) {}
	~RedrawPS () { WinEndPaint(handle()); }
};

class BitmapPS : public PS
{	HDC DeviceHandle;
	HBITMAP BitmapHandle;
	PBITMAPINFO2 Info;
	int Valid,Planes,Colorbits;
	public :
	BitmapPS (Window &window);
	~BitmapPS ();
	void copy (PS &ps);
	void save (char *filename);
};

class Font
{	FONTMETRICS *Fm;
	PS *Ps;
	public :
	Font ();
	Font (PS &ps);
	~Font ();
	void set (PS &Ps);
	FONTMETRICS *fm () { return Fm; }
	LONG wbox () { return Fm->lAveCharWidth+1; }
	LONG hbox () { return Fm->lMaxDescender+Fm->lMaxAscender; }
	LONG above () { return Fm->lMaxAscender; }
	LONG below () { return Fm->lMaxDescender; }
	void text (int c, int r, char *s, ULONG col=CLR_DEFAULT);
	void lefttext (int c, int r, char *s, ULONG col=CLR_DEFAULT)
	{	text(c,r,s,col);
	}
	void centertext (int c, int r, int w, char *s,
		ULONG col=CLR_DEFAULT);
	void righttext (int c, int r, int w, char *s,
		ULONG col=CLR_DEFAULT);
	void textframe (int c, int r, int w, int h, int rad=0,
		ULONG col=CLR_DEFAULT, int framed=1);
	void getframe (int c, int r, int w, int h,
		LONG &x1, LONG &y1, LONG &x2, LONG &y2);
	int inframe (int c, int r, int w, int h, LONG x, LONG y);
	int line (int r, int h, LONG y);
	PS *ps () { return Ps; }
	operator int () { return Ps!=NULL; }
};

enum textjustify { text_left,text_center,text_right };

class Textbox
{	int X,Y,W,H;
	Font *F;
	int Framed,Rad;
	textjustify Justify;
	ULONG Color;
	char *Text;
	public :
	Textbox (Font &font, int x, int y, int w, int h, char *text="",
		textjustify justify=text_center, ULONG color=CLR_DEFAULT,
		int framed=1, int rad=2)
	{	F=NULL; X=x; Y=y; W=w; H=h; Framed=framed; Color=color;
		Justify=justify;
		Text=text; F=&font;
		redraw();
	}
	void settext (char *text="")
	{	Text=text; redraw();
	}
	void redraw ();
	int contains (LONG x, LONG y)
	{	return F->inframe(X,Y,W,H,x,y);
	}
	int line (LONG y)
	{	return F->line(Y,H,y);
	}
};

class Help
{   HWND Handle;
	int Valid;
	public :
	Help (Window &window, int id, char *filename, char *title="");
	void Help::general (void)
	{	if (Valid) WinSendMsg(Handle,HM_EXT_HELP,NULL,NULL);
	}
	void Help::index (void)
	{	if (Valid) WinSendMsg(Handle,HM_HELP_INDEX,NULL,NULL);
	}
	void Help::content (void)
	{   if (Valid) WinSendMsg(Handle,HM_HELP_CONTENTS,NULL,NULL);
	}
};














	TID Tid;
	int Started;
	long Stacksize;
	Parameter P;
	public :
	Thread (int (*f) (Parameter), long stacksize=4096) : P(0)
	{	F=f;
		Started=0; Stacksize=stacksize;
	}
	void start (Parameter p=0);
	void stop ();
	void suspend ();
	void resume ();
	void wait ();
	Parameter p () { return P; }
	int call () { return F(P); }
};

class Dialogitem;























};






















































































