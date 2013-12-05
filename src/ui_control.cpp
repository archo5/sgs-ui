

#include "ui_control.h"

#include <algorithm>



inline float lerpf( float a, float b, float s )
{
	return a * (1-s) + b * s;
}


int UIControl_CtrlProc( SGS_CTX )
{
	SGSFN( "UIControl_CtrlProc" );
	sgs_Method( C );
	if( !sgs_IsObject( C, 0, UIControl::_sgs_interface ) )
		return sgs_Printf( C, SGS_WARNING, "expected UIControl as argument 0 / this" );
	UIControl* ctrl = sgs_GetVar<UIControl::Handle>()( C, 0 );
	
	UIEvent* event = sgs_GetVarObj<UIEvent>()( C, 1 );
	if( !event )
		return sgs_Printf( C, SGS_WARNING, "expected UIEvent as argument 1" );
	
	switch( event->type )
	{
	case EV_Layout:
		{
			float pr0x, pr0y, pr1x, pr1y;
			UIControl* prt = ctrl->parent;
			UIFrame* frame = ctrl->frame;
			if( prt )
			{
				pr0x = prt->rx0;
				pr0y = prt->ry0;
				pr1x = prt->rx1;
				pr1y = prt->ry1;
			}
			else if( frame )
			{
				pr0x = frame->x;
				pr0y = frame->y;
				pr1x = pr0x + frame->width;
				pr1y = pr0y + frame->height;
			}
			else
			{
				pr0x = 0;
				pr0y = 0;
				pr1x = 0;
				pr1y = 0;
			}
			ctrl->rx0 = ctrl->x;
			ctrl->ry0 = ctrl->y;
			ctrl->rx1 = ctrl->rx0 + ctrl->width;
			ctrl->ry1 = ctrl->ry0 + ctrl->height;
			ctrl->rx0 += lerpf( pr0x, pr1x, ctrl->q0x );
			ctrl->rx1 += lerpf( pr0x, pr1x, ctrl->q1x );
			ctrl->ry0 += lerpf( pr0y, pr1y, ctrl->q0y );
			ctrl->ry1 += lerpf( pr0y, pr1y, ctrl->q1y );
		}
		sgs_PushInt( C, 1 );
		return 1;
	
	case EV_Attach:
	case EV_Detach:
		{
			UIEvent e;
			e.type = EV_Layout;
			ctrl->niEvent( &e );
		}
		sgs_PushInt( C, 1 );
		return 1;
	
	case EV_HitTest:
		sgs_PushInt( C, event->x >= ctrl->rx0 && event->x <= ctrl->rx1 &&
			event->y >= ctrl->ry0 && event->y <= ctrl->ry1 );
		return 1;
	
	}
	
	sgs_PushInt( C, 0 );
	return 1;
}



UIEvent::UIEvent() : type(0), key(0), button(0), uchar(0), x(0), y(0), rx(0), ry(0)
{
}


UIFrame::UIFrame() : x(0), y(0), width(9999), height(9999), prevMouseX(0), prevMouseY(0)
{
}


void UIFrame::event( UIEvent* e )
{
	if( root )
		root->niEvent( e );
}

void UIFrame::render()
{
	if( root )
		root->niRender();
}

void UIFrame::doMouseMove( float x, float y )
{
	UIEvent e;
	e.type = EV_MouseMove;
	e.x = x;
	e.y = y;
	e.rx = x - prevMouseX;
	e.ry = y - prevMouseY;
	prevMouseX = x;
	prevMouseY = y;
	event( &e );
}

void UIFrame::doMouseButton( int btn, bool down )
{
	UIEvent e;
	e.type = down ? EV_ButtonDown : EV_ButtonUp;
	e.button = btn;
	event( &e );
}

void UIFrame::doKeyPress( int key, bool down )
{
	UIEvent e;
	e.type = down ? EV_KeyDown : EV_KeyUp;
	e.key = key;
	event( &e );
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
	q0x(0.0f), q0y(0.0f), q1x(0.0f), q1y(0.0f), index(0),
	rx0(0.0f), rx1(0.0f), ry0(0.0f), ry1(0.0f)
{
	sgs_PushCFunction( C, UIControl_CtrlProc );
	callback = sgsVariable( C, -1 );
	sgs_Pop( C, 1 );
	
	sgs_PushDict( C, 0 );
	m_events = sgsVariable( C, -1 );
	sgs_Pop( C, 1 );
}

int UIControl::niEvent( UIEvent* e )
{
	int orig = sgs_StackSize( C );
	sgs_PushVar( C, Handle( m_sgsObject, C ) );
	UI_PushEvent( C, e );
	callback.push();
	sgs_ThisCall( C, 1, 1 );
	int ret = sgs_GetInt( C, -1 );
	sgs_SetStackSize( C, orig );
	return ret;
}

void UIControl::niRender()
{
	int orig = sgs_StackSize( C );
	sgs_PushVar( C, Handle( m_sgsObject, C ) );
	renderfunc.push();
	sgs_ThisCall( C, 0, 0 );
	sgs_SetStackSize( C, orig );
	
	for( HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
	{
		(*it)->niRender();
	}
}

void UIControl::updateLayout()
{
	UIEvent e;
	e.type = EV_Layout;
	niEvent( &e );
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
	
	UIEvent e;
	e.type = EV_Attach;
	ch->niEvent( &e );
	
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
	ch->frame = UIFrame::Handle();
	sortChildren();
	
	UIEvent e;
	e.type = EV_Detach;
	ch->niEvent( &e );
	
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


bool UIControl::bindEvent( std::string name, sgsVariable callable )
{
	m_events.push();
	// try reading the existing array
	sgs_PushStringBuf( C, name.c_str(), name.size() );
	if( sgs_PushIndexExt( C, -2, -1, 0 ) )
	{
		callable.push();
		sgs_PushArray( C, 1 ); // create an array if it doesn't exist
		sgs_StoreIndexExt( C, -3, -2, 0 );
	}
	else
	{
		callable.push();
		// check if item isn't already in the array
		sgs_SizeVal ii = sgs_ObjectAction( C, -2, SGS_ACT_ARRAY_FIND, -1 );
		if( ii != SGS_ENOTFND )
			return false;
		// append new item
		sgs_ObjectAction( C, -2, SGS_ACT_ARRAY_PUSH, 1 );
	}
	return true;
}

bool UIControl::unbindEvent( std::string name, sgsVariable callable )
{
	m_events.push();
	// check if there's an entry for the event
	sgs_PushStringBuf( C, name.c_str(), name.size() );
	if( sgs_PushIndexExt( C, -2, -1, 0 ) )
		return false;
	// remove item from array
	callable.push();
	return !!sgs_ObjectAction( C, -2, SGS_ACT_ARRAY_RM_ONE, -1 );
}

bool UIControl::callEvent( std::string name, UIEvent* e )
{
	m_events.push();
	// check if there's an entry for the event
	sgs_PushStringBuf( C, name.c_str(), name.size() );
	if( sgs_PushIndexExt( C, -2, -1, 0 ) )
		return false;
	// iterate the array
	sgs_PushIterator( C, -1 );
	while( sgs_IterAdvance( C, -1 ) > 0 )
	{
		sgs_PushHandle( C, UIControl::Handle( m_sgsObject, C ) );
		UI_PushEvent( C, e );
		sgs_IterPushData( C, -3, 0, 1 );
		sgs_ThisCall( C, 1, 0 );
	}
	return true;
}


int UIControl::sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int )
{
	UIControl* ctrl = static_cast<UIControl*>(obj->data);
	for( HandleArray::iterator it = ctrl->m_children.begin(), itend = ctrl->m_children.end(); it != itend; ++it )
		it->gcmark();
	ctrl->parent.gcmark();
	ctrl->frame.gcmark();
	ctrl->callback.gcmark();
	ctrl->data.gcmark();
	return SGS_SUCCESS;
}




