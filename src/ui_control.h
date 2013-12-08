

#ifndef __SGS_UI_CONTROL__
#define __SGS_UI_CONTROL__


#include <sgs_cppbc.h>

#include <vector>


#define EV_Paint      1
#define EV_Layout     2
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
#define EV_Attach     30
#define EV_Detach     31
#define EV_HitTest    32

#define Mouse_ButtonL 0
#define Mouse_ButtonR 1
#define Mouse_ButtonM 2
#define Mouse_Button_Count 3


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
	
	SGS_METHOD void event( UIEvent* e );
	SGS_METHOD void render();
	SGS_METHOD void handleMouseMove();
	SGS_METHOD void setFocus( UIControl* ctrl );
	
	// event generation shortcuts
	SGS_METHOD void doMouseMove( float x, float y );
	SGS_METHOD void doMouseButton( int btn, bool down );
	SGS_METHOD void doKeyPress( int key, bool down );
	
	SGS_IFUNC(SGS_OP_GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int );
	
	SGS_PROPERTY sgsVariable render_image;
	SGS_PROPERTY sgsVariable render_text;
	SGS_PROPERTY sgsVariable ui_event;
	SGS_PROPERTY float x;
	SGS_PROPERTY float y;
	SGS_PROPERTY float width;
	SGS_PROPERTY float height;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK initRoot ) sgsHandle< UIControl > root;
	
	SGS_PROPERTY READ float mouseX;
	SGS_PROPERTY READ float mouseY;
	
	void preRemoveControl( UIControl* ctrl );
	void initRoot();
	
	UIControl* m_hover;
	UIControl* m_focus;
	UIControl* m_clicktargets[ Mouse_Button_Count ];
};


struct UIControl
{
	typedef sgsHandle< UIControl > Handle;
	typedef std::vector< Handle > HandleArray;
	
	SGS_OBJECT;
	
	UIControl();
	
	int niEvent( UIEvent* event );
	void niBubblingEvent( UIEvent* e );
	void niRender();
	void updateLayout();
	
	void setFrame( UIFrame::Handle fh );
	
	SGS_METHOD bool addChild( UIControl::Handle ch );
	SGS_METHOD bool removeChild( UIControl::Handle ch );
	SGS_METHOD UIControl::Handle findChild( std::string name );
	SGS_METHOD sgsVariable children();
	SGS_METHOD void sortChildren();
	SGS_METHOD void sortSiblings();
	
	SGS_METHOD bool bindEvent( std::string name, sgsVariable callable );
	SGS_METHOD bool unbindEvent( std::string name, sgsVariable callable );
	SGS_METHOD bool callEvent( std::string name, UIEvent* e );
	
	SGS_IFUNC(SGS_OP_GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int );
	
	SGS_PROPERTY std::string name;
	SGS_PROPERTY std::string caption;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float width;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float height;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float q0x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float q0y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float q1x;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK updateLayout ) float q1y;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK sortSiblings ) int index;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK sortSiblings ) bool topmost;
	SGS_PROPERTY std::string type;
	SGS_PROPERTY READ Handle parent;
	SGS_PROPERTY READ UIFrame::Handle frame;
	SGS_PROPERTY sgsVariable callback;
	SGS_PROPERTY sgsVariable renderfunc;
	SGS_PROPERTY sgsVariable data;
	
	SGS_PROPERTY READ float rx0;
	SGS_PROPERTY READ float rx1;
	SGS_PROPERTY READ float ry0;
	SGS_PROPERTY READ float ry1;
	
	SGS_PROPERTY bool _updatingLayout;
	SGS_PROPERTY READ bool mouseOn;
	SGS_PROPERTY READ bool clicked;
	SGS_PROPERTY READ bool keyboardFocus;
	
	HandleArray m_children;
	HandleArray m_sorted;
	sgsVariable m_events;
};



inline void UI_PushEvent( SGS_CTX, UIEvent* e ){ sgs_PushClassFrom( C, e ); }


#endif // __SGS_UI_CONTROL__

