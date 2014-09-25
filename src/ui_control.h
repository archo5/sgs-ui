

#ifndef __SGS_UI_CONTROL__
#define __SGS_UI_CONTROL__


#include "ui_utils.h"

#include <float.h>


#define UI_MaxValue FLT_MAX

#define UI_NO_ID 0xffffffff

#define UI_Anchor_Top    0x01
#define UI_Anchor_Bottom 0x02
#define UI_Anchor_Left   0x04
#define UI_Anchor_Right  0x08
#define UI_Anchor_Hor   (UI_Anchor_Left | UI_Anchor_Right)
#define UI_Anchor_Vert  (UI_Anchor_Top | UI_Anchor_Bottom)
#define UI_Anchor_All   (UI_Anchor_Hor | UI_Anchor_Vert)

#define _UI_Align_H      0x000f
#define _UI_Align_V      0x00f0
//      ---
#define UI_Align_Left    0x0001
#define UI_Align_Center  0x0002
#define UI_Align_Right   0x0004
#define UI_Align_Justify 0x0008
#define UI_Align_Top     0x0010
#define UI_Align_VCenter 0x0020
#define UI_Align_Bottom  0x0040

#define UI_Pos_Abs       0
#define UI_Pos_Line      1
#define UI_Pos_SideA     2
#define UI_Pos_SideB     3

#define UI_Stack_Top     0
#define UI_Stack_Left    1
#define UI_Stack_Bottom  2
#define UI_Stack_Right   3
#define UI_Stack_AutoSize 0x10
#define UI_Stack_AutoSizeTop (UI_Stack_AutoSize|UI_Stack_Top)
#define UI_Stack_AutoSizeLeft (UI_Stack_AutoSize|UI_Stack_Left)
#define UI_Stack_AutoSizeBottom (UI_Stack_AutoSize|UI_Stack_Bottom)
#define UI_Stack_AutoSizeRight (UI_Stack_AutoSize|UI_Stack_Right)

#define EV_Changed      1
#define EV_ChgTheme     3
#define EV_KeyDown      10
#define EV_KeyUp        11
#define EV_Char         12
#define EV_FocusEnter   13
#define EV_FocusLeave   14
#define EV_NeedFocus    15
#define EV_ButtonDown   20
#define EV_ButtonUp     21
#define EV_MouseMove    22
#define EV_MouseEnter   23
#define EV_MouseLeave   24
#define EV_MouseWheel   25
#define EV_Attach       30
#define EV_Detach       31
#define EV_HitTest      32
#define EV_AddChild     33
#define EV_RemChild     34
#define EV_AddComponent 40
#define EV_RemComponent 41

#define EV_Changed_Font   0x0001
#define EV_Changed_Image  0x0002
#define EV_Changed_Icon   0x0004
#define EV_Changed_Cursor 0x0008
#define EV_Changed_Order  0x0010
#define EV_Changed_Box    0x0020
#define EV_Changed_Layout (EV_Changed_Font|EV_Changed_Order|EV_Changed_Box)

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
#define Key_Enter     16

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


struct UIColor
{
	SGS_OBJECT_LITE;
	
	UIColor() : r(1), g(1), b(1), a(1){}
	UIColor( float _r, float _g, float _b, float _a ) : r(_r), g(_g), b(_b), a(_a){}
	
	SGS_PROPERTY float r;
	SGS_PROPERTY float g;
	SGS_PROPERTY float b;
	SGS_PROPERTY float a;
	
	int _get_size(){ return 4; }
	SGS_PROPERTY_FUNC( READ _get_size ) SGS_ALIAS( int size );
	
	operator bool (){ return true; }
	UIColor operator + ( const UIColor& o ) const { return UIColor( r + o.r, g + o.g, b + o.b, a + o.a ); }
	UIColor operator - ( const UIColor& o ) const { return UIColor( r - o.r, g - o.g, b - o.b, a - o.a ); }
	UIColor operator * ( const UIColor& o ) const { return UIColor( r * o.r, g * o.g, b * o.b, a * o.a ); }
	UIColor scale( float q ) const { return UIColor( r * q, g * q, b * q, a * q ); }
	
	SGS_IFUNC(GETINDEX) int sgs_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop );
	SGS_IFUNC(EXPR) int sgs_expr( SGS_CTX, sgs_VarObj* obj, sgs_Variable* A, sgs_Variable* B, int isprop );
};
static inline bool _readcolorvar_i( SGS_CTX, sgs_StkIdx item, int idx, float* out )
{
	sgs_Variable key;
	sgs_InitInt( &key, idx );
	if( SGS_SUCCEEDED( sgs_PushIndexIP( C, item, &key, 0 ) ) )
	{
		*out = sgs_GetReal( C, -1 );
		sgs_Pop( C, 1 );
		return true;
	}
	return false;
}
static inline bool _readcolorvar_p( SGS_CTX, sgs_Variable* var, int idx, float* out )
{
	sgs_Variable key;
	sgs_InitInt( &key, idx );
	if( SGS_SUCCEEDED( sgs_PushIndexPP( C, var, &key, 0 ) ) )
	{
		*out = sgs_GetReal( C, -1 );
		sgs_Pop( C, 1 );
		return true;
	}
	return false;
}
template<> inline void sgs_PushVar<UIColor>( SGS_CTX, const UIColor& v ){ sgs_PushLiteClassFrom( C, &v ); }
template<> struct sgs_GetVar<UIColor> { UIColor operator () ( SGS_CTX, sgs_StkIdx item ){
	if( sgs_IsObject( C, item, UIColor::_sgs_interface ) )
		return *(UIColor*)sgs_GetObjectData( C, item );
	UIColor oc( 1, 1, 1, 1 );
	if( !_readcolorvar_i( C, item, 0, &oc.r ) ) ; // 0 colors read successfully
	else if( !_readcolorvar_i( C, item, 1, &oc.g ) ){ oc.a = oc.b = oc.g = oc.r; } // 1
	else if( !_readcolorvar_i( C, item, 2, &oc.b ) ){ oc.a = oc.g; oc.b = oc.g = oc.r; } // 2
	else _readcolorvar_i( C, item, 3, &oc.a ); // 3 / 4
	return oc;
}};
template<> struct sgs_GetVarP<UIColor> { UIColor operator () ( SGS_CTX, sgs_Variable* val ){
	if( sgs_IsObjectP( val, UIColor::_sgs_interface ) )
		return *(UIColor*)sgs_GetObjectDataP( val );
	UIColor oc( 1, 1, 1, 1 );
	if( !_readcolorvar_p( C, val, 0, &oc.r ) ) ; // 0 colors read successfully
	else if( !_readcolorvar_p( C, val, 1, &oc.g ) ){ oc.a = oc.b = oc.g = oc.r; } // 1
	else if( !_readcolorvar_p( C, val, 2, &oc.b ) ){ oc.a = oc.g; oc.b = oc.g = oc.r; } // 2
	else _readcolorvar_p( C, val, 3, &oc.a ); // 3 / 4
	return oc;
}};


struct UIEvent
{
	SGS_OBJECT_LITE;
	
	UIEvent() : type(0), key(0), uchar(0), x(0), y(0), rx(0), ry(0){}
	
	SGS_PROPERTY int type;
	SGS_PROPERTY int key;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME subtype ) SGS_ALIAS( int key );
	SGS_PROPERTY_FUNC( READ WRITE VARNAME button ) SGS_ALIAS( int key );
	SGS_PROPERTY uint32_t uchar;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME clicks ) SGS_ALIAS( uint32_t uchar );
	SGS_PROPERTY float x;
	SGS_PROPERTY float y;
	SGS_PROPERTY float rx;
	SGS_PROPERTY float ry;
	SGS_PROPERTY sgsHandle<struct UIControl> target;
};

UIEvent* UI_CreateEvent( SGS_CTX, sgsVariable& out, int type = 0 );


struct UIStyle /* style storage */
{
	SGS_OBJECT_LITE;
	
	SGS_PROPERTY sgsMaybe<float> x;
	SGS_PROPERTY sgsMaybe<float> y;
	SGS_PROPERTY sgsMaybe<float> width;
	SGS_PROPERTY sgsMaybe<float> height;
	SGS_PROPERTY sgsMaybe<float> q0x;
	SGS_PROPERTY sgsMaybe<float> q0y;
	SGS_PROPERTY sgsMaybe<float> q1x;
	SGS_PROPERTY sgsMaybe<float> q1y;
	SGS_PROPERTY sgsMaybe<float> nonClientTop;
	SGS_PROPERTY sgsMaybe<float> nonClientLeft;
	SGS_PROPERTY sgsMaybe<float> nonClientRight;
	SGS_PROPERTY sgsMaybe<float> nonClientBottom;
	SGS_PROPERTY sgsMaybe<bool>  visible;
	SGS_PROPERTY sgsMaybe<int>   index;
	SGS_PROPERTY sgsMaybe<bool>  topmost;
	SGS_PROPERTY sgsMaybe<float> minWidth;
	SGS_PROPERTY sgsMaybe<float> maxWidth;
	SGS_PROPERTY sgsMaybe<float> minHeight;
	SGS_PROPERTY sgsMaybe<float> maxHeight;
	SGS_PROPERTY sgsMaybe<float> marginLeft;
	SGS_PROPERTY sgsMaybe<float> marginRight;
	SGS_PROPERTY sgsMaybe<float> marginTop;
	SGS_PROPERTY sgsMaybe<float> marginBottom;
	SGS_PROPERTY sgsMaybe<float> paddingLeft;
	SGS_PROPERTY sgsMaybe<float> paddingRight;
	SGS_PROPERTY sgsMaybe<float> paddingTop;
	SGS_PROPERTY sgsMaybe<float> paddingBottom;
	SGS_PROPERTY sgsMaybe<int8_t> posMode;
	SGS_PROPERTY sgsMaybe<int8_t> stackMode;
	SGS_PROPERTY sgsMaybe<bool>  overflow;
	SGS_PROPERTY sgsMaybe<UIColor> backgroundColor;
	SGS_PROPERTY sgsMaybe<UIColor> textColor;
	SGS_PROPERTY sgsMaybe<uint16_t> halign;
	SGS_PROPERTY sgsMaybe<uint16_t> valign;
	SGS_PROPERTY sgsVariable     cursor;
	SGS_PROPERTY sgsString       font;
	SGS_PROPERTY sgsMaybe<float> fontSize;
	SGS_PROPERTY sgsString       image;
	SGS_PROPERTY sgsString       icon;
	SGS_PROPERTY sgsVariable     renderfunc;
	
#define UIS_DEFINE_ACCESSORS_ADC( ty, nm ) \
	ty get_##nm##H() const { return TMIN( nm##Left.isset ? nm##Left.data : 0, nm##Right.isset ? nm##Right.data : 0 ); } \
	ty get_##nm##V() const { return TMIN( nm##Top.isset ? nm##Top.data : 0, nm##Bottom.isset ? nm##Bottom.data : 0 ); } \
	ty get_##nm() const { return TMIN( TMIN( nm##Left.isset ? nm##Left.data : 0, nm##Right.isset ? nm##Right.data : 0 ), \
		TMIN( nm##Top.isset ? nm##Top.data : 0, nm##Bottom.isset ? nm##Bottom.data : 0 ) ); } \
	void set_##nm##H( ty nm ){ nm##Left.set( nm ); nm##Right.set( nm ); } \
	void set_##nm##V( ty nm ){ nm##Top.set( nm ); nm##Bottom.set( nm ); } \
	void set_##nm( ty nm ){ nm##Left.set( nm ); nm##Right.set( nm ); nm##Top.set( nm ); nm##Bottom.set( nm ); }

	UIS_DEFINE_ACCESSORS_ADC( float, nonClient );
	UIS_DEFINE_ACCESSORS_ADC( float, margin );
	UIS_DEFINE_ACCESSORS_ADC( float, padding );
	
	int get_anchorMode();
	void set_anchorMode( int mode );
	uint16_t get_align();
	void set_align( uint16_t mode );
	
	SGS_PROPERTY_FUNC( READ get_nonClientH WRITE set_nonClientH ) SGS_ALIAS( float nonClientH );
	SGS_PROPERTY_FUNC( READ get_nonClientV WRITE set_nonClientV ) SGS_ALIAS( float nonClientV );
	SGS_PROPERTY_FUNC( READ get_nonClient WRITE set_nonClient ) SGS_ALIAS( float nonClient );
	SGS_PROPERTY_FUNC( READ get_marginH WRITE set_marginH ) SGS_ALIAS( float marginH );
	SGS_PROPERTY_FUNC( READ get_marginV WRITE set_marginV ) SGS_ALIAS( float marginV );
	SGS_PROPERTY_FUNC( READ get_margin WRITE set_margin ) SGS_ALIAS( float margin );
	SGS_PROPERTY_FUNC( READ get_paddingH WRITE set_paddingH ) SGS_ALIAS( float paddingH );
	SGS_PROPERTY_FUNC( READ get_paddingV WRITE set_paddingV ) SGS_ALIAS( float paddingV );
	SGS_PROPERTY_FUNC( READ get_padding WRITE set_padding ) SGS_ALIAS( float padding );
	SGS_PROPERTY_FUNC( READ get_anchorMode WRITE set_anchorMode ) SGS_ALIAS( int anchorMode );
	SGS_PROPERTY_FUNC( READ get_align WRITE set_align ) SGS_ALIAS( uint16_t align );
	
	void gcmark(){ cursor.gcmark(); renderfunc.gcmark(); }
};
SGS_DEFAULT_LITE_OBJECT_INTERFACE( UIStyle );

struct UIStyleCache /* computed style cache */
{
	float x;
	float y;
	float width;
	float height;
	float q0x;
	float q0y;
	float q1x;
	float q1y;
	float nonClientTop;
	float nonClientLeft;
	float nonClientRight;
	float nonClientBottom;
	bool  visible;
	int   index;
	bool  topmost;
	float minWidth;
	float maxWidth;
	float minHeight;
	float maxHeight;
	float marginLeft;
	float marginRight;
	float marginTop;
	float marginBottom;
	float paddingLeft;
	float paddingRight;
	float paddingTop;
	float paddingBottom;
	int8_t posMode;
	int8_t stackMode;
	bool  overflow;
	UIColor backgroundColor;
	UIColor textColor;
	uint16_t halign;
	uint16_t valign;
	sgsVariable cursor;
	sgsString   font;
	float       fontSize;
	sgsString   image;
	sgsString   icon;
	sgsVariable renderfunc;
	
	int get_anchorMode();
	uint16_t get_align(){ return halign | valign; }
};

struct UIStyleSelector
{
	struct Fragment
	{
		enum Type
		{
			T_None = 0,
			T_MoveOn, // all done with current control, can move on to next
			T_ReqNext, // require that the element to be matched is next (directly related) in line of parents/children (>)
			T_MatchAny, // doesn't matter what's there - as long as there's something
			T_MatchType, // first word matches the type (button,autolayout,..) of the control, if any is given
			// following words, prefixed by dot, specify classes to match
			T_MatchClassExact, // .class
			T_MatchClassPartBegin, // .^class
			T_MatchClassPartEnd, // .$class
			T_MatchClassPart, // .~class
			// following words, prefixed by 'at' (@), specify the exact name attribute to expect
			T_MatchNameExact, // @name
			T_MatchNamePartBegin, // @^name
			T_MatchNamePartEnd, // @$name
			T_MatchNamePart, // @~name
			// following words, prefixed by colon (:), specify the state/position to expect
			T_MatchStateHover, // :hover (cursor is on it)
			T_MatchStateActive, // :active (clicked on it)
			T_MatchStateFocus, // :focus (has keyboard focus)
			T_MatchPosFirstChild, // :first-child
			T_MatchPosLastChild, // :last-child
		};
		
		Type type;
		const char* start;
		const char* end;
	};
	
	typedef std::vector< Fragment > FragmentArray;
	
	sgsString selector;
	FragmentArray fragments;
	int numnext;
	int numtypes;
	int numclasses;
	int numnames;
};
typedef std::vector< UIStyleSelector > StyleSelArray;

void StyleArr_trimSelector( const char** str, size_t* size );
const char* StyleArr_addSelector( StyleSelArray& selectors, sgsString sgsstr, const char* str, size_t size );
const char* StyleArr_addSelectors( StyleSelArray& selectors, sgsString sgsstr );

struct UIStyleRule
{
	typedef sgsHandle< UIStyleRule > Handle;
	typedef std::vector< Handle > HandleArray;
	
	SGS_OBJECT;
	
	SGS_PROPERTY UIStyle style;
	StyleSelArray selectors;
	
	SGS_MULTRET _handleSGS( const char* serr, const char* base );
	SGS_METHOD SGS_MULTRET addSelector( sgsString sgsstr );
	SGS_METHOD SGS_MULTRET addSelectors( sgsString sgsstr );
	SGS_METHOD bool checkMatch( sgsHandle< struct UIControl > ctrl );
};

struct UIStyleSheet
{
	typedef sgsHandle< UIStyleSheet > Handle;
	
	SGS_OBJECT;
	
	SGS_IFUNC(GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj );
	
	UIStyleRule::HandleArray rules;
	
	int _get_ruleCount(){ return rules.size(); }
	SGS_PROPERTY_FUNC( READ _get_ruleCount ) SGS_ALIAS( int ruleCount );
	
	SGS_METHOD SGS_MULTRET addRule( UIStyleRule::Handle rule );
	SGS_METHOD void build( sgsVariable var );
};
typedef std::vector< UIStyleSheet::Handle > StyleSheetArray;


const char* UI_ParseSelector( UIStyleSelector* sel, sgsString sgsstr, const char* str, size_t size );
bool UI_SelectorTestControl( const UIStyleSelector* sel, struct UIControl* ctrl );
int UI_CompareSelectors( const UIStyleSelector* sel1, const UIStyleSelector* sel2 );
void UI_StyleMerge( UIStyle* style, UIStyle* add );
void UI_ToStyleCache( UIStyleCache* cache, UIStyle* style );

bool UI_TxMatchExact( const char* stfrom, const char* stto, const char* tgt, size_t tgtsize );
bool UI_TxMatchPartBegin( const char* stfrom, const char* stto, const char* tgt, size_t tgtsize );
bool UI_TxMatchPartEnd( const char* stfrom, const char* stto, const char* tgt, size_t tgtsize );
bool UI_TxMatchPart( const char* stfrom, const char* stto, const char* tgt, size_t tgtsize );

struct UIFilteredStyle
{
	UIStyleRule* rule;
	size_t which_sel; // selector number
};
typedef std::vector< UIFilteredStyle > UIFilteredStyleArray;


struct UIControl;
struct UIFrame
{
	typedef sgsHandle< UIFrame > Handle;
	typedef std::vector< UIControl* > CtrlPtrArray;
	
	SGS_OBJECT;
	
	UIFrame();
	~UIFrame();
	
	SGS_METHOD sgsHandle< UIControl > createControl( sgsString type );
	SGS_METHOD sgsHandle<struct UIQuery> find( /* args */ );
	
	SGS_METHOD void event( sgsVariable ev );
	SGS_METHOD void render();
	UIControl* _getControlAtPosition( float x, float y, bool fillarr = false );
	SGS_METHOD void handleMouseMove( bool optional );
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
	bool inScissorRect( UIControl* ctrl );
	
	SGS_IFUNC(GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj );
	
	// styling
	SGS_METHOD void addStyleSheet( UIStyleSheet::Handle sheet );
	SGS_METHOD void removeStyleSheet( UIStyleSheet::Handle sheet );
	SGS_METHOD sgsVariable getStyleSheets();
	SGS_METHOD UIStyleSheet::Handle getStyleSheet( size_t i ){ if( i >= m_styleSheets.size() ) return UIStyleSheet::Handle(); return m_styleSheets[ i ]; }
	SGS_METHOD size_t getStyleSheetCount(){ return m_styleSheets.size(); }
	void _updateStyles( UIControl* initial );
	
	// info retrieval
	SGS_METHOD float getClickOffsetX( int button ){ if( button < 0 || button >= Mouse_Button_Count ) return 0; return m_clickoffsets[ button ][0]; }
	SGS_METHOD float getClickOffsetY( int button ){ if( button < 0 || button >= Mouse_Button_Count ) return 0; return m_clickoffsets[ button ][1]; }
	SGS_METHOD sgsHandle< UIControl > getHoverControl();
	SGS_METHOD sgsHandle< UIControl > getFocusControl();
	SGS_METHOD sgsHandle< UIControl > getControlUnderCursor();
	SGS_METHOD sgsHandle< UIControl > getControlUnderPoint( float x, float y );
	SGS_METHOD bool isControlUnderCursor( const sgsHandle< UIControl >& ctrl );
	SGS_METHOD bool isControlUnderPoint( const sgsHandle< UIControl >& ctrl, float x, float y );
	
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
	
	/*	font_func
		arguments: font name, font size
		- null name returns the default font
		should return object capable of being used by the theme
	*/
	SGS_PROPERTY sgsVariable font_func;
	
	/*	image_func
		arguments: image name
		- null/empty-string name returns whatever represents the "no image" state (could be null or another image)
		should return object capable of being used by the theme
	*/
	SGS_PROPERTY sgsVariable image_func;
	
	/*	icon_func
		arguments: icon name
		- null/empty-string name returns whatever represents the "no icon" state (could be null or another icon)
		should return object capable of being used by the theme
	*/
	SGS_PROPERTY sgsVariable icon_func;
	
	/*	theme
		dict containing data about theme
		the convention:
		- <type>_renderfunc - rendering function for the specified control type
		- <type>_<other> - misc. data for the specified control type
	*/
	SGS_PROPERTY READ sgsVariable theme;
	SGS_METHOD void setTheme( sgsVariable newtheme );
	
	SGS_PROPERTY float clickTime;
	
	
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK onLayoutChange ) float x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK onLayoutChange ) float y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK onLayoutChange ) float width;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK onLayoutChange ) float height;
	SGS_METHOD void resize( float _w, float _h ){ width = _w; height = _h; onLayoutChange(); }
	SGS_PROPERTY READ sgsHandle< UIControl > root;
	
	SGS_PROPERTY READ float mouseX;
	SGS_PROPERTY READ float mouseY;
	
	void onLayoutChange();
	void updateTheme();
	void forceUpdateCursor( UIControl* ctrl );
	void preRemoveControl( UIControl* ctrl );
	
	UIControl* m_hover;
	UIControl* m_focus;
	UIControl* m_clicktargets[ Mouse_Button_Count ];
	float m_clickoffsets[ Mouse_Button_Count ][2];
	int m_lastClickedButton;
	float m_lastClickTime;
	int m_clickCount;
	void* m_lastClickItem;
	UITimerMap m_timers;
	int64_t m_timerAutoID;
	IDGen m_controlIDGen;
	UIRectArray m_scissorRects;
	CtrlPtrArray m_animatedControls;
	CtrlPtrArray m_hoverTrail;
	StyleSheetArray m_styleSheets;
	
};


struct UIStackLayoutState
{
	/* layout state */
	float xc0, yc0; /* cursor position 0 (furthest no margin) */
	float xc1, yc1; /* cursor position 1 (furthest with margin) */
	float /*xn0,*/ yn0; /* predicted cursor data (no margin) */
	float /*xn1,*/ yn1; /* predicted cursor data (with margin) */
	float tw; /* total width (height = xn0) */
	/* control state */
	float cx, cy; /* calculated control position */
};


struct UIComponent
{
	SGS_OBJECT;
	
	SGS_PROPERTY sgsString type;
	SGS_PROPERTY sgsVariable func;
	SGS_PROPERTY sgsVariable data;
};
typedef sgsHandle< UIComponent > CmptHandle;
typedef std::vector< CmptHandle > CmptArray;


// Animation tools
struct UIAnimation
{
	sgsVariable prevState;
	sgsVariable currState;
	float time, end;
	sgsVariable func; /* args: v0, v1, q, tend;  ret = interpolated */
	sgsVariable oncomplete; /* this = control */
	
	void gcmark(){ prevState.gcmark(); currState.gcmark(); func.gcmark(); oncomplete.gcmark(); }
};
template<> inline SGSRESULT sgs_GCMarkVar<UIAnimation>( SGS_CTX, UIAnimation& v ){ v.gcmark(); return SGS_SUCCESS; }
typedef std::vector< UIAnimation > UIAnimArray;


struct UIControl
{
	typedef sgsHandle< UIControl > Handle;
	typedef std::vector< Handle > HandleArray;
	
	
	SGS_OBJECT;
	
	UIControl();
	~UIControl();
	
	int niEvent( sgsVariable& ev, bool only = false );
	void niDeepEvent( sgsVariable& ev );
	void niBubblingEvent( sgsVariable& ev );
	void niRender();
	SGS_METHOD void updateScroll();
	SGS_METHOD void updateTheme();
	void updateThemeRecursive();
	void updateCursor();
	void updateFont();
	void updateImage();
	void updateIcon();
	
	void ppgLayoutChange( UIControl* from = NULL );
	SGS_METHOD void onLayoutChange();
	
	bool isStacked(){ return get_posMode() != UI_Pos_Abs; }
	UIStackLayoutState m_stackedLayout;
	
	SGS_METHOD bool insertChild( UIControl::Handle ch, ssize_t pos );
	SGS_METHOD bool removeChild( UIControl::Handle ch );
	SGS_METHOD bool removeAllChildren();
	SGS_METHOD void detach();
	SGS_METHOD void destroy( bool hard ); /* the goal here is to minimize the possibility of circular references */
	SGS_METHOD void destroyAllChildren( bool hard, int clientness );
	SGS_METHOD bool swapChild( UIControl::Handle ch, UIControl::Handle nch );
	SGS_METHOD UIControl::Handle findChild( sgsString name );
	SGS_METHOD sgsVariable children( bool nonclient );
	SGS_METHOD sgsVariable allChildren();
	SGS_METHOD void sortChildren();
	SGS_METHOD void sortSiblings();
	SGS_METHOD int getChildIndex( UIControl::Handle ch );
	SGS_METHOD int getChildCount( bool nonclient );
	// child helper functions
	SGS_METHOD bool prependChild( UIControl::Handle ch ){ return insertChild( ch, 0 ); }
	SGS_METHOD bool appendChild( UIControl::Handle ch ){ return insertChild( ch, m_children.size() ); }
	SGS_METHOD bool addChild( UIControl::Handle ch ){ return insertChild( ch, m_children.size() ); } // old name, =append
	SGS_METHOD int getOrderIndex(){ if( parent.not_null() ) return parent->getChildIndex( Handle( this ) ); else return -1; }
	SGS_METHOD int getAllChildCount(){ return m_children.size(); }
	SGS_METHOD bool moveUp(){ int oi = getOrderIndex(); if( oi < 1 ) return false; return parent->insertChild( Handle( this ), oi - 1 ); }
	SGS_METHOD bool moveDown(){ int oi = getOrderIndex(); if( oi < 0 || oi >= (int) m_children.size() ) return false; return parent->insertChild( Handle( this ), oi + 1 ); }
	SGS_METHOD bool moveToIndex( int i ){
		if( !parent.not_null() ) return false;
		if( i < (int) parent->m_children.size() && getOrderIndex() < i ) i++;
		return parent->insertChild( Handle( this ), i ); }
	SGS_METHOD bool swapOutFor( UIControl::Handle nch ){ if( !parent.not_null() ) return false; else return parent->swapChild( Handle( this ), nch ); }
	// child metrics
	SGS_METHOD SGS_MULTRET getChildAABB( int clientness );
	
	int get_anchorMode(){ return computedStyle.get_anchorMode(); }
	void set_anchorMode( int mode ){ style.set_anchorMode( mode ); _remergeStyle(); }
	SGS_METHOD void setAnchorRect( float x0, float y0, float x1, float y1 );
	
	UIControl* _getPrev();
	UIControl* _getNext();
	SGS_METHOD Handle getPrev(){ UIControl* ctrl = _getPrev(); if( ctrl ) return Handle( C, ctrl->m_sgsObject ); return Handle(); }
	SGS_METHOD Handle getNext(){ UIControl* ctrl = _getNext(); if( ctrl ) return Handle( C, ctrl->m_sgsObject ); return Handle(); }
	
	SGS_METHOD bool hasEventBinding( sgsString name, sgsVariable callable );
	SGS_METHOD UIControl::Handle bindEvent( sgsString name, sgsVariable callable );
	SGS_METHOD UIControl::Handle unbindEvent( sgsString name, sgsVariable callable );
	SGS_METHOD UIControl::Handle unbindEventAll( sgsString name );
	SGS_METHOD void unbindEverything();
	SGS_METHOD bool callEvent( sgsString name, sgsVariable data );
	
	SGS_METHOD void addComponent( sgsString type, sgsVariable func, sgsVariable data );
	SGS_METHOD int getComponentCount(){ return m_components.size(); }
	SGS_METHOD CmptHandle getComponent( int i ){ if( i < 0 || i >= m_components.size() ) return CmptHandle(); return m_components[ i ]; }
	SGS_METHOD CmptHandle findComponentByType( sgsString type );
	SGS_METHOD bool removeComponentByType( sgsString type );
	SGS_METHOD bool removeComponentAtIndex( int i );
	bool niComponentMsg( int i, sgsVariable ev );
	bool niComponentMsgAll( sgsVariable ev );
	
	SGS_METHOD UIControl::Handle animate( sgsVariable state, float length, sgsVariable func, sgsVariable oncomplete );
	SGS_METHOD UIControl::Handle stop( bool nofinish ); /* skip + dequeue */
	SGS_METHOD UIControl::Handle dequeue(); /* remove all except current (time>0) */
	SGS_METHOD UIControl::Handle skip( bool nofinish ); /* remove current */
	SGS_METHOD int queueSize(){ return m_animQueue.size(); }
	void _advanceAnimation( float dt );
	void _applyCurAnimState();
	void _finishCurAnim();
	void _startCurAnim();
	
	SGS_IFUNC(GETINDEX) int sgs_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop );
	SGS_IFUNC(SETINDEX) int sgs_setindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, sgs_Variable* val, int isprop );
	
	/// PRIMARY DATA
	SGS_PROPERTY READ uint32_t id;
	SGS_PROPERTY sgsString name;
	SGS_PROPERTY sgsString caption;
	SGS_PROPERTY sgsString type;
	SGS_PROPERTY READ Handle parent; SGS_GCREF( parent );
	SGS_PROPERTY READ UIFrame::Handle frame; SGS_GCREF( frame );
	
	sgsString classes;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _classUpdated VARNAME class ) SGS_ALIAS( sgsString classes );
	void _classUpdated(){ frame->_updateStyles( this ); }
	SGS_METHOD UIControl::Handle addClass( const sgsString& ss ){ addClass( ss.c_str(), ss.size() ); return Handle( this ); }
	SGS_METHOD UIControl::Handle removeClass( const sgsString& ss ){ removeClass( ss.c_str(), ss.size() ); return Handle( this ); }
	SGS_METHOD UIControl::Handle addRemClass( const sgsString& ss, bool add ){ if( add ) return addClass( ss ); else return removeClass( ss ); }
	SGS_METHOD bool hasClass( const sgsString& ss ){ return hasClass( ss.c_str(), ss.size() ); }
	void _setClasses3( const char* str1, size_t size1, const char* str2, size_t size2, const char* str3, size_t size3 );
	bool addClass( const char* str, size_t size );
	bool removeClass( const char* str, size_t size );
	bool hasClass( const char* str, size_t size );
	bool hasClassPartBegin( const char* str, size_t size );
	bool hasClassPartEnd( const char* str, size_t size );
	bool hasClassPart( const char* str, size_t size );
	size_t _findClassAt( const char* str, size_t size, int checksides = 1|2 );
	static void _trimClass( const char** str, size_t* size );
	
	/// STYLES
	SGS_PROPERTY Handle styleParent; SGS_GCREF( styleParent );
	UIControl* getStyleParentPtr(){ return styleParent.not_null() ? styleParent : parent; }
	UIStyleCache computedStyle; // active style data (after merging rule style and local override, on applying changes)
	SGS_PROPERTY READ UIStyle filteredStyle; SGS_GCREF( filteredStyle ); // calculated from style rules
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _remergeStyle ) UIStyle style; SGS_GCREF( style ); // local override
	void _refilterStyles( UIFilteredStyleArray& styles );
	void _remergeStyle();
	void _applyStyle( const UIStyleCache& nsc );
	SGS_METHOD sgsVariable _getMatchedSelectors();
	
#define UIC_DEFINE_ACCESSORS( ty, nm ) \
	ty get_##nm() const { return computedStyle.nm; } \
	void set_##nm( sgsMaybe<ty> nm ){ style.nm = nm; _remergeStyle(); }
#define UIC_DEFINE_ACCESSORS2( ty, nm ) \
	ty get_##nm() const { return computedStyle.nm; } \
	void set_##nm( ty nm ){ style.nm = nm; _remergeStyle(); }
	// additional directional combination accessors
#define UIC_DEFINE_ACCESSORS_ADC( ty, nm ) \
	ty get_##nm##H() const { return TMIN( computedStyle.nm##Left, computedStyle.nm##Right ); } \
	ty get_##nm##V() const { return TMIN( computedStyle.nm##Top, computedStyle.nm##Bottom ); } \
	ty get_##nm() const { return TMIN( TMIN( computedStyle.nm##Left, computedStyle.nm##Right ), TMIN( computedStyle.nm##Top, computedStyle.nm##Bottom ) ); } \
	void set_##nm##H( sgsMaybe<ty> nm ){ style.nm##Left = nm; style.nm##Right = nm; _remergeStyle(); } \
	void set_##nm##V( sgsMaybe<ty> nm ){ style.nm##Top = nm; style.nm##Bottom = nm; _remergeStyle(); } \
	void set_##nm( sgsMaybe<ty> nm ){ style.nm##Left = nm; style.nm##Right = nm; style.nm##Top = nm; style.nm##Bottom = nm; _remergeStyle(); }
	
	UIC_DEFINE_ACCESSORS( float, x );
	UIC_DEFINE_ACCESSORS( float, y );
	UIC_DEFINE_ACCESSORS( float, width );
	UIC_DEFINE_ACCESSORS( float, height );
	UIC_DEFINE_ACCESSORS( float, q0x );
	UIC_DEFINE_ACCESSORS( float, q0y );
	UIC_DEFINE_ACCESSORS( float, q1x );
	UIC_DEFINE_ACCESSORS( float, q1y );
	UIC_DEFINE_ACCESSORS( float, nonClientTop );
	UIC_DEFINE_ACCESSORS( float, nonClientLeft );
	UIC_DEFINE_ACCESSORS( float, nonClientRight );
	UIC_DEFINE_ACCESSORS( float, nonClientBottom );
	UIC_DEFINE_ACCESSORS( bool, visible );
	UIC_DEFINE_ACCESSORS( int, index );
	UIC_DEFINE_ACCESSORS( bool, topmost );
	UIC_DEFINE_ACCESSORS( float, minWidth );
	UIC_DEFINE_ACCESSORS( float, maxWidth );
	UIC_DEFINE_ACCESSORS( float, minHeight );
	UIC_DEFINE_ACCESSORS( float, maxHeight );
	UIC_DEFINE_ACCESSORS( float, marginLeft );
	UIC_DEFINE_ACCESSORS( float, marginRight );
	UIC_DEFINE_ACCESSORS( float, marginTop );
	UIC_DEFINE_ACCESSORS( float, marginBottom );
	UIC_DEFINE_ACCESSORS( float, paddingLeft );
	UIC_DEFINE_ACCESSORS( float, paddingRight );
	UIC_DEFINE_ACCESSORS( float, paddingTop );
	UIC_DEFINE_ACCESSORS( float, paddingBottom );
	UIC_DEFINE_ACCESSORS( int8_t, posMode );
	UIC_DEFINE_ACCESSORS( int8_t, stackMode );
	UIC_DEFINE_ACCESSORS( bool, overflow );
	UIC_DEFINE_ACCESSORS( UIColor, backgroundColor );
	UIC_DEFINE_ACCESSORS( UIColor, textColor );
	UIC_DEFINE_ACCESSORS( uint16_t, halign );
	UIC_DEFINE_ACCESSORS( uint16_t, valign );
	UIC_DEFINE_ACCESSORS2( sgsVariable, cursor );
	UIC_DEFINE_ACCESSORS2( sgsString, font );
	UIC_DEFINE_ACCESSORS2( sgsString, image );
	UIC_DEFINE_ACCESSORS2( sgsString, icon );
	UIC_DEFINE_ACCESSORS( float, fontSize );
	UIC_DEFINE_ACCESSORS2( sgsVariable, renderfunc );
	
	UIC_DEFINE_ACCESSORS_ADC( float, nonClient );
	UIC_DEFINE_ACCESSORS_ADC( float, margin );
	UIC_DEFINE_ACCESSORS_ADC( float, padding );
	uint16_t get_align(){ return computedStyle.get_align(); }
	void set_align( uint16_t mode ){ style.set_align( mode ); _remergeStyle(); }
	
	float get_offsetLeft(){ return get_x(); }
	float get_offsetRight(){ return -get_width() - get_x(); }
	float get_offsetTop(){ return get_y(); }
	float get_offsetBottom(){ return -get_height() - get_y(); }
	void set_offsetLeft( float v ){ style.width.set( get_width() - v + get_x() ); set_x( v ); }
	void set_offsetRight( float v ){ set_width( -v - get_x() ); }
	void set_offsetTop( float v ){ style.height.set( get_height() - v + get_y() ); set_y( v ); }
	void set_offsetBottom( float v ){ set_height( -v - get_y() ); }
	
	SGS_PROPERTY_FUNC( READ get_x WRITE set_x ) SGS_ALIAS( sgsMaybe<float> x );
	SGS_PROPERTY_FUNC( READ get_y WRITE set_y ) SGS_ALIAS( sgsMaybe<float> y );
	SGS_PROPERTY_FUNC( READ get_width WRITE set_width ) SGS_ALIAS( sgsMaybe<float> width );
	SGS_PROPERTY_FUNC( READ get_height WRITE set_height ) SGS_ALIAS( sgsMaybe<float> height );
	SGS_PROPERTY_FUNC( READ get_q0x WRITE set_q0x ) SGS_ALIAS( sgsMaybe<float> q0x );
	SGS_PROPERTY_FUNC( READ get_q0y WRITE set_q0y ) SGS_ALIAS( sgsMaybe<float> q0y );
	SGS_PROPERTY_FUNC( READ get_q1x WRITE set_q1x ) SGS_ALIAS( sgsMaybe<float> q1x );
	SGS_PROPERTY_FUNC( READ get_q1y WRITE set_q1y ) SGS_ALIAS( sgsMaybe<float> q1y );
	SGS_PROPERTY_FUNC( READ get_nonClientTop WRITE set_nonClientTop ) SGS_ALIAS( sgsMaybe<float> nonClientTop );
	SGS_PROPERTY_FUNC( READ get_nonClientLeft WRITE set_nonClientLeft ) SGS_ALIAS( sgsMaybe<float> nonClientLeft );
	SGS_PROPERTY_FUNC( READ get_nonClientRight WRITE set_nonClientRight ) SGS_ALIAS( sgsMaybe<float> nonClientRight );
	SGS_PROPERTY_FUNC( READ get_nonClientBottom WRITE set_nonClientBottom ) SGS_ALIAS( sgsMaybe<float> nonClientBottom );
	SGS_PROPERTY_FUNC( READ get_visible WRITE set_visible ) SGS_ALIAS( sgsMaybe<bool> visible );
	SGS_PROPERTY_FUNC( READ get_index WRITE set_index ) SGS_ALIAS( sgsMaybe<int> index );
	SGS_PROPERTY_FUNC( READ get_topmost WRITE set_topmost ) SGS_ALIAS( sgsMaybe<bool> topmost );
	SGS_PROPERTY_FUNC( READ get_minWidth WRITE set_minWidth ) SGS_ALIAS( sgsMaybe<float> minWidth );
	SGS_PROPERTY_FUNC( READ get_maxWidth WRITE set_maxWidth ) SGS_ALIAS( sgsMaybe<float> maxWidth );
	SGS_PROPERTY_FUNC( READ get_minHeight WRITE set_minHeight ) SGS_ALIAS( sgsMaybe<float> minHeight );
	SGS_PROPERTY_FUNC( READ get_maxHeight WRITE set_maxHeight ) SGS_ALIAS( sgsMaybe<float> maxHeight );
	SGS_PROPERTY_FUNC( READ get_marginLeft WRITE set_marginLeft ) SGS_ALIAS( sgsMaybe<float> marginLeft );
	SGS_PROPERTY_FUNC( READ get_marginRight WRITE set_marginRight ) SGS_ALIAS( sgsMaybe<float> marginRight );
	SGS_PROPERTY_FUNC( READ get_marginTop WRITE set_marginTop ) SGS_ALIAS( sgsMaybe<float> marginTop );
	SGS_PROPERTY_FUNC( READ get_marginBottom WRITE set_marginBottom ) SGS_ALIAS( sgsMaybe<float> marginBottom );
	SGS_PROPERTY_FUNC( READ get_paddingLeft WRITE set_paddingLeft ) SGS_ALIAS( sgsMaybe<float> paddingLeft );
	SGS_PROPERTY_FUNC( READ get_paddingRight WRITE set_paddingRight ) SGS_ALIAS( sgsMaybe<float> paddingRight );
	SGS_PROPERTY_FUNC( READ get_paddingTop WRITE set_paddingTop ) SGS_ALIAS( sgsMaybe<float> paddingTop );
	SGS_PROPERTY_FUNC( READ get_paddingBottom WRITE set_paddingBottom ) SGS_ALIAS( sgsMaybe<float> paddingBottom );
	SGS_PROPERTY_FUNC( READ get_posMode WRITE set_posMode ) SGS_ALIAS( sgsMaybe<int8_t> posMode );
	SGS_PROPERTY_FUNC( READ get_stackMode WRITE set_stackMode ) SGS_ALIAS( sgsMaybe<int8_t> stackMode );
	SGS_PROPERTY_FUNC( READ get_overflow WRITE set_overflow ) SGS_ALIAS( sgsMaybe<bool> overflow );
	SGS_PROPERTY_FUNC( READ get_backgroundColor WRITE set_backgroundColor ) SGS_ALIAS( sgsMaybe<UIColor> backgroundColor );
	SGS_PROPERTY_FUNC( READ get_textColor WRITE set_textColor ) SGS_ALIAS( sgsMaybe<UIColor> textColor );
	SGS_PROPERTY_FUNC( READ get_halign WRITE set_halign ) SGS_ALIAS( sgsMaybe<uint16_t> halign );
	SGS_PROPERTY_FUNC( READ get_valign WRITE set_valign ) SGS_ALIAS( sgsMaybe<uint16_t> valign );
	SGS_PROPERTY_FUNC( READ get_cursor WRITE set_cursor ) SGS_ALIAS( sgsVariable cursor );
	SGS_PROPERTY_FUNC( READ get_font WRITE set_font ) SGS_ALIAS( sgsString font );
	SGS_PROPERTY_FUNC( READ get_image WRITE set_image ) SGS_ALIAS( sgsString image );
	SGS_PROPERTY_FUNC( READ get_icon WRITE set_icon ) SGS_ALIAS( sgsString icon );
	SGS_PROPERTY_FUNC( READ get_fontSize WRITE set_fontSize ) SGS_ALIAS( sgsMaybe<float> fontSize );
	SGS_PROPERTY_FUNC( READ get_renderfunc WRITE set_renderfunc ) SGS_ALIAS( sgsVariable renderfunc );
	
	SGS_PROPERTY_FUNC( READ get_nonClientH WRITE set_nonClientH ) SGS_ALIAS( sgsMaybe<float> nonClientH );
	SGS_PROPERTY_FUNC( READ get_nonClientV WRITE set_nonClientV ) SGS_ALIAS( sgsMaybe<float> nonClientV );
	SGS_PROPERTY_FUNC( READ get_nonClient WRITE set_nonClient ) SGS_ALIAS( sgsMaybe<float> nonClient );
	SGS_PROPERTY_FUNC( READ get_marginH WRITE set_marginH ) SGS_ALIAS( sgsMaybe<float> marginH );
	SGS_PROPERTY_FUNC( READ get_marginV WRITE set_marginV ) SGS_ALIAS( sgsMaybe<float> marginV );
	SGS_PROPERTY_FUNC( READ get_margin WRITE set_margin ) SGS_ALIAS( sgsMaybe<float> margin );
	SGS_PROPERTY_FUNC( READ get_paddingH WRITE set_paddingH ) SGS_ALIAS( sgsMaybe<float> paddingH );
	SGS_PROPERTY_FUNC( READ get_paddingV WRITE set_paddingV ) SGS_ALIAS( sgsMaybe<float> paddingV );
	SGS_PROPERTY_FUNC( READ get_padding WRITE set_padding ) SGS_ALIAS( sgsMaybe<float> padding );
	SGS_PROPERTY_FUNC( READ get_anchorMode WRITE set_anchorMode ) SGS_ALIAS( int anchorMode );
	SGS_PROPERTY_FUNC( READ get_align WRITE set_align ) SGS_ALIAS( uint16_t align );
	
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateScroll ) float scroll_x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateScroll ) float scroll_y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK onLayoutChange ) bool nonclient;
	SGS_PROPERTY_FUNC( READ get_offsetLeft WRITE set_offsetLeft ) SGS_ALIAS( float offsetLeft );
	SGS_PROPERTY_FUNC( READ get_offsetRight WRITE set_offsetRight ) SGS_ALIAS( float offsetRight );
	SGS_PROPERTY_FUNC( READ get_offsetTop WRITE set_offsetTop ) SGS_ALIAS( float offsetTop );
	SGS_PROPERTY_FUNC( READ get_offsetBottom WRITE set_offsetBottom ) SGS_ALIAS( float offsetBottom );
	
	SGS_PROPERTY sgsVariable _cachedFont; SGS_GCREF( _cachedFont );
	SGS_METHOD void setFont( const sgsString& newFont, float newFontSize ){ style.font = newFont; set_fontSize( newFontSize ); }
	SGS_PROPERTY sgsVariable _cachedImage; SGS_GCREF( _cachedImage );
	SGS_PROPERTY sgsVariable _cachedIcon; SGS_GCREF( _cachedIcon );
	
	SGS_PROPERTY sgsVariable callback; SGS_GCREF( callback );
	SGS_PROPERTY sgsVariable data; SGS_GCREF( data );
	SGS_PROPERTY sgsVariable _interface; SGS_GCREF( _interface );
	
	// these are writable to allow controls with manual layouts to accept parameters from style rules and undo auto layouts
	SGS_PROPERTY READ float autoWidth;
	SGS_PROPERTY READ float autoHeight;
	// full rect
	SGS_PROPERTY float rx0;
	SGS_PROPERTY float rx1;
	SGS_PROPERTY float ry0;
	SGS_PROPERTY float ry1;
	float get_realWidth(){ return rx1 - rx0; }
	SGS_PROPERTY_FUNC( READ get_realWidth ) float realWidth;
	float get_realHeight(){ return ry1 - ry0; }
	SGS_PROPERTY_FUNC( READ get_realHeight ) float realHeight;
	// client rect
	SGS_PROPERTY float cx0;
	SGS_PROPERTY float cx1;
	SGS_PROPERTY float cy0;
	SGS_PROPERTY float cy1;
	float get_clientWidth(){ return cx1 - cx0; }
	SGS_PROPERTY_FUNC( READ get_clientWidth ) float clientWidth;
	float get_clientHeight(){ return cy1 - cy0; }
	SGS_PROPERTY_FUNC( READ get_clientHeight ) float clientHeight;
	// padded rect
	SGS_PROPERTY float px0;
	SGS_PROPERTY float px1;
	SGS_PROPERTY float py0;
	SGS_PROPERTY float py1;
	float get_paddedWidth(){ return px1 - px0; }
	SGS_PROPERTY_FUNC( READ get_paddedWidth ) float paddedWidth;
	float get_paddedHeight(){ return py1 - py0; }
	SGS_PROPERTY_FUNC( READ get_paddedHeight ) float paddedHeight;
	
	SGS_METHOD float calcWidth();
	SGS_METHOD float calcHeight();
	SGS_METHOD void _updateFullRect();
	SGS_METHOD void _updateChildRects();
	SGS_METHOD void _changedFullRect();
	
	SGS_PROPERTY bool _updatingLayout : 1; /* true if updating layout and don't want to trigger further layout changes */
	SGS_PROPERTY bool _roundedCoords : 1; /* true if final coords (r[xy][01]) should be rounded */
	SGS_PROPERTY bool _childAffectsLayout : 1; /* true if child has any influence over the layout and should be updated if child is */
	SGS_PROPERTY bool _clientRectFromPadded : 1; /* true to calculate relative position for child controls from the padded rect, not client rect */
	SGS_PROPERTY bool _neverHit : 1; /* true if cannot hit (regardless of hit test) */
	SGS_PROPERTY bool _layoutRectOverride : 1; /* true if EV_Layout overrides rect placement and default scrolling optimization would not work here */
	SGS_PROPERTY bool _disableClickBubbling : 1; /* true if button events (button up/down) should not bubble, impl = return 0 on default handler */
	SGS_PROPERTY READ bool mouseOn : 1;
	SGS_PROPERTY READ bool keyboardFocus : 1;
	SGS_PROPERTY READ int clicked;
	
	HandleArray m_children; SGS_GCREF( m_children );
	HandleArray m_sorted;
	sgsVariable m_events; SGS_GCREF( m_events );
	CmptArray m_components; SGS_GCREF( m_components );
	UIAnimArray m_animQueue; SGS_GCREF( m_animQueue );
};


struct UIQuery
{
	typedef sgsHandle< UIQuery > Handle;
	
	SGS_OBJECT;
	SGS_IFUNC(CONVERT) int sgs_convert( SGS_CTX, sgs_VarObj* obj, int type );
	SGS_IFUNC(GETINDEX) int sgs_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop );
	
	// utility functions
	bool _parseArgs( sgs_StkIdx stacksize );
	bool _checkCtrl( UIControl* ctrl );
	bool _hasCtrl( UIControl* ctrl );
	
	// query functions (args=unspecified number of selector strings, selector objects or UI controls)
	static UIQuery::Handle Create( /* args, */ UIFrame::Handle frame );
	SGS_METHOD UIQuery::Handle find( /* args */ );
	SGS_METHOD UIQuery::Handle children( /* args */ );
	SGS_METHOD UIQuery::Handle parent();
	SGS_METHOD UIQuery::Handle closest( /* args */ );
	SGS_METHOD UIQuery::Handle prev( /* args */ );
	SGS_METHOD UIQuery::Handle next( /* args */ );
	SGS_METHOD UIQuery::Handle prevAll( /* args */ );
	SGS_METHOD UIQuery::Handle nextAll( /* args */ );
	
	// filtering functions
	SGS_METHOD UIQuery::Handle filter( /* args */ );
	SGS_METHOD UIQuery::Handle first();
	SGS_METHOD UIQuery::Handle last();
	
	// data functions
	SGS_METHOD sgsVariable getAttr( sgsString key );
	SGS_METHOD UIQuery::Handle setAttr( sgsString key, sgsVariable value );
	SGS_METHOD UIQuery::Handle addClass( const sgsString& ss );
	SGS_METHOD UIQuery::Handle removeClass( const sgsString& ss );
	SGS_METHOD UIQuery::Handle bindEvent( sgsString name, sgsVariable callable );
	SGS_METHOD UIQuery::Handle unbindEvent( sgsString name, sgsVariable callable );
	SGS_METHOD UIQuery::Handle unbindEventAll( sgsString name );
	SGS_METHOD UIQuery::Handle callEvent( sgsString name, sgsVariable data );
	SGS_METHOD UIQuery::Handle animate( sgsVariable state, float length, sgsVariable func, sgsVariable oncomplete );
	SGS_METHOD UIQuery::Handle stop( bool nofinish );
	SGS_METHOD UIQuery::Handle dequeue();
	SGS_METHOD UIQuery::Handle skip( bool nofinish );
	
	// shortcuts
	SGS_METHOD UIQuery::Handle setVisible( bool visible );
	SGS_METHOD UIQuery::Handle show(){ return setVisible( true ); }
	SGS_METHOD UIQuery::Handle hide(){ return setVisible( false ); }
	
	SGS_PROPERTY READ UIFrame::Handle m_frame; SGS_GCREF( m_frame );
	StyleSelArray m_selectors;
	UIControl::HandleArray m_items; SGS_GCREF( m_items );
	
	// ArrayIterator interface
	static const char* IteratorTypeName;
	UIControl::Handle operator [] ( sgs_SizeVal i ) const { return m_items[ i ]; }
	sgs_SizeVal size() const { return m_items.size(); }
};



#endif // __SGS_UI_CONTROL__

