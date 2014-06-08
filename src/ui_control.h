

#ifndef __SGS_UI_CONTROL__
#define __SGS_UI_CONTROL__


#include "ui_utils.h"

#include <float.h>


#define UI_MaxValue FLT_MAX

#define UI_NO_ID 0xffffffff

#define Anchor_Top    0x01
#define Anchor_Bottom 0x02
#define Anchor_Left   0x04
#define Anchor_Right  0x08
#define Anchor_Hor   (Anchor_Left | Anchor_Right)
#define Anchor_Vert  (Anchor_Top | Anchor_Bottom)
#define Anchor_All   (Anchor_Hor | Anchor_Vert)

#define EV_Paint      1
#define EV_Layout     2
#define EV_ChgTheme   3
#define EV_KeyDown    10
#define EV_KeyUp      11
#define EV_Char       12
#define EV_FocusEnter 13
#define EV_FocusLeave 14
#define EV_NeedFocus  15
#define EV_ButtonDown 20
#define EV_ButtonUp   21
#define EV_MouseMove  22
#define EV_MouseEnter 23
#define EV_MouseLeave 24
#define EV_MouseWheel 25
#define EV_Attach     30
#define EV_Detach     31
#define EV_HitTest    32
#define EV_AddChild   33
#define EV_RemChild   34

#define Mouse_ButtonL 0
#define Mouse_ButtonR 1
#define Mouse_ButtonM 2
#define Mouse_Button_Count 3

#define Key_Left      1  // shift makes selection stick
#define Key_Right     2  // ^
#define Key_Up        3  // ^
#define Key_Down      4  // ^
#define Key_DelLeft   5
#define Key_DelRight  6
#define Key_Tab       7  // shift makes focus move backwards
#define Key_Cut       8
#define Key_Copy      9
#define Key_Paste     10
#define Key_Undo      11
#define Key_Redo      12
#define Key_SelectAll 13
#define Key_PageUp    14
#define Key_PageDown  15

#define Hit_Client    2
#define Hit_NonClient 1
#define Hit_None      0

#define UI_Cursor_Arrow      0
#define UI_Cursor_Text       1
#define UI_Cursor_Wait       2
#define UI_Cursor_Crosshair  3
#define UI_Cursor_WaitArrow  4
#define UI_Cursor_SizeNWSE   5
#define UI_Cursor_SizeNESW   6
#define UI_Cursor_SizeWE     7
#define UI_Cursor_SizeNS     8
#define UI_Cursor_SizeAll    9
#define UI_Cursor_No         10
#define UI_Cursor_Hand       11

#define KeyMod_Filter 0x0FF
#define KeyMod_Shift  0x100


struct UIRect
{
	float x0, y0, x1, y1;
};
typedef std::vector< UIRect > UIRectArray;


struct UITimerData
{
	int64_t id;
	float interval;
	float timeout;
	sgsVariable func;
};

typedef std::map< int64_t, UITimerData > UITimerMap;


struct UIEvent
{
	SGS_OBJECT;
	
	UIEvent();
	
	SGS_PROPERTY int type;
	SGS_PROPERTY int key;
	SGS_PROPERTY int button;
	SGS_PROPERTY int uchar;
	SGS_PROPERTY float x;
	SGS_PROPERTY float y;
	SGS_PROPERTY float rx;
	SGS_PROPERTY float ry;
};


struct UIControl;
struct UIFrame
{
	typedef sgsHandle< UIFrame > Handle;
	
	SGS_OBJECT;
	
	UIFrame();
	~UIFrame();
	
	SGS_METHOD sgsHandle< UIControl > createControl( sgsString type );
	
	SGS_METHOD void event( UIEvent* e );
	SGS_METHOD void render();
	UIControl* _getControlAtPosition( float x, float y );
	SGS_METHOD void handleMouseMove();
	SGS_METHOD void setFocus( UIControl* ctrl );
	
	// event generation shortcuts
	SGS_METHOD void doMouseMove( float x, float y );
	SGS_METHOD void doMouseButton( int btn, bool down );
	SGS_METHOD void doMouseWheel( float x, float y );
	SGS_METHOD void doKeyPress( int key, bool down );
	SGS_METHOD void doPutChar( int chr );
	
	// timers
	SGS_METHOD int64_t setTimeout( float t, sgsVariable func ){ return _setTimer( t, func, true ); }
	SGS_METHOD int64_t setInterval( float t, sgsVariable func ){ return _setTimer( t, func, false ); }
	SGS_METHOD void clearTimeout( int64_t id ){ _clearTimer( id ); }
	SGS_METHOD void clearInterval( int64_t id ){ _clearTimer( id ); }
	int64_t _setTimer( float t, sgsVariable func, bool one_shot );
	void _clearTimer( int64_t id );
	SGS_METHOD void processTimers( float t );
	
	// scissor rectangle
	SGS_METHOD bool pushScissorRect( float x0, float y0, float x1, float y1 );
	SGS_METHOD bool pushScissorRectUnclipped( float x0, float y0, float x1, float y1 );
	SGS_METHOD bool popScissorRect();
	SGS_METHOD int getScissorRectCount(){ return m_scissorRects.size(); }
	void _applyScissorState();
	
	SGS_IFUNC(GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj );
	
	// info retrieval
	SGS_METHOD float getClickOffsetX( int button ){ if( button < 0 || button >= Mouse_Button_Count ) return 0; return m_clickoffsets[ button ][0]; }
	SGS_METHOD float getClickOffsetY( int button ){ if( button < 0 || button >= Mouse_Button_Count ) return 0; return m_clickoffsets[ button ][1]; }
	SGS_METHOD sgsHandle< UIControl > getHoverControl();
	SGS_METHOD sgsHandle< UIControl > getFocusControl();
	SGS_METHOD sgsHandle< UIControl > getControlUnderCursor();
	SGS_METHOD sgsHandle< UIControl > getControlUnderPoint( float x, float y );
	
	SGS_PROPERTY sgsVariable render_image;
	SGS_PROPERTY sgsVariable render_text;
	SGS_PROPERTY sgsVariable scissor_func; /* no arguments (all NULL) -> clear, 4 arguments (int) -> set */
	SGS_PROPERTY sgsVariable clipboard_func; /* no arguments -> get; 1 argument (string) -> set */
	
	/*	cursor_func
		argument: cursor variable from control (unknown type)
		function must support the following values:
		- null: default / do not change (cursor outside UI)
		- boolean `false`: no cursor
		- boolean `true`: default cursor
		- integer UI_Cursor_ constants
	*/
	SGS_PROPERTY sgsVariable cursor_func;
	
	/*	theme
		dict containing data about theme
		the convention:
		- <type>_renderfunc - rendering function for the specified control type
		- <type>_<other> - misc. data for the specified control type
	*/
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateTheme ) sgsVariable theme;
	
	
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float width;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float height;
	SGS_PROPERTY READ sgsHandle< UIControl > root;
	
	SGS_PROPERTY READ float mouseX;
	SGS_PROPERTY READ float mouseY;
	
	void updateLayout();
	void updateTheme();
	void forceUpdateCursor( UIControl* ctrl );
	void preRemoveControl( UIControl* ctrl );
	
	UIControl* m_hover;
	UIControl* m_focus;
	UIControl* m_clicktargets[ Mouse_Button_Count ];
	float m_clickoffsets[ Mouse_Button_Count ][2];
	UITimerMap m_timers;
	int64_t m_timerAutoID;
	IDGen m_controlIDGen;
	UIRectArray m_scissorRects;
	
};


struct UIControl
{
	typedef sgsHandle< UIControl > Handle;
	typedef std::vector< Handle > HandleArray;
	
	SGS_OBJECT;
	
	UIControl();
	~UIControl();
	
	int niEvent( UIEvent* event );
	void niBubblingEvent( UIEvent* e );
	void niRender();
	SGS_METHOD void updateLayout();
	SGS_METHOD void updateTheme();
	void updateThemeRecursive();
	void updateCursor();
	
	SGS_METHOD bool addChild( UIControl::Handle ch );
	SGS_METHOD bool removeChild( UIControl::Handle ch );
	SGS_METHOD bool removeAllChildren();
	SGS_METHOD UIControl::Handle findChild( sgsString name );
	SGS_METHOD sgsVariable children( bool nonclient );
	SGS_METHOD void sortChildren();
	SGS_METHOD void sortSiblings();
	SGS_METHOD void setAnchorMode( int mode );
	SGS_METHOD void setAnchorRect( float x0, float y0, float x1, float y1 );
	
	SGS_METHOD bool bindEvent( sgsString name, sgsVariable callable );
	SGS_METHOD bool unbindEvent( sgsString name, sgsVariable callable );
	SGS_METHOD bool callEvent( sgsString name, UIEvent* e );
	
	SGS_IFUNC(GETINDEX) int sgs_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop );
	SGS_IFUNC(GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj );
	
	void _set_width( float v ){ width = v; if( !_whNoAuth ){ minWidth = maxWidth = width; } updateLayout(); }
	void _set_height( float v ){ height = v; if( !_whNoAuth ){ minHeight = maxHeight = height; } updateLayout(); }
	void _set_minWidth( float v ){ minWidth = v; if( !_updatingMinMaxWH && width < minWidth ){ width = minWidth; updateLayout(); } }
	void _set_maxWidth( float v ){ maxWidth = v; if( !_updatingMinMaxWH && width > maxWidth ){ width = maxWidth; updateLayout(); } }
	void _set_minHeight( float v ){ minHeight = v; if( !_updatingMinMaxWH && height < minHeight ){ height = minHeight; updateLayout(); } }
	void _set_maxHeight( float v ){ maxHeight = v; if( !_updatingMinMaxWH && height > maxHeight ){ height = maxHeight; updateLayout(); } }
	float _get_marginLeft(){ return x; }
	float _get_marginRight(){ return -width - x; }
	float _get_marginTop(){ return y; }
	float _get_marginBottom(){ return -height - y; }
	void _set_marginLeft( float v ){ width -= v - x; x = v; updateLayout(); }
	void _set_marginRight( float v ){ width = -v - x; updateLayout(); }
	void _set_marginTop( float v ){ height -= v - y; y = v; updateLayout(); }
	void _set_marginBottom( float v ){ height = -v - y; updateLayout(); }
	
	SGS_PROPERTY READ uint32_t id;
	SGS_PROPERTY sgsString name;
	SGS_PROPERTY sgsString caption;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float y;
	SGS_PROPERTY_FUNC( READ WRITE _set_width ) float width;
	SGS_PROPERTY_FUNC( READ WRITE _set_height ) float height;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float q0x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float q0y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float q1x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float q1y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float scroll_x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float scroll_y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float nc_top;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float nc_left;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float nc_right;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float nc_bottom;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) bool visible;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK sortSiblings ) int index;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK sortSiblings ) bool topmost;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) bool nonclient;
	SGS_PROPERTY_FUNC( READ WRITE _set_minWidth ) float minWidth;
	SGS_PROPERTY_FUNC( READ WRITE _set_maxWidth ) float maxWidth;
	SGS_PROPERTY_FUNC( READ WRITE _set_minHeight ) float minHeight;
	SGS_PROPERTY_FUNC( READ WRITE _set_maxHeight ) float maxHeight;
	SGS_PROPERTY_FUNC( READ _get_marginLeft WRITE _set_marginLeft ) SGS_ALIAS( float marginLeft );
	SGS_PROPERTY_FUNC( READ _get_marginRight WRITE _set_marginRight ) SGS_ALIAS( float marginRight );
	SGS_PROPERTY_FUNC( READ _get_marginTop WRITE _set_marginTop ) SGS_ALIAS( float marginTop );
	SGS_PROPERTY_FUNC( READ _get_marginBottom WRITE _set_marginBottom ) SGS_ALIAS( float marginBottom );
	SGS_PROPERTY sgsString type;
	SGS_PROPERTY READ Handle parent;
	SGS_PROPERTY READ UIFrame::Handle frame;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateCursor ) sgsVariable cursor;
	SGS_PROPERTY sgsVariable callback;
	SGS_PROPERTY sgsVariable renderfunc;
	SGS_PROPERTY sgsVariable data;
	SGS_PROPERTY sgsVariable _interface;
	
	SGS_PROPERTY READ float rx0;
	SGS_PROPERTY READ float rx1;
	SGS_PROPERTY READ float ry0;
	SGS_PROPERTY READ float ry1;
	
	SGS_PROPERTY bool _updatingLayout : 1; /* true if updating layout and don't want to trigger further layout changes */
	SGS_PROPERTY bool _updatingMinMaxWH : 1; /* true if updating min/max width/height and don't watn to trigger further size changes */
	SGS_PROPERTY bool _whNoAuth : 1; /* true if width/height shouldn't set min/max width/height too */
	SGS_PROPERTY READ bool mouseOn;
	SGS_PROPERTY READ bool clicked;
	SGS_PROPERTY READ bool keyboardFocus;
	
	HandleArray m_children;
	HandleArray m_sorted;
	sgsVariable m_events;
};



inline void UI_PushEvent( SGS_CTX, UIEvent* e ){ sgs_PushClassFrom( C, e ); }


#endif // __SGS_UI_CONTROL__

