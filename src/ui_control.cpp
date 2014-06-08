

#include "ui_control.h"

#include <math.h>
#include <algorithm>



inline float lerpf( float a, float b, float s )
{
	return a * (1-s) + b * s;
}

inline float round( float x )
{
	return floor( x + 0.5 );
}


int UIControl_CtrlProc( SGS_CTX )
{
	SGSFN( "UIControl_CtrlProc" );
	sgs_Method( C );
	if( !sgs_IsObject( C, 0, UIControl::_sgs_interface ) )
		return sgs_Msg( C, SGS_WARNING, "expected UIControl as argument 0 / this" );
	UIControl* ctrl = sgs_GetVar<UIControl::Handle>()( C, 0 );
	
	UIEvent* event = sgs_GetVarObj<UIEvent>()( C, 1 );
	if( !event )
		return sgs_Msg( C, SGS_WARNING, "expected UIEvent as argument 1" );
	
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
			if( ctrl->_roundedCoords )
			{
				ctrl->rx0 = round( ctrl->rx0 );
				ctrl->rx1 = round( ctrl->rx1 );
				ctrl->ry0 = round( ctrl->ry0 );
				ctrl->ry1 = round( ctrl->ry1 );
			}
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
		ctrl->callEvent( sgsString( "mousedown", C ), event );
		if( ctrl->frame.object )
			ctrl->frame->setFocus( ctrl );
		return 1;
	case EV_ButtonUp:
		ctrl->callEvent( sgsString( "mouseup", C ), event );
		if( ctrl->clicked && ctrl->frame.object && ctrl->frame->isControlUnderCursor( UIControl::CreateHandle( ctrl ) ) )
			ctrl->callEvent( sgsString( "click", C ), event );
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
	
	root = createControl( sgsString( "root", C ) );
	root->q1x = 1;
	root->q1y = 1;
	root->updateLayout();
}

UIFrame::~UIFrame()
{
	root = UIControl::Handle();
}

UIControl::Handle UIFrame::createControl( sgsString type )
{
	UIControl* ctrl = SGS_PUSHCLASS( C, UIControl, () );
	ctrl->type = type;
	ctrl->frame = Handle( m_sgsObject, C );
	ctrl->id = m_controlIDGen.GetID();
	ctrl->updateFont();
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

UIControl* UIFrame::_getControlAtPosition( float x, float y )
{
	UIEvent htev;
	htev.type = EV_HitTest;
	htev.x = x;
	htev.y = y;
	
	UIControl* ctrl = root, *atpos = NULL;
	int nonclient = -1;
	atpos = NULL;
	while( ctrl && atpos != ctrl )
	{
		atpos = ctrl;
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
	
	return atpos;
}

void UIFrame::handleMouseMove()
{
	for( int i = 0; i < Mouse_Button_Count; ++i )
		if( m_clicktargets[ i ] )
			return;
	
	// find new mouse-over item
	UIControl* prevhover = m_hover;
	m_hover = _getControlAtPosition( mouseX, mouseY );
	
	if( m_hover != prevhover )
	{
		UIEvent mev;
		mev.x = mouseX;
		mev.y = mouseY;
		if( prevhover ){ mev.type = EV_MouseLeave; prevhover->niBubblingEvent( &mev ); }
		if( m_hover ){ mev.type = EV_MouseEnter; m_hover->niBubblingEvent( &mev ); }
		forceUpdateCursor( m_hover );
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
	
	for( int i = 0; i < Mouse_Button_Count; ++i )
	{
		if( !m_clicktargets[ i ] )
			continue;
		
		bool already = false;
		for( int j = 0; j < i; ++j )
		{
			if( m_clicktargets[ j ] == m_clicktargets[ i ] )
			{
				already = true;
				break;
			}
		}
		if( already )
			continue;
		
		m_clicktargets[ i ]->niBubblingEvent( &e );
	}
	
	if( root.object )
	{
		root->callEvent( sgsString( "globalmousemove", C ), &e );
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
		root->callEvent( sgsString( down ? "globalbuttondown" : "globalbuttonup", C ), &e );
	}
}

void UIFrame::doMouseWheel( float x, float y )
{
	UIEvent e;
	e.type = EV_MouseWheel;
	e.x = x;
	e.y = y;
	
	if( m_hover )
	{
		m_hover->niBubblingEvent( &e );
	}
	
	if( root.object )
	{
		root->callEvent( sgsString( "globalmousewheel", C ), &e );
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
	
	for( size_t i = 0; i < m_animatedControls.size(); ++i )
	{
		UIControl* ctrl = m_animatedControls[ i ];
		ctrl->_advanceAnimation( t );
		if( !ctrl->m_animQueue.size() )
		{
			m_animatedControls.erase( m_animatedControls.begin() + i );
			i--;
		}
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

void UIFrame::updateTheme()
{
	root->updateThemeRecursive();
	updateLayout();
}

void UIFrame::forceUpdateCursor( UIControl* ctrl )
{
	if( cursor_func.not_null() )
	{
		if( ctrl )
			ctrl->cursor.push( C );
		else
			sgs_PushNull( C );
		if( SGS_FAILED( sgs_CallP( C, &cursor_func.var, 1, 0 ) ) )
			sgs_Pop( C, 1 );
	}
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

int UIFrame::sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	UIFrame* frame = static_cast<UIFrame*>(obj->data);
	frame->scissor_func.gcmark();
	frame->clipboard_func.gcmark();
	frame->cursor_func.gcmark();
	frame->font_func.gcmark();
	frame->theme.gcmark();
	frame->root.gcmark();
	return SGS_SUCCESS;
}

sgsHandle< UIControl > UIFrame::getHoverControl()
{
	return sgsHandle< UIControl >( m_hover ? m_hover->m_sgsObject : NULL, C );
}

sgsHandle< UIControl > UIFrame::getFocusControl()
{
	return sgsHandle< UIControl >( m_focus ? m_focus->m_sgsObject : NULL, C );
}

sgsHandle< UIControl > UIFrame::getControlUnderCursor()
{
	UIControl* ctrl = _getControlAtPosition( mouseX, mouseY );
	return sgsHandle< UIControl >( ctrl ? ctrl->m_sgsObject : NULL, C );
}

sgsHandle< UIControl > UIFrame::getControlUnderPoint( float x, float y )
{
	UIControl* ctrl = _getControlAtPosition( x, y );
	return sgsHandle< UIControl >( ctrl ? ctrl->m_sgsObject : NULL, C );
}

bool UIFrame::isControlUnderCursor( const sgsHandle< UIControl >& ctrl )
{
	return isControlUnderPoint( ctrl, mouseX, mouseY );
}

bool UIFrame::isControlUnderPoint( const sgsHandle< UIControl >& ctrl, float x, float y )
{
	UIControl* c2 = _getControlAtPosition( x, y );
	while( c2 )
	{
		if( c2 == ctrl )
			return true;
		c2 = c2->parent;
	}
	return false;
}


UIControl::UIControl() :
	id( UI_NO_ID ), x(0.0f), y(0.0f), width(0.0f), height(0.0f),
	q0x(0.0f), q0y(0.0f), q1x(0.0f), q1y(0.0f),
	scroll_x(0.0f), scroll_y(0.0f),
	nc_top(0.0f), nc_left(0.0f), nc_right(0.0f), nc_bottom(0.0f),
	visible(true), index(0), topmost(false), nonclient(false),
	minWidth(0.0f), maxWidth(FLT_MAX), minHeight(0.0f), maxHeight(FLT_MAX),
	fontSize(0.0f),
	rx0(0.0f), rx1(0.0f), ry0(0.0f), ry1(0.0f),
	_updatingLayout(false), _updatingMinMaxWH(false), _whNoAuth(false), _roundedCoords(true),
	mouseOn(false), clicked(false), keyboardFocus(false)
{
	sgs_PushCFunction( C, UIControl_CtrlProc );
	callback = sgsVariable( C, -1 );
	sgs_Pop( C, 1 );
	
	sgs_PushDict( C, 0 );
	m_events = sgsVariable( C, -1 );
	sgs_Pop( C, 1 );
	
	sgs_InitBool( &cursor.var, 1 );
	cursor.C = C;
}

UIControl::~UIControl()
{
	size_t sz = frame->m_animatedControls.size();
	for( size_t i = 0; i < sz; ++i )
	{
		if( frame->m_animatedControls[ i ] == this )
		{
			frame->m_animatedControls.erase( frame->m_animatedControls.begin() + i );
			break;
		}
	}
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
		sgs_StkIdx orig = sgs_StackSize( C );
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

void UIControl::updateTheme()
{
	UIEvent e;
	e.type = EV_ChgTheme;
	niEvent( &e );
}

void UIControl::updateThemeRecursive()
{
	updateTheme();
	
	for( UIControl::HandleArray::iterator it = m_children.begin(), itend = m_children.end(); it != itend; ++it )
		(*it)->updateTheme();
}

void UIControl::updateCursor()
{
	if( frame.object )
	{
		for( int i = 0; i < Mouse_Button_Count; ++i )
			if( frame->m_clicktargets[ i ] )
				return;
		
		if( frame->m_hover == this )
			frame->forceUpdateCursor( this );
	}
}

void UIControl::updateFont()
{
	if( frame.object && frame->font_func.not_null() )
	{
		sgs_StkIdx orig = sgs_StackSize( C );
		sgs_PushVar( C, frame );
		font.push( C );
		sgs_PushReal( C, fontSize );
		frame->font_func.push( C );
		sgs_ThisCall( C, 2, 1 );
		_cachedFont = sgsVariable( C, -1 );
		sgs_SetStackSize( C, orig );
	}
}

bool UIControl::addChild( UIControl::Handle ch )
{
	if( !ch.object )
	{
		sgs_Msg( C, SGS_WARNING, "cannot add null" );
		return false;
	}
	
	if( ch->frame != frame )
	{
		sgs_Msg( C, SGS_WARNING, "frames don't match" );
		return false;
	}
	
	if( ch->parent.object )
	{
		if( !ch->parent->removeChild( ch ) )
		{
			sgs_Msg( C, SGS_WARNING, "failed to remove child from previous parent" );
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

bool UIControl::removeAllChildren()
{
	bool removed = !!m_children.size();
	while( m_children.size() )
		removeChild( *m_children.begin() );
	return removed;
}

UIControl::Handle UIControl::findChild( sgsString name )
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

void UIControl::setAnchorMode( int mode )
{
	if( mode & Anchor_Left )
	{
		q0x = 0;
		if( mode & Anchor_Right )
			q1x = 1;
		else
			q1x = 0;
	}
	else if( mode & Anchor_Right )
	{
		q0x = 1;
		q1x = 1;
	}
	if( mode & Anchor_Top )
	{
		q0y = 0;
		if( mode & Anchor_Bottom )
			q1y = 1;
		else
			q1y = 0;
	}
	else if( mode & Anchor_Bottom )
	{
		q0y = 1;
		q1y = 1;
	}
	updateLayout();
}

void UIControl::setAnchorRect( float x0, float y0, float x1, float y1 )
{
	q0x = x0;
	q0y = y0;
	q1x = x1;
	q1y = y1;
	updateLayout();
}


bool UIControl::bindEvent( sgsString name, sgsVariable callable )
{
	// try reading the existing array
	name.push( C );
	if( SGS_FAILED( sgs_PushIndexPI( C, &m_events.var, -1, 0 ) ) )
	{
		callable.push();
		sgs_PushArray( C, 1 ); // create an array if it doesn't exist
		sgs_StoreIndexPI( C, &m_events.var, -2, 0 );
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

bool UIControl::unbindEvent( sgsString name, sgsVariable callable )
{
	// check if there's an entry for the event
	name.push( C );
	if( sgs_PushIndexPI( C, &m_events.var, -1, 0 ) )
		return false;
	// remove item from array
	callable.push();
	return !!sgs_ObjectAction( C, -2, SGS_ACT_ARRAY_RM_ONE, -1 );
}

bool UIControl::callEvent( sgsString name, UIEvent* e )
{
	sgs_StkIdx orig = sgs_StackSize( C );
	// check if there's an entry for the event
	name.push( C );
	if( sgs_PushIndexPI( C, &m_events.var, -1, 0 ) )
	{
		sgs_SetStackSize( C, orig );
		return false;
	}
	// iterate the array
	sgs_PushIterator( C, -1 );
	while( sgs_IterAdvance( C, -1 ) > 0 )
	{
		sgs_StkIdx ssz = sgs_StackSize( C );
		
		sgs_PushObjectPtr( C, m_sgsObject );
		UI_PushEvent( C, e );
		sgs_IterPushData( C, -3, 0, 1 );
		sgs_ThisCall( C, 1, 0 );
		
		sgs_SetStackSize( C, ssz );
	}
	sgs_SetStackSize( C, orig );
	return true;
}


UIControl::Handle UIControl::animate( sgsVariable state, float length, sgsVariable func, sgsVariable oncomplete )
{
	if( !frame.object )
		return Handle();
	
	// EASING FUNCTION
	// if function is null, assume "smooth"
	if( !func.not_null() )
	{
		sgs_PushString( C, "smooth" );
		func = sgsVariable( C, -1 );
		sgs_Pop( C, 1 );
	}
	// retrieve function from a list of global easing functions if it's a string
	if( func.var.type == SGS_VT_STRING )
	{
		sgs_Variable var;
		if( SGS_FAILED( sgs_PushGlobal( C, "UI_EasingFunctions" ) ) ||
			SGS_FAILED( sgs_GetIndexIPP( C, -1, &func.var, &var, 0 ) ) )
		{
			sgs_Msg( C, SGS_WARNING, "could not find easing function: %s", sgs_var_cstr( &func.var ) );
			return Handle();
		}
		func = sgsVariable( C, &var );
		sgs_Release( C, &var );
	}
	// check if that function is callable to avoid doing so later
	if( !sgs_IsCallableP( &func.var ) )
	{
		sgs_Msg( C, SGS_WARNING, "easing function is not callable" );
		return Handle();
	}
	
	// PREVIOUS STATE AND STATE FILTERING
	sgs_PushDict( C, 0 );
	sgsVariable prevState( C, -1 );
	sgs_Pop( C, 1 );
	sgs_PushDict( C, 0 );
	sgsVariable currState( C, -1 );
	sgs_Pop( C, 1 );
	sgs_PushHandle( C, UIControl::CreateHandle( this ) );
	sgsVariable me( C, -1 );
	sgs_Pop( C, 1 );
	// try to read state and on success, add data to prev/curr states
	sgs_StkIdx orig = sgs_StackSize( C );
	if( SGS_SUCCEEDED( sgs_PushIteratorP( C, &state.var ) ) )
	{
		while( sgs_IterAdvance( C, -1 ) > 0 )
		{
			sgs_StkIdx orig2 = sgs_StackSize( C );
			
			sgs_IterPushData( C, -1, 1, 1 ); /* name, tgt.value */
			sgs_ToString( C, -2 );
			if( SGS_SUCCEEDED( sgs_PushIndexPI( C, &me.var, -2, 1 ) ) )
			{
				sgs_StoreIndexPI( C, &prevState.var, -3, 0 );
				sgs_StoreIndexPI( C, &currState.var, -2, 0 );
			}
			
			sgs_SetStackSize( C, orig2 );
		}
	}
	sgs_SetStackSize( C, orig );
	
	// PUSH ANIMATION
	Animation A =
	{
		prevState,
		currState,
		0.0f,
		length,
		func,
		oncomplete
	};
	m_animQueue.push_back( A );
	
	// ENABLE ANIMATION IN FRAME
	size_t i = 0, sz = frame->m_animatedControls.size();
	for( ; i < sz; ++i )
	{
		if( frame->m_animatedControls[ i ] == this )
			break;
	}
	if( i >= sz )
	{
		frame->m_animatedControls.push_back( this );
	}
	
	return Handle( m_sgsObject, C );
}

UIControl::Handle UIControl::stop( bool nofinish )
{
	if( !nofinish && m_animQueue.size() && m_animQueue[0].time > 0 )
		_finishCurAnim();
	
	m_animQueue.clear();
	return Handle( m_sgsObject, C );
}

UIControl::Handle UIControl::dequeue()
{
	if( m_animQueue.size() && m_animQueue[0].time > 0 )
		m_animQueue.erase( m_animQueue.begin() + 1, m_animQueue.end() );
	else
		m_animQueue.clear();
	return Handle( m_sgsObject, C );
}

UIControl::Handle UIControl::skip( bool nofinish )
{
	if( !nofinish && m_animQueue.size() && m_animQueue[0].time > 0 )
		_finishCurAnim();
	
	return Handle( m_sgsObject, C );
}

void UIControl::_advanceAnimation( float dt )
{
	while( dt > 0 && m_animQueue.size() )
	{
		Animation& A = m_animQueue[0];
		
		A.time += dt;
		dt = A.time - A.end;
		if( dt >= 0 )
		{
			// Finished
			A.time = A.end;
			_finishCurAnim();
		}
		else
			_applyCurAnimState();
	}
}

void UIControl::_applyCurAnimState()
{
	Animation& A = m_animQueue[0];
	
	float q = A.time;
	if( A.end > 0.001f )
		q /= A.end;
	
	sgs_StkIdx orig = sgs_StackSize( C );
	if( SGS_SUCCEEDED( sgs_PushIteratorP( C, &A.currState.var ) ) )
	{
		while( sgs_IterAdvance( C, -1 ) > 0 )
		{
			sgs_StkIdx orig2 = sgs_StackSize( C );
			
			sgs_IterPushData( C, -1, 1, 1 ); /* name, cur.value */
			sgs_PushIndexPI( C, &A.prevState.var, -2, 1 ); /* prev.value */
			
			/* if previous is string, apply current only at the end */
			if( sgs_ItemType( C, -1 ) == SGS_VT_STRING )
			{
				if( A.time >= A.end )
					sgs_Pop( C, 1 ); /* current = -1 */
				/* otherwise, previous = -1 */
			}
			else
			{
				/* prev. value already at -1 */
				sgs_PushItem( C, -2 );
				sgs_PushReal( C, q );
				sgs_PushReal( C, A.end ); /* time = A.end * q */
				sgs_CallP( C, &A.func.var, 4, 1 );
				/* interpolated = -1 */
				sgs_ToReal( C, -1 );
			}
			
			sgs_PushHandle( C, UIControl::CreateHandle( this ) );
			sgs_SetIndexIII( C, -1, orig2, -2, 1 );
			
			sgs_SetStackSize( C, orig2 );
		}
	}
	sgs_SetStackSize( C, orig );
}

void UIControl::_finishCurAnim()
{
	_applyCurAnimState();
	
	Animation& A = m_animQueue[0];
	if( A.oncomplete.not_null() )
	{
		sgs_StkIdx orig = sgs_StackSize( C );
		sgs_PushHandle( C, UIControl::CreateHandle( this ) );
		A.oncomplete.push( C );
		sgs_ThisCall( C, 0, 0 );
		sgs_SetStackSize( C, orig );
	}
	
	m_animQueue.erase( m_animQueue.begin() );
}


int UIControl::sgs_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop )
{
	UIControl* ctrl = static_cast<UIControl*>(obj->data);
	if( sgs_PushIndexPP( C, &ctrl->_interface.var, key, isprop ) == SGS_SUCCESS )
		return SGS_SUCCESS;
	return UIControl::_sgs_getindex( C, obj, key, isprop );
}

int UIControl::sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	UIControl* ctrl = static_cast<UIControl*>(obj->data);
	for( HandleArray::iterator it = ctrl->m_children.begin(), itend = ctrl->m_children.end(); it != itend; ++it )
		it->gcmark();
	/* m_sorted = m_children */
	ctrl->parent.gcmark();
	ctrl->frame.gcmark();
	ctrl->cursor.gcmark();
	ctrl->_cachedFont.gcmark();
	ctrl->callback.gcmark();
	ctrl->renderfunc.gcmark();
	ctrl->data.gcmark();
	ctrl->_interface.gcmark();
	ctrl->m_events.gcmark();
	for( AnimArray::iterator it = ctrl->m_animQueue.begin(), itend = ctrl->m_animQueue.end(); it != itend; ++it )
		it->gcmark();
	return SGS_SUCCESS;
}




