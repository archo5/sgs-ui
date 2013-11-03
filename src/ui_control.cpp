

#include "ui_control.h"


UIControl::Handle UIControl::findChild( std::string name )
{
	for( ControlSet::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
		return *it;
	return UIControl::Handle();
}

sgsVariable UIControl::children()
{
	sgs_SetStackSize( C, 0 );
	for( ControlSet::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
		sgs_PushHandle( C, *it );
	sgs_PushArray( C, sgs_StackSize( C ) );
	return sgsVariable( C, -1 );
}

int UIControl::sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int )
{
	UIControl* ctrl = static_cast<UIControl*>(obj->data);
	for( ControlSet::iterator it = ctrl->m_children.begin(), itend = ctrl->m_children.end(); it != itend; ++it )
		it->gcmark();
	return SGS_SUCCESS;
}

