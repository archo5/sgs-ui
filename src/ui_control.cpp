

#include "ui_control.h"

#include <algorithm>



UIEvent::UIEvent() : type(0), key(0), button(0), uchar(0), x(0), y(0), rx(0), ry(0)
{
}


UIFrame::UIFrame() : x(0), y(0), width(9999), height(9999)
{
}


void UIFrame::event( UIEvent* e )
{
	if( root )
		root->_event( e );
}

int UIFrame::sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int )
{
	UIFrame* frame = static_cast<UIFrame*>(obj->data);
	frame->render_image.gcmark();
	frame->render_text.gcmark();
	frame->ui_event.gcmark();
	frame->root.gcmark();
	return SGS_SUCCESS;
}


UIControl::UIControl() :
	x(0.0f), y(0.0f), width(0.0f), height(0.0f),
	q0x(0.0f), q0y(0.0f), q1x(0.0f), q1y(0.0f), index(0)
{
}

void UIControl::_event( UIEvent* e )
{
	sgs_PushVar( C, e );
	sgs_Call( C, 1, 0 );
	
	for( HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
	{
		if( *it )
			(*it)->_event( e );
	}
}

bool UIControl::addChild( UIControl::Handle ch )
{
	for( HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
	{
		if( *it == ch )
			return false;
	}
	m_children.push_back( ch );
	m_sorted.push_back( ch );
	ch->parent = Handle( m_sgsObject, C );
	ch->frame = frame;
	sortChildren();
	return true;
}

bool UIControl::removeChild( UIControl::Handle ch )
{
	bool found = false;
	for( HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
	{
		if( *it == ch )
		{
			m_children.erase( it );
			found = true;
			break;
		}
	}
	for( HandleArray::iterator it = m_sorted.begin(), itend = m_sorted.end(); it != itend; ++it )
	{
		if( *it == ch )
		{
			m_sorted.erase( it );
			found = true;
			break;
		}
	}
	ch->parent = Handle();
	ch->frame = Handle();
	sortChildren();
	return found;
}

UIControl::Handle UIControl::findChild( std::string name )
{
	for( HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
	{
		if( (*it)->name != name )
			continue;
		return *it;
	}
	return UIControl::Handle();
}

sgsVariable UIControl::children()
{
	sgs_SetStackSize( C, 0 );
	for( HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
		sgs_PushHandle( C, *it );
	sgs_PushArray( C, sgs_StackSize( C ) );
	return sgsVariable( C, -1 );
}


bool UIControl_chSortCmpFunc( const UIControl::Handle& h1, const UIControl::Handle& h2 )
{
	return h1->index < h2->index;
}

void UIControl::sortChildren()
{
	std::sort( m_sorted.begin(), m_sorted.end(), UIControl_chSortCmpFunc );
}

int UIControl::sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int )
{
	UIControl* ctrl = static_cast<UIControl*>(obj->data);
	for( HandleArray::iterator it = ctrl->m_children.begin(), itend = ctrl->m_children.end(); it != itend; ++it )
		it->gcmark();
	return SGS_SUCCESS;
}




