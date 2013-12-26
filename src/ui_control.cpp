

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
	
	// default return value
	sgs_PushInt( C, 1 );
	switch( event->type )
	{
	case EV_Layout:
		{
			float pr0x, pr0y, pr1x, pr1y;
			UIControl* prt = ctrl->parent;
			UIFrame* frame = ctrl->frame;
			if( prt )
			{
				pr0x = prt->rx0 + prt->nc_left;
				pr0y = prt->ry0 + prt->nc_top;
				pr1x = prt->rx1 - prt->nc_right;
				pr1y = prt->ry1 - prt->nc_bottom;
				if( !ctrl->nonclient )
				{
					pr0x += prt->scroll_x;
					pr0y += prt->scroll_y;
					pr1x += prt->scroll_x;
					pr1y += prt->scroll_y;
				}
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
		for( UIControl::HandleArray::iterator it = ctrl->m_children.begin(), itend = ctrl->m_children.end(); it != itend; ++it )
			(*it)->updateLayout();
		if( ctrl->parent.object )
			ctrl->parent->updateLayout();
		return 1;
	
	case EV_Attach:
	case EV_Detach:
	case EV_AddChild:
	case EV_RemChild:
		ctrl->updateLayout();
		sgs_PushInt( C, 1 );
		return 1;
	
	case EV_HitTest:
		if( event->x >= ctrl->rx0 && event->x <= ctrl->rx1 &&
			event->y >= ctrl->ry0 && event->y <= ctrl->ry1 )
		{
			if( event->x >= ctrl->rx0 + ctrl->nc_left && event->x <= ctrl->rx1 - ctrl->nc_right &&
				event->y >= ctrl->ry0 + ctrl->nc_top && event->y <= ctrl->ry1 - ctrl->nc_bottom )
				sgs_PushInt( C, Hit_Client );
			else
				sgs_PushInt( C, Hit_NonClient );
		}
		else
			sgs_PushInt( C, Hit_None );
		return 1;
		
	case EV_MouseMove:
	case EV_MouseWheel:
	case EV_KeyDown:
	case EV_KeyUp:
		// allow bubbling
		return 1;
	
	case EV_MouseEnter:
		ctrl->mouseOn = true;
		return 1;
	case EV_MouseLeave:
		ctrl->mouseOn = false;
		return 1;
	case EV_ButtonDown:
		ctrl->clicked = ctrl->mouseOn;
		ctrl->callEvent( "mousedown", event );
		if( ctrl->frame.object )
			ctrl->frame->setFocus( ctrl );
		return 1;
	case EV_ButtonUp:
		ctrl->callEvent( "mouseup", event );
		if( ctrl->clicked && ctrl->mouseOn )
			ctrl->callEvent( "click", event );
		ctrl->clicked = false;
		return 1;
		
	case EV_FocusEnter:
		ctrl->keyboardFocus = true;
		return 1;
	case EV_FocusLeave:
		ctrl->keyboardFocus = false;
		return 1;
	case EV_NeedFocus:
		return 0; // don't take focus by default
	
	}
	
	sgs_PushInt( C, 0 );
	return 1;
}



UIEvent::UIEvent() : type(0), key(0), button(0), uchar(0), x(0), y(0), rx(0), ry(0)
{
}


UIFrame::UIFrame() : x(0), y(0), width(9999), height(9999), mouseX(0), mouseY(0),
	m_hover(NULL), m_focus(NULL), m_timerAutoID(1)
{
	memset( m_clicktargets, 0, sizeof(m_clicktargets) );
	memset( m_clickoffsets, 0, sizeof(m_clickoffsets) );
	
	root = createControl( "root" );
	root->q1x = 1;
	root->q1y = 1;
	root->updateLayout();
}

UIFrame::~UIFrame()
{
	root = UIControl::Handle();
}

UIControl::Handle UIFrame::createControl( std::string type )
{
	UIControl* ctrl = SGS_PUSHCLASS( C, UIControl, () );
	ctrl->type = type;
	ctrl->frame = Handle( m_sgsObject, C );
	ctrl->id = m_controlIDGen.GetID();
	UIControl::Handle handle = sgs_GetVar< UIControl::Handle >()( C, -1 );
	sgs_Pop( C, 1 );
	return handle;
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

void UIFrame::handleMouseMove()
{
	// find new mouse-over item
	UIEvent htev;
	htev.type = EV_HitTest;
	htev.x = mouseX;
	htev.y = mouseY;
	
	UIControl* ctrl = root, *prevhover = m_hover;
	int nonclient = -1;
	m_hover = NULL;
	while( ctrl && m_hover != ctrl )
	{
		m_hover = ctrl;
		for( UIControl::HandleArray::reverse_iterator it = ctrl->m_sorted.rbegin(), itend = ctrl->m_sorted.rend(); it != itend; ++it )
		{
			UIControl* nc = *it;
			if( !nc->visible )
				continue;
			if( nonclient < 0 || nonclient == ( nc->nonclient ? 1 : 0 ) )
			{
				int hit = nc->niEvent( &htev );
				if( hit != Hit_None )
				{
					ctrl = nc;
					nonclient = hit == Hit_NonClient ? 1 : 0;
					break;
				}
			}
		}
	}
	
	if( m_hover != prevhover )
	{
		UIEvent mev;
		mev.x = mouseX;
		mev.y = mouseY;
		if( prevhover ){ mev.type = EV_MouseLeave; prevhover->niBubblingEvent( &mev ); }
		if( m_hover ){ mev.type = EV_MouseEnter; m_hover->niBubblingEvent( &mev ); }
	}
}

void UIFrame::setFocus( UIControl* ctrl )
{
	if( m_focus == ctrl )
		return;
	
	UIEvent e;
	e.type = EV_NeedFocus;
	
	if( ctrl->niEvent( &e ) )
	{
		if( m_focus ){ e.type = EV_FocusLeave; m_focus->niEvent( &e ); }
		m_focus = ctrl;
		if( m_focus ){ e.type = EV_FocusEnter; m_focus->niEvent( &e ); }
	}
}

void UIFrame::doMouseMove( float x, float y )
{
	// send event
	UIEvent e;
	e.type = EV_MouseMove;
	e.x = x;
	e.y = y;
	e.rx = x - mouseX;
	e.ry = y - mouseY;
	event( &e );
	mouseX = x;
	mouseY = y;
	
	handleMouseMove();
	
	if( m_hover )
	{
		UIEvent mev;
		mev.x = mouseX;
		mev.y = mouseY;
		if( m_hover ){ mev.type = EV_MouseMove; m_hover->niBubblingEvent( &mev ); }
	}
	
	if( root.object )
	{
		root->callEvent( "globalmousemove", &e );
	}
}

void UIFrame::doMouseButton( int btn, bool down )
{
	if( btn < 0 || btn >= Mouse_Button_Count )
		return;
	
	UIEvent e;
	e.type = down ? EV_ButtonDown : EV_ButtonUp;
	e.button = btn;
	e.x = mouseX;
	e.y = mouseY;
	
	if( !down )
	{
		if( m_clicktargets[ btn ] )
		{
			m_clicktargets[ btn ]->niBubblingEvent( &e );
			m_clicktargets[ btn ] = NULL;
		}
	}
	else if( m_hover )
	{
		m_clicktargets[ btn ] = m_hover;
		m_clickoffsets[ btn ][0] = mouseX - m_hover->rx0;
		m_clickoffsets[ btn ][1] = mouseY - m_hover->ry0;
		m_hover->niBubblingEvent( &e );
	}
	
	if( root.object )
	{
		root->callEvent( down ? "globalbuttondown" : "globalbuttonup", &e );
	}
}

void UIFrame::doMouseWheel( float down )
{
	UIEvent e;
	e.type = EV_MouseWheel;
	e.x = e.y = down;
	
	if( m_hover )
	{
		m_hover->niBubblingEvent( &e );
	}
	
	if( root.object )
	{
		root->callEvent( "globalmousewheel", &e );
	}
}

void UIFrame::doKeyPress( int key, bool down )
{
	UIEvent e;
	e.type = down ? EV_KeyDown : EV_KeyUp;
	e.key = key;
	
	if( m_focus )
		m_focus->niEvent( &e );
}

void UIFrame::doPutChar( int chr )
{
	UIEvent e;
	e.type = EV_Char;
	e.uchar = chr;
	
	if( m_focus )
		m_focus->niEvent( &e );
}

int64_t UIFrame::_setTimer( float t, sgsVariable func, bool one_shot )
{
	if( !m_timerAutoID )
		m_timerAutoID++;
	
	UITimerData td = { m_timerAutoID, one_shot ? 0.0f : t, t, func };
	
	m_timers[ td.id ] = td;
	return m_timerAutoID++;
}

void UIFrame::_clearTimer( int64_t id )
{
	UITimerMap::iterator it = m_timers.find( id );
	if( it != m_timers.end() )
		m_timers.erase( it );
}

void UIFrame::processTimers( float t )
{
	UITimerMap::iterator it = m_timers.begin();
	while( it != m_timers.end() )
	{
		UITimerData* td = &it->second;
		td->timeout -= t;
		if( td->timeout < 0 )
		{
			td->func.push( C );
			sgs_Call( C, 0, 0 );
			
			if( td->interval )
				td->timeout += td->interval;
			else
			{
				m_timers.erase( it++ );
				continue;
			}
		}
		++it;
	}
}

bool UIFrame::pushScissorRect( float x0, float y0, float x1, float y1 )
{
	if( m_scissorRects.size() )
	{
		UIRect& pr = *--m_scissorRects.end();
		if( x0 < pr.x0 ) x0 = pr.x0;
		if( y0 < pr.y0 ) y0 = pr.y0;
		if( x1 > pr.x1 ) x1 = pr.x1;
		if( y1 > pr.y1 ) y1 = pr.y1;
	}
	return pushScissorRectUnclipped( x0, y0, x1, y1 );
}

bool UIFrame::pushScissorRectUnclipped( float x0, float y0, float x1, float y1 )
{
	if( x0 >= x1 || y0 >= y1 )
		return false;
	UIRect r = { x0, y0, x1, y1 };
	m_scissorRects.push_back( r );
	_applyScissorState();
	return true;
}

bool UIFrame::popScissorRect()
{
	if( m_scissorRects.size() )
	{
		m_scissorRects.pop_back();
		_applyScissorState();
		return true;
	}
	return false;
}

void UIFrame::_applyScissorState()
{
	if( !scissor_func.not_null() )
		return;
	
	int ssz = sgs_StackSize( C );
	if( m_scissorRects.size() )
	{
		UIRect& rect = *--m_scissorRects.end();
		sgs_PushReal( C, rect.x0 );
		sgs_PushReal( C, rect.y0 );
		sgs_PushReal( C, rect.x1 );
		sgs_PushReal( C, rect.y1 );
		scissor_func.push( C );
		sgs_Call( C, 4, 0 );
	}
	else
	{
		scissor_func.push( C );
		sgs_Call( C, 0, 0 );
	}
	sgs_SetStackSize( C, ssz );
}

void UIFrame::updateLayout()
{
	root->updateLayout();
}

void UIFrame::preRemoveControl( UIControl* ctrl )
{
	if( m_hover == ctrl )
		m_hover = NULL;
	if( m_focus == ctrl )
		m_focus = NULL;
	for( int i = 0; i < Mouse_Button_Count; ++i )
	{
		if( m_clicktargets[ i ] == ctrl )
			m_clicktargets[ i ] = NULL;
	}
}

int UIFrame::sgs_gcmark( SGS_CTX, sgs_VarObj* obj, int )
{
	UIFrame* frame = static_cast<UIFrame*>(obj->data);
	frame->render_image.gcmark();
	frame->render_text.gcmark();
	frame->clipboard_func.gcmark();
	frame->root.gcmark();
	return SGS_SUCCESS;
}


UIControl::UIControl() :
	id( UI_NO_ID ), x(0.0f), y(0.0f), width(0.0f), height(0.0f),
	q0x(0.0f), q0y(0.0f), q1x(0.0f), q1y(0.0f),
	scroll_x(0.0f), scroll_y(0.0f),
	nc_top(0.0f), nc_left(0.0f), nc_right(0.0f), nc_bottom(0.0f),
	visible(true), index(0), topmost(false), nonclient(false),
	rx0(0.0f), rx1(0.0f), ry0(0.0f), ry1(0.0f),
	_updatingLayout(false), mouseOn(false), clicked(false), keyboardFocus(false)
{
	sgs_PushCFunction( C, UIControl_CtrlProc );
	callback = sgsVariable( C, -1 );
	sgs_Pop( C, 1 );
	
	sgs_PushDict( C, 0 );
	m_events = sgsVariable( C, -1 );
	sgs_Pop( C, 1 );
}

UIControl::~UIControl()
{
	frame->m_controlIDGen.ReleaseID( id );
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

void UIControl::niBubblingEvent( UIEvent* e )
{
	UIControl* cc = this;
	while( cc )
	{
		if( !cc->niEvent( e ) )
			break;
		cc = cc->parent;
	}
}

void UIControl::niRender()
{
	if( !visible )
		return;
	
	if( renderfunc.not_null() )
	{
		int orig = sgs_StackSize( C );
		sgs_PushVar( C, Handle( m_sgsObject, C ) );
		renderfunc.push( C );
		sgs_ThisCall( C, 0, 0 );
		sgs_SetStackSize( C, orig );
	}
	
	if( frame->pushScissorRect( rx0 + nc_left, ry0 + nc_top, rx1 - nc_right, ry1 - nc_bottom ) )
	{
		for( HandleArray::iterator it = m_sorted.begin(), itend = m_sorted.end(); it != itend; ++it )
		{
			if( !(*it)->nonclient )
				(*it)->niRender();
		}
		frame->popScissorRect();
	}
	
	for( HandleArray::iterator it = m_sorted.begin(), itend = m_sorted.end(); it != itend; ++it )
	{
		if( (*it)->nonclient )
			(*it)->niRender();
	}
}

void UIControl::updateLayout()
{
	if( _updatingLayout )
		return;
	
	_updatingLayout = true;
	UIEvent e;
	e.type = EV_Layout;
	niEvent( &e );
	_updatingLayout = false;
}

bool UIControl::addChild( UIControl::Handle ch )
{
	if( !ch.object )
	{
		sgs_Printf( C, SGS_WARNING, "cannot add null" );
		return false;
	}
	
	if( ch->frame != frame )
	{
		sgs_Printf( C, SGS_WARNING, "frames don't match" );
		return false;
	}
	
	if( ch->parent.object )
	{
		if( !ch->parent->removeChild( ch ) )
		{
			sgs_Printf( C, SGS_WARNING, "failed to remove child from previous parent" );
			return false;
		}
	}
	
	for( HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
	{
		if( *it == ch )
			return false;
	}
	m_children.push_back( ch );
	m_sorted.push_back( ch );
	ch->parent = Handle( m_sgsObject, C );
	sortChildren();
	
	UIEvent e;
	e.type = EV_Attach;
	ch->niEvent( &e );
	
	e.type = EV_AddChild;
	niEvent( &e );
	
	return true;
}

bool UIControl::removeChild( UIControl::Handle ch )
{
	UIEvent e;
	e.type = EV_Detach;
	ch->niEvent( &e );
	
	e.type = EV_RemChild;
	niEvent( &e );
	
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
	if( ch->frame.object && ch->frame->m_hover == (UIControl*) ch )
	{
		ch->frame->handleMouseMove();
		ch->frame->preRemoveControl( ch );
	}
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

sgsVariable UIControl::children( bool nonclient )
{
	sgs_SetStackSize( C, 0 );
	for( HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
	{
		if( (*it)->nonclient == nonclient )
			sgs_PushHandle( C, *it );
	}
	sgs_PushArray( C, sgs_StackSize( C ) );
	return sgsVariable( C, -1 );
}


bool UIControl_chSortCmpFunc( const UIControl::Handle& h1, const UIControl::Handle& h2 )
{
	if( h1->topmost != h2->topmost ) return h1->topmost < h2->topmost;
	if( h1->index != h2->index ) return h1->index < h2->index;
	return false;
}

void UIControl::sortChildren()
{
	std::sort( m_sorted.begin(), m_sorted.end(), UIControl_chSortCmpFunc );
}

void UIControl::sortSiblings()
{
	if( parent.object )
		parent->sortChildren();
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
	ctrl->renderfunc.gcmark();
	ctrl->data.gcmark();
	ctrl->m_events.gcmark();
	return SGS_SUCCESS;
}




