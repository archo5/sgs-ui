

#ifndef __SGS_UI_CONTROL__
#define __SGS_UI_CONTROL__


#include <sgs_cppbc.h>

#include <vector>


#define EV_Paint      1
#define EV_Layout     2
#define EV_KeyDown    10
#define EV_KeyUp      11
#define EV_Char       12
#define EV_ButtonDown 20
#define EV_ButtonUp   21
#define EV_MouseMove  22
#define EV_Attach     30
#define EV_Detach     31
#define EV_HitTest    32


struct UIEvent
{
	SGS_OBJECT;
	
	UIEvent();
	
	SGS_PROPERTY int type;
	SGS_PROPERTY int key;
	SGS_PROPERTY int button;
	SGS_PROPERTY int uchar;
	SGS_PROPERTY int x;
	SGS_PROPERTY int y;
	SGS_PROPERTY int rx;
	SGS_PROPERTY int ry;
};


struct UIControl;
struct UIFrame
{
	typedef sgsHandle< UIFrame > Handle;
	
	SGS_OBJECT;
	
	UIFrame();
	
	SGS_METHOD void event( UIEvent* e );
	SGS_METHOD void render();
	
	SGS_IFUNC(SGS_OP_GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int );
	
	SGS_PROPERTY sgsVariable render_image;
	SGS_PROPERTY sgsVariable render_text;
	SGS_PROPERTY sgsVariable ui_event;
	SGS_PROPERTY float x;
	SGS_PROPERTY float y;
	SGS_PROPERTY float width;
	SGS_PROPERTY float height;
	SGS_PROPERTY sgsHandle< UIControl > root;
};


struct UIControl
{
	typedef sgsHandle< UIControl > Handle;
	typedef std::vector< Handle > HandleArray;
	
	SGS_OBJECT;
	
	UIControl();
	
	int niEvent( UIEvent* event );
	void niRender();
	
	SGS_METHOD bool addChild( UIControl::Handle ch );
	SGS_METHOD bool removeChild( UIControl::Handle ch );
	SGS_METHOD UIControl::Handle findChild( std::string name );
	SGS_METHOD sgsVariable children();
	SGS_METHOD void sortChildren();
	
	SGS_IFUNC(SGS_OP_GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int );
	
	SGS_PROPERTY std::string name;
	SGS_PROPERTY std::string caption;
	SGS_PROPERTY float x;
	SGS_PROPERTY float y;
	SGS_PROPERTY float width;
	SGS_PROPERTY float height;
	SGS_PROPERTY float q0x;
	SGS_PROPERTY float q0y;
	SGS_PROPERTY float q1x;
	SGS_PROPERTY float q1y;
	SGS_PROPERTY int index;
	SGS_PROPERTY std::string type;
	SGS_PROPERTY READ Handle parent;
	SGS_PROPERTY READ UIFrame::Handle frame;
	SGS_PROPERTY sgsVariable callback;
	SGS_PROPERTY sgsVariable renderfunc;
	
	SGS_PROPERTY READ float rx0;
	SGS_PROPERTY READ float rx1;
	SGS_PROPERTY READ float ry0;
	SGS_PROPERTY READ float ry1;
	
	HandleArray m_children;
	HandleArray m_sorted;
};



inline void UI_PushEvent( SGS_CTX, UIEvent* e ){ sgs_PushClassFrom( C, e ); }


#endif // __SGS_UI_CONTROL__

