

#ifndef __SGS_UI_CONTROL__
#define __SGS_UI_CONTROL__


#include <sgs_cppbc.h>

#include <set>


struct UIControl
{
	typedef sgsHandle< UIControl > Handle;
	typedef std::set< Handle > ControlSet;
	
	SGS_OBJECT;
	
	SGS_METHOD void addChild( UIControl::Handle ch ){ m_children.insert( ch ); }
	SGS_METHOD void removeChild( UIControl::Handle ch ){ m_children.erase( ch ); }
	SGS_METHOD UIControl::Handle findChild( std::string name );
	SGS_METHOD sgsVariable children();
	
	SGS_IFUNC(SGS_OP_GCMARK) int sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int );
	
	SGS_PROPERTY std::string m_name;
	SGS_PROPERTY float m_x;
	SGS_PROPERTY float m_y;
	SGS_PROPERTY float m_width;
	SGS_PROPERTY float m_height;
	SGS_PROPERTY float m_q0x;
	SGS_PROPERTY float m_q0y;
	SGS_PROPERTY float m_q1x;
	SGS_PROPERTY float m_q1y;
	
	ControlSet m_children;
};


#endif // __SGS_UI_CONTROL__

