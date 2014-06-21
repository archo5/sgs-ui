

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


int UIColor::sgs_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop )
{
	UIColor* COL = (UIColor*) obj->data;
	if( key->type == SGS_VT_INT )
	{
		int idx = sgs_GetIntP( C, key );
		if( idx == 0 ) SGS_RETURN_REAL( COL->r );
		if( idx == 1 ) SGS_RETURN_REAL( COL->g );
		if( idx == 2 ) SGS_RETURN_REAL( COL->b );
		if( idx == 3 ) SGS_RETURN_REAL( COL->a );
		return sgs_Msg( C, SGS_WARNING, "index out of bounds" );
	}
	return UIColor::_sgs_getindex( C, obj, key, isprop );
}

int UIColor::sgs_expr( SGS_CTX, sgs_VarObj* obj, sgs_Variable* A, sgs_Variable* B, int type )
{
	if( type != SGS_EOP_ADD && type != SGS_EOP_SUB && type != SGS_EOP_MUL )
		return SGS_ENOTSUP;
	UIColor CA, CB;
	sgs_Real rA, rB;
	if( !( ( sgs_IsObjectP( A, _sgs_interface ) && ( CA = *(UIColor*) sgs_GetObjectDataP( A ) ) )
		|| ( sgs_ParseRealP( C, A, &rA ) && ( CA = UIColor( rA, rA, rA, rA ) ) ) ) ) return SGS_EINVAL;
	if( !( ( sgs_IsObjectP( B, _sgs_interface ) && ( CB = *(UIColor*) sgs_GetObjectDataP( B ) ) )
		|| ( sgs_ParseRealP( C, B, &rB ) && ( CB = UIColor( rB, rB, rB, rB ) ) ) ) ) return SGS_EINVAL;
	if( type == SGS_EOP_ADD ) sgs_PushVar( C, CA + CB );
	if( type == SGS_EOP_SUB ) sgs_PushVar( C, CA - CB );
	if( type == SGS_EOP_MUL ) sgs_PushVar( C, CA * CB );
	return SGS_SUCCESS;
}


int UIStyle::get_anchorMode()
{
	int out = 0;
	if( q0x.isset && q0x.data == 0 )
	{
		if( q1x.isset && q1x.data == 0 ) out |= UI_Anchor_Left;
		else if( q1x.isset && q1x.data == 1 ) out |= UI_Anchor_Hor;
	}
	else if( q0x.isset && q0x.data == 1 && q1x.isset && q1x.data == 1 ) out |= UI_Anchor_Right;
	if( q0y.isset && q0y.data == 0 )
	{
		if( q1y.isset && q1y.data == 0 ) out |= UI_Anchor_Top;
		else if( q1y.isset && q1y.data == 1 ) out |= UI_Anchor_Vert;
	}
	else if( q0y.isset && q0y.data == 1 && q1y.isset && q1y.data == 1 ) out |= UI_Anchor_Bottom;
	return out;
}

void UIStyle::set_anchorMode( int mode )
{
	if( mode & UI_Anchor_Left )
	{
		q0x.set( 0 );
		if( mode & UI_Anchor_Right )
			q1x.set( 1 );
		else
			q1x.set( 0 );
	}
	else if( mode & UI_Anchor_Right )
	{
		q0x.set( 1 );
		q1x.set( 1 );
	}
	if( mode & UI_Anchor_Top )
	{
		q0y.set( 0 );
		if( mode & UI_Anchor_Bottom )
			q1y.set( 1 );
		else
			q1y.set( 0 );
	}
	else if( mode & UI_Anchor_Bottom )
	{
		q0y.set( 1 );
		q1y.set( 1 );
	}
}


void StyleArr_trimSelector( const char** str, size_t* size )
{
	const char* s1 = *str;
	const char* s2 = s1 + *size;
	while( s1 < s2 && ( *s1 == ' ' || *s1 == '\t' || *s1 == '\n' || *s1 == '\r' ) )
		s1++;
	while( s1 < s2 && ( *(s2-1) == ' ' || *(s2-1) == '\t' || *(s2-1) == '\n' || *(s2-1) == '\r' ) )
		s2--;
	*str = s1;
	*size = s2 - s1;
}

const char* StyleArr_addSelector( StyleSelArray& selectors, sgsString sgsstr, const char* str, size_t size )
{
	StyleArr_trimSelector( &str, &size );
	selectors.resize( selectors.size() + 1 );
	UIStyleSelector* sel = &selectors[ selectors.size() - 1 ];
	const char* serr = UI_ParseSelector( sel, sgsstr, str, size );
	if( serr )
	{
		selectors.resize( selectors.size() - 1 );
		return serr;
	}
	
	size_t selpos = selectors.size() - 1;
	while( selpos > 0 && UI_CompareSelectors( &selectors[ selpos - 1 ], &selectors[ selpos ] ) < 0 )
	{
		VQSWAP( selectors, selpos - 1, selpos );
		selpos--;
	}
	return NULL;
}

const char* StyleArr_addSelectors( StyleSelArray& selectors, sgsString sgsstr )
{
	const char* pp = sgsstr.c_str();
	const char* ppend = pp + sgsstr.size();
	const char* ppcur = pp;
	while( pp <= ppend )
	{
		if( pp == ppend || *pp == ',' )
		{
			const char* serr = StyleArr_addSelector( selectors, sgsstr, ppcur, pp - ppcur );
			if( serr )
				return serr;
			ppcur = ++pp;
			continue;
		}
		pp++;
	}
	return NULL;
}


SGS_MULTRET UIStyleRule::_handleSGS( const char* serr, const char* base )
{
	if( serr )
	{
		sgs_PushBool( C, 0 );
		sgs_PushInt( C, serr - base );
		return 2;
	}
	SGS_RETURN_THIS( C );
}

SGS_MULTRET UIStyleRule::addSelector( sgsString sgsstr )
{
	return _handleSGS( StyleArr_addSelector( selectors, sgsstr, sgsstr.c_str(), sgsstr.size() ), sgsstr.c_str() );
}

SGS_MULTRET UIStyleRule::addSelectors( sgsString sgsstr )
{
	return _handleSGS( StyleArr_addSelectors( selectors, sgsstr ), sgsstr.c_str() );
}

bool UIStyleRule::checkMatch( sgsHandle< UIControl > ctrl )
{
	for( size_t i = 0; i < selectors.size(); ++i )
		if( UI_SelectorTestControl( &selectors[ i ], ctrl ) )
			return true;
	return false;
}

int UIStyleSheet::sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	UIStyleSheet* sheet = (UIStyleSheet*) obj->data;
	for( size_t i = 0; i < sheet->rules.size(); ++i )
		sheet->rules[ i ].gcmark();
	return SGS_SUCCESS;
}

SGS_MULTRET UIStyleSheet::addRule( UIStyleRule::Handle rule )
{
	if( rule.object && VFIND( rules, rule ) >= rules.size() )
		rules.push_back( rule );
	SGS_RETURN_THIS(C);
}

void UIStyleSheet::build( sgsVariable var )
{
	// key = selectors, value = iterable{ key/value => UIStyle }
	sgs_StkIdx orig = sgs_StackSize( C );
	if( SGS_FAILED( sgs_PushIteratorP( C, &var.var ) ) )
		goto iter_fail;
	while( sgs_IterAdvance( C, -1 ) > 0 )
	{
		sgs_StkIdx orig2 = sgs_StackSize( C );
		UIStyleRule* rule = SGS_PUSHCLASS( C, UIStyleRule, () );
		
		if( SGS_FAILED( sgs_IterPushData( C, -2, 1, 1 ) ) )
			goto iter_fail;
		if( !sgs_ToString( C, -2 ) )
			goto key_fail;
		
		// try to add selector
		const char* errpos = StyleArr_addSelectors( rule->selectors, sgsString( C, orig2 + 1 ) );
		if( errpos )
		{
			sgs_Msg( C, SGS_WARNING, "failed to parse selectors: '%.*s' (position %d)",
				sgs_GetStringSize( C, orig2 + 1 ), sgs_GetStringPtr( C, orig2 + 1 ),
				(int) ( errpos - sgs_GetStringPtr( C, orig2 + 1 ) ) );
			goto end;
		}
		
		// iterate style data
		if( SGS_FAILED( sgs_PushIterator( C, -1 ) ) )
			goto iter2_fail;
		while( sgs_IterAdvance( C, -1 ) > 0 )
		{
			sgs_StkIdx orig3 = sgs_StackSize( C );
			
			if( SGS_FAILED( sgs_IterPushData( C, -1, 1, 1 ) ) )
				goto iter2_fail;
			if( !sgs_ToString( C, -2 ) )
				goto key_fail;
			
			// set style value
			sgsVariable key( C, -2 );
			sgsVariable val( C, -1 );
			sgs_VarObj fakeobj; // HACK
			fakeobj.data = &rule->style;
			int ret = UIStyle::_sgs_interface->setindex( C, &fakeobj, &key.var, &val.var, 0 );
			if( ret == SGS_ENOTFND )
			{
				sgs_Msg( C, SGS_WARNING, "property not found: '%.*s'", sgs_GetStringSize( C, orig3 ), sgs_GetStringPtr( C, orig3 ) );
				goto end;
			}
			if( SGS_FAILED( ret ) )
				goto int_fail;
			
			sgs_SetStackSize( C, orig3 );
		}
		
		UIStyleRule::Handle rulehandle = UIStyleRule::Handle( C, rule->m_sgsObject );
		if( rulehandle.object && VFIND( rules, rulehandle ) >= rules.size() )
			rules.push_back( rulehandle );
		
		sgs_SetStackSize( C, orig2 );
	}
end:
	sgs_SetStackSize( C, orig );
	return;
iter_fail:
	sgs_Msg( C, SGS_WARNING, "passed variable is not iterable" );
	goto end;
iter2_fail:
	sgs_Msg( C, SGS_WARNING, "passed variable's style data is not iterable" );
	goto end;
key_fail:
	sgs_Msg( C, SGS_WARNING, "key cannot be converted to a string" );
	goto end;
int_fail:
	sgs_Msg( C, SGS_WARNING, "internal UIStyle interface error" );
	goto end;
}

#define UIS_FRAG UIStyleSelector::Fragment
static inline bool _sft_isanymove( int type ){ return type == UIS_FRAG::T_MoveOn || type == UIS_FRAG::T_ReqNext; }
static inline bool _sct_isanyspec( int type )
{
	return
	type == '.' ||
	type == '@' ||
	type == '>' ||
	type == '*' ||
	type == ':' ||
	type == ' ' ||
	type == '\n' ||
	type == '\r' ||
	type == '\t';
}

const char* UI_ParseSelector( UIStyleSelector* sel, sgsString sgsstr, const char* str, size_t size )
{
	sel->selector = sgsstr;
	sel->fragments.clear();
	sel->numnext = 0;
	sel->numtypes = 0;
	sel->numclasses = 0;
	sel->numnames = 0;
	
	const char* text = str;
	const char* textend = text + size;
	while( text < textend )
	{
		char c = *text;
		if( c == '>' )
		{
			if( !sel->fragments.size() )
				return text; // error: this item cannot be first in matcher list
			char lasttype = VLASTOF( sel->fragments ).type;
			if( lasttype == UIS_FRAG::T_ReqNext )
				return text; // error: this item cannot follow another item of same type
			else if( lasttype == UIS_FRAG::T_MoveOn )
				VLASTOF( sel->fragments ).type = UIS_FRAG::T_ReqNext;
			else
			{
				UIS_FRAG F = { UIS_FRAG::T_ReqNext, NULL, NULL };
				sel->fragments.push_back( F );
			}
			sel->numnext++;
		}
		else if( c == ' ' || c == '\n' || c == '\r' || c == '\t' )
		{
			if( sel->fragments.size() && !_sft_isanymove( VLASTOF( sel->fragments ).type ) )
			{
				UIS_FRAG F = { UIS_FRAG::T_MoveOn, NULL, NULL };
				sel->fragments.push_back( F );
			}
		}
		else if( c == '*' )
		{
			UIS_FRAG F = { UIS_FRAG::T_MatchAny, NULL, NULL };
			sel->fragments.push_back( F );
			sel->numtypes++;
		}
		else if( c == '.' || c == '@' )
		{
			text++;
			if( text >= textend ) return text; // unexpected end of string
			
			char spc = 0;
			if( *text == '^' || *text == '$' || *text == '~' )
			{
				spc = *text++;
				if( text >= textend ) return text; // unexpected end of string
			}
			
			const char* start = text;
			while( text < textend && !_sct_isanyspec( *text ) )
				text++;
			const char* end = text--;
			if( start == end )
				return text; // unexpected end of matcher
			
			UIS_FRAG::Type type;
			switch( spc )
			{
			case '^': type = c == '.' ? UIS_FRAG::T_MatchClassPartBegin : UIS_FRAG::T_MatchNamePartBegin; break;
			case '$': type = c == '.' ? UIS_FRAG::T_MatchClassPartEnd : UIS_FRAG::T_MatchNamePartEnd; break;
			case '~': type = c == '.' ? UIS_FRAG::T_MatchClassPart : UIS_FRAG::T_MatchNamePart; break;
			default: type = c == '.' ? UIS_FRAG::T_MatchClassExact : UIS_FRAG::T_MatchNameExact; break;
			}
			UIS_FRAG F = { type, start, end };
			sel->fragments.push_back( F );
			if( c == '.' )
				sel->numclasses++;
			else
				sel->numnames++;
		}
		else if( c == ':' ) // state matcher
		{
			text++;
			if( text >= textend ) return text; // unexpected end of string
			
			const char* start = text;
			while( text < textend && !_sct_isanyspec( *text ) )
				text++;
			const char* end = text--;
			if( start == end )
				return text; // unexpected end of matcher
			
			UIS_FRAG::Type type;
			if( UI_TxMatchExact( start, end, "hover", 5 ) ) type = UIS_FRAG::T_MatchStateHover;
			else if( UI_TxMatchExact( start, end, "active", 6 ) ) type = UIS_FRAG::T_MatchStateActive;
			else if( UI_TxMatchExact( start, end, "focus", 5 ) ) type = UIS_FRAG::T_MatchStateFocus;
			else if( UI_TxMatchExact( start, end, "first-child", 11 ) ) type = UIS_FRAG::T_MatchPosFirstChild;
			else if( UI_TxMatchExact( start, end, "last-child", 10 ) ) type = UIS_FRAG::T_MatchPosLastChild;
			else return start; // wrong matcher type
			
			UIS_FRAG F = { type, start, end };
			sel->fragments.push_back( F );
			sel->numclasses++; // technically it's not really a class but in terms of styling should behave like one
		}
		else // type matcher
		{
			const char* start = text;
			while( text < textend && !_sct_isanyspec( *text ) )
				text++;
			const char* end = text--;
			if( start == end )
				return text; // unexpected end of matcher (should never be hit)
			UIS_FRAG F = { UIS_FRAG::T_MatchType, start, end };
			sel->fragments.push_back( F );
			sel->numtypes++;
		}
		text++;
	}
	
	return NULL;
}

bool UI_SelectorTestControl( const UIStyleSelector* sel, UIControl* ctrl )
{
	int curfrag = sel->fragments.size() - 1;
	int begfrag = curfrag;
	bool moveonfail = false;
	while( curfrag >= 0 && ctrl )
	{
		const UIS_FRAG& F = sel->fragments[ curfrag ];
		// separators
		if( F.type == UIS_FRAG::T_MoveOn )
		{
			moveonfail = true;
			begfrag = --curfrag;
			ctrl = ctrl->parent;
			continue;
		}
		else if( F.type == UIS_FRAG::T_ReqNext )
		{
			moveonfail = false;
			begfrag = --curfrag;
			ctrl = ctrl->parent;
			continue;
		}
		// matching
		else if( F.type == UIS_FRAG::T_MatchAny )
		{
			// always succeeds
		}
		// - TYPE
		else if( F.type == UIS_FRAG::T_MatchType )
		{
			if( !UI_TxMatchExact( F.start, F.end, ctrl->type.c_str(), ctrl->type.size() ) )
				goto matchfail;
		}
		// - CLASS
		else if( F.type == UIS_FRAG::T_MatchClassExact )
		{
			if( !ctrl->hasClass( F.start, F.end - F.start ) )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchClassPartBegin )
		{
			if( !ctrl->hasClassPartBegin( F.start, F.end - F.start ) )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchClassPartEnd )
		{
			if( !ctrl->hasClassPartEnd( F.start, F.end - F.start ) )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchClassPart )
		{
			if( !ctrl->hasClassPart( F.start, F.end - F.start ) )
				goto matchfail;
		}
		// - NAME
		else if( F.type == UIS_FRAG::T_MatchNameExact )
		{
			if( !UI_TxMatchExact( F.start, F.end, ctrl->name.c_str(), ctrl->name.size() ) )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchNamePartBegin )
		{
			if( !UI_TxMatchPartBegin( F.start, F.end, ctrl->name.c_str(), ctrl->name.size() ) )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchNamePartEnd )
		{
			if( !UI_TxMatchPartEnd( F.start, F.end, ctrl->name.c_str(), ctrl->name.size() ) )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchNamePart )
		{
			if( !UI_TxMatchPart( F.start, F.end, ctrl->name.c_str(), ctrl->name.size() ) )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchStateHover )
		{
			if( !ctrl->mouseOn )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchStateActive )
		{
			if( !ctrl->clicked )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchStateFocus )
		{
			if( !ctrl->keyboardFocus )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchPosFirstChild )
		{
			if( !ctrl->parent.object || ctrl->parent->m_children[0] != ctrl )
				goto matchfail;
		}
		else if( F.type == UIS_FRAG::T_MatchPosLastChild )
		{
			if( !ctrl->parent.object || VLASTOF( ctrl->parent->m_children ) != ctrl )
				goto matchfail;
		}
		curfrag--;
		continue;
		
	matchfail:
		if( moveonfail )
		{
			curfrag = begfrag;
			ctrl = ctrl->parent;
		}
		else
			return false;
	}
	// success is when we didn't run out of controls and matched all fragments
	return ctrl && curfrag < 0;
}

int UI_CompareSelectors( const UIStyleSelector* sel1, const UIStyleSelector* sel2 )
{
	int q1 = sel1->numtypes + sel1->numnext * 2 + sel1->numclasses * 3 + sel1->numnames * 4;
	int q2 = sel2->numtypes + sel2->numnext * 2 + sel2->numclasses * 3 + sel2->numnames * 4;
	return q1 - q2;
}

void UI_StyleMerge( UIStyle* style, UIStyle* add )
{
	if( !style->x.isset ) style->x = add->x;
	if( !style->y.isset ) style->y = add->y;
	if( !style->width.isset ) style->width = add->width;
	if( !style->height.isset ) style->height = add->height;
	if( !style->q0x.isset ) style->q0x = add->q0x;
	if( !style->q0y.isset ) style->q0y = add->q0y;
	if( !style->q1x.isset ) style->q1x = add->q1x;
	if( !style->q1y.isset ) style->q1y = add->q1y;
	if( !style->nonClientTop.isset ) style->nonClientTop = add->nonClientTop;
	if( !style->nonClientLeft.isset ) style->nonClientLeft = add->nonClientLeft;
	if( !style->nonClientRight.isset ) style->nonClientRight = add->nonClientRight;
	if( !style->nonClientBottom.isset ) style->nonClientBottom = add->nonClientBottom;
	if( !style->visible.isset ) style->visible = add->visible;
	if( !style->index.isset ) style->index = add->index;
	if( !style->topmost.isset ) style->topmost = add->topmost;
	if( !style->minWidth.isset ) style->minWidth = add->minWidth;
	if( !style->maxWidth.isset ) style->maxWidth = add->maxWidth;
	if( !style->minHeight.isset ) style->minHeight = add->minHeight;
	if( !style->maxHeight.isset ) style->maxHeight = add->maxHeight;
	if( !style->marginLeft.isset ) style->marginLeft = add->marginLeft;
	if( !style->marginRight.isset ) style->marginRight = add->marginRight;
	if( !style->marginTop.isset ) style->marginTop = add->marginTop;
	if( !style->marginBottom.isset ) style->marginBottom = add->marginBottom;
	if( !style->paddingLeft.isset ) style->paddingLeft = add->paddingLeft;
	if( !style->paddingRight.isset ) style->paddingRight = add->paddingRight;
	if( !style->paddingTop.isset ) style->paddingTop = add->paddingTop;
	if( !style->paddingBottom.isset ) style->paddingBottom = add->paddingBottom;
	if( !style->backgroundColor.isset ) style->backgroundColor = add->backgroundColor;
	if( !style->overflow.isset ) style->overflow = add->overflow;
	if( !style->textColor.isset ) style->textColor = add->textColor;
	if( !style->cursor.not_null() ) style->cursor = add->cursor;
	if( !style->font.c_str() ) style->font = add->font;
	if( !style->fontSize.isset ) style->fontSize = add->fontSize;
	if( !style->renderfunc.not_null() ) style->renderfunc = add->renderfunc;
}

void UI_ToStyleCache( UIStyleCache* cache, UIStyle* style )
{
	cache->x = style->x.isset ? style->x.data : 0;
	cache->y = style->y.isset ? style->y.data : 0;
	cache->width = style->width.isset ? style->width.data : 0;
	cache->height = style->height.isset ? style->height.data : 0;
	cache->q0x = style->q0x.isset ? style->q0x.data : 0;
	cache->q0y = style->q0y.isset ? style->q0y.data : 0;
	cache->q1x = style->q1x.isset ? style->q1x.data : 0;
	cache->q1y = style->q1y.isset ? style->q1y.data : 0;
	cache->nonClientTop = style->nonClientTop.isset ? style->nonClientTop.data : 0;
	cache->nonClientLeft = style->nonClientLeft.isset ? style->nonClientLeft.data : 0;
	cache->nonClientRight = style->nonClientRight.isset ? style->nonClientRight.data : 0;
	cache->nonClientBottom = style->nonClientBottom.isset ? style->nonClientBottom.data : 0;
	cache->visible = style->visible.isset ? style->visible.data : true;
	cache->index = style->index.isset ? style->index.data : 0;
	cache->topmost = style->topmost.isset ? style->topmost.data : false;
	cache->minWidth = style->minWidth.isset ? style->minWidth.data : 0;
	cache->maxWidth = style->maxWidth.isset ? style->maxWidth.data : UI_MaxValue;
	cache->minHeight = style->minHeight.isset ? style->minHeight.data : 0;
	cache->maxHeight = style->maxHeight.isset ? style->maxHeight.data : UI_MaxValue;
	cache->marginLeft = style->marginLeft.isset ? style->marginLeft.data : 0;
	cache->marginRight = style->marginRight.isset ? style->marginRight.data : 0;
	cache->marginTop = style->marginTop.isset ? style->marginTop.data : 0;
	cache->marginBottom = style->marginBottom.isset ? style->marginBottom.data : 0;
	cache->paddingLeft = style->paddingLeft.isset ? style->paddingLeft.data : 0;
	cache->paddingRight = style->paddingRight.isset ? style->paddingRight.data : 0;
	cache->paddingTop = style->paddingTop.isset ? style->paddingTop.data : 0;
	cache->paddingBottom = style->paddingBottom.isset ? style->paddingBottom.data : 0;
	cache->overflow = style->overflow.isset ? style->overflow.data : false;
	cache->backgroundColor = style->backgroundColor.isset ? style->backgroundColor.data : UIColor();
	cache->textColor = style->textColor.isset ? style->textColor.data : UIColor(0,0,0,1);
	if( style->cursor.not_null() )
		cache->cursor = style->cursor;
	else
	{
		cache->cursor._release();
		sgs_InitBool( &cache->cursor.var, 1 );
	}
	cache->font = style->font;
	cache->fontSize = style->fontSize.isset ? style->fontSize.data : 11;
	cache->renderfunc = style->renderfunc;
}


bool UI_TxMatchExact( const char* stfrom, const char* stto, const char* tgt, size_t tgtsize )
{
	return stto - stfrom == (ptrdiff_t) tgtsize && memcmp( stfrom, tgt, stto - stfrom ) == 0;
}

bool UI_TxMatchPartBegin( const char* stfrom, const char* stto, const char* tgt, size_t tgtsize )
{
	return stto - stfrom <= (ptrdiff_t) tgtsize && memcmp( stfrom, tgt, stto - stfrom ) == 0;
}

bool UI_TxMatchPartEnd( const char* stfrom, const char* stto, const char* tgt, size_t tgtsize )
{
	return stto - stfrom <= (ptrdiff_t) tgtsize && memcmp( stfrom, tgt + tgtsize - ( stto - stfrom ), stto - stfrom ) == 0;
}

bool UI_TxMatchPart( const char* stfrom, const char* stto, const char* tgt, size_t tgtsize )
{
	if( stto - stfrom > (ptrdiff_t) tgtsize )
		return false;
	const char* tgtend = tgt + tgtsize - ( stto - stfrom );
	while( tgt < tgtend )
		if( memcmp( stfrom, tgt++, stto - stfrom ) == 0 )
			return true;
	return false;
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
				pr0x = prt->rx0 + prt->get_nonClientLeft();
				pr0y = prt->ry0 + prt->get_nonClientTop();
				pr1x = prt->rx1 - prt->get_nonClientRight();
				pr1y = prt->ry1 - prt->get_nonClientBottom();
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
			ctrl->rx0 = ctrl->get_x();
			ctrl->ry0 = ctrl->get_y();
			ctrl->rx1 = ctrl->rx0 + ctrl->get_width();
			ctrl->ry1 = ctrl->ry0 + ctrl->get_height();
			ctrl->rx0 += lerpf( pr0x, pr1x, ctrl->get_q0x() );
			ctrl->rx1 += lerpf( pr0x, pr1x, ctrl->get_q1x() );
			ctrl->ry0 += lerpf( pr0y, pr1y, ctrl->get_q0y() );
			ctrl->ry1 += lerpf( pr0y, pr1y, ctrl->get_q1y() );
			ctrl->_changedFullRect();
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
			if( event->x >= ctrl->rx0 + ctrl->get_nonClientLeft() && event->x <= ctrl->rx1 - ctrl->get_nonClientRight() &&
				event->y >= ctrl->ry0 + ctrl->get_nonClientTop() && event->y <= ctrl->ry1 - ctrl->get_nonClientBottom() )
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
		ctrl->frame->_updateStyles( ctrl );
		return 1;
	case EV_MouseLeave:
		ctrl->mouseOn = false;
		ctrl->frame->_updateStyles( ctrl );
		return 1;
	case EV_ButtonDown:
		ctrl->clicked += ctrl->mouseOn;
		ctrl->frame->_updateStyles( ctrl );
		ctrl->_callEvent( sgsString( C, "mousedown" ), event );
		if( ctrl->frame.object )
			ctrl->frame->setFocus( ctrl );
		return 1;
	case EV_ButtonUp:
		ctrl->_callEvent( sgsString( C, "mouseup" ), event );
		if( ctrl->clicked && ctrl->frame.object && ctrl->frame->isControlUnderCursor( UIControl::Handle( ctrl ) ) )
			ctrl->_callEvent( sgsString( C, "click" ), event );
		ctrl->clicked--;
		ctrl->frame->_updateStyles( ctrl );
		if( ctrl->clicked < 0 )
			ctrl->clicked = 0;
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



UIFrame::UIFrame() : x(0), y(0), width(9999), height(9999), mouseX(0), mouseY(0),
	m_hover(NULL), m_focus(NULL), m_timerAutoID(1)
{
	memset( m_clicktargets, 0, sizeof(m_clicktargets) );
	memset( m_clickoffsets, 0, sizeof(m_clickoffsets) );
	
	root = createControl( sgsString( C, "root" ) );
	root->style.q1x.set( 1 );
	root->style.q1y.set( 1 );
	root->_remergeStyle();
}

UIFrame::~UIFrame()
{
	root = UIControl::Handle();
}

UIControl::Handle UIFrame::createControl( sgsString type )
{
	UIControl* ctrl = SGS_PUSHCLASS( C, UIControl, () );
	ctrl->type = type;
	ctrl->frame = Handle( C, m_sgsObject );
	ctrl->id = m_controlIDGen.GetID();
	ctrl->updateFont();
	UIFilteredStyleArray fsa;
	ctrl->_refilterStyles( fsa );
	UIControl::Handle handle = sgs_GetVar< UIControl::Handle >()( C, -1 );
	sgs_Pop( C, 1 );
	return handle;
}

sgsHandle<struct UIQuery> UIFrame::find( /* args */ )
{
	return UIQuery::Create( Handle( C, m_sgsObject ) );
}


void UIFrame::event( UIEvent* e )
{
	if( root )
		root->niEvent( e, true );
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
			if( !nc->get_visible() )
				continue;
			if( nonclient < 0 || nonclient == ( nc->nonclient ? 1 : 0 ) )
			{
				int hit = nc->niEvent( &htev, true );
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

void UIFrame::handleMouseMove( bool optional )
{
	if( optional )
	{
		for( int i = 0; i < Mouse_Button_Count; ++i )
			if( m_clicktargets[ i ] )
				return;
	}
	
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
		
		// for cursor stability
		for( int i = 0; i < Mouse_Button_Count; ++i )
			if( m_clicktargets[ i ] )
				return;
		forceUpdateCursor( m_hover );
	}
}

void UIFrame::setFocus( UIControl* ctrl )
{
	if( m_focus == ctrl )
		return;
	
	UIEvent e;
	e.type = EV_NeedFocus;
	
	if( ctrl->niEvent( &e, true ) )
	{
		if( m_focus ){ e.type = EV_FocusLeave; m_focus->niEvent( &e, true ); }
		m_focus = ctrl;
		if( m_focus ){ e.type = EV_FocusEnter; m_focus->niEvent( &e, true ); }
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
	
	handleMouseMove( true );
	
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
		root->_callEvent( sgsString( C, "globalmousemove" ), &e );
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
		handleMouseMove( true );
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
		root->_callEvent( sgsString( C, down ? "globalbuttondown" : "globalbuttonup" ), &e );
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
		root->_callEvent( sgsString( C, "globalmousewheel" ), &e );
	}
}

void UIFrame::doKeyPress( int key, bool down )
{
	UIEvent e;
	e.type = down ? EV_KeyDown : EV_KeyUp;
	e.key = key;
	
	if( m_focus )
		m_focus->niEvent( &e, true );
}

void UIFrame::doPutChar( int chr )
{
	UIEvent e;
	e.type = EV_Char;
	e.uchar = chr;
	
	if( m_focus )
		m_focus->niEvent( &e, true );
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


static UIStyleSheet::Handle _theme_getstylesheet( SGS_CTX, sgsVariable theme )
{
	if( theme.not_null() )
	{
		sgs_String32 str32;
		sgs_Variable key, val;
		sgs_InitString32( &key, &str32, "stylesheet" );
		
		if( SGS_SUCCEEDED( sgs_GetIndexPPP( C, &theme.var, &key, &val, 0 ) ) )
		{
			sgs_CheckString32( &str32 );
			if( val.type == SGS_VT_OBJECT )
				return UIStyleSheet::Handle( C, val.data.O );
		}
		sgs_CheckString32( &str32 );
	}
	
	return UIStyleSheet::Handle();
}

void UIFrame::setTheme( sgsVariable newtheme )
{
	UIStyleSheet::Handle ssh = _theme_getstylesheet( C, theme );
	if( ssh.object )
		removeStyleSheet( ssh );
		
	theme = newtheme;
	
	ssh = _theme_getstylesheet( C, newtheme );
	if( ssh.object )
		addStyleSheet( ssh );
	
	updateTheme();
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
			ctrl->get_cursor().push( C );
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
	for( size_t i = 0; i < frame->m_styleSheets.size(); ++i )
		frame->m_styleSheets[ i ].gcmark();
	return SGS_SUCCESS;
}


void UIFrame::addStyleSheet( UIStyleSheet::Handle sheet )
{
	if( !sheet.object )
		return;
	if( VFIND( m_styleSheets, sheet ) >= m_styleSheets.size() )
		m_styleSheets.push_back( sheet );
	_updateStyles( root );
}

void UIFrame::removeStyleSheet( UIStyleSheet::Handle sheet )
{
	if( !sheet.object )
		return;
	size_t pos = VFIND( m_styleSheets, sheet );
	if( pos < m_styleSheets.size() )
		VREMOVEAT( m_styleSheets, pos );
	_updateStyles( root );
}

sgsVariable UIFrame::getStyleSheets()
{
	for( size_t i = 0; i < m_styleSheets.size(); ++i )
		m_styleSheets[ i ].push( C );
	sgs_PushArray( C, m_styleSheets.size() );
	return sgsVariable( C, -1 );
}

void UIFrame::_updateStyles( UIControl* initial )
{
	UIFilteredStyleArray fsa;
	std::vector< UIControl* > controls;
	controls.push_back( initial );
	while( controls.size() )
	{
		UIControl* ctrl = controls[0];
		VREMOVEAT( controls, 0 );
		
		ctrl->_refilterStyles( fsa );
		
		for( size_t i = 0; i < ctrl->m_children.size(); ++i )
			controls.push_back( ctrl->m_children[ i ] );
	}
}


UIControl::Handle UIFrame::getHoverControl()
{
	return UIControl::Handle( m_hover );
}

UIControl::Handle UIFrame::getFocusControl()
{
	return UIControl::Handle( m_focus );
}

UIControl::Handle UIFrame::getControlUnderCursor()
{
	UIControl* ctrl = _getControlAtPosition( mouseX, mouseY );
	return UIControl::Handle( ctrl );
}

UIControl::Handle UIFrame::getControlUnderPoint( float x, float y )
{
	UIControl* ctrl = _getControlAtPosition( x, y );
	return UIControl::Handle( ctrl );
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
	id( UI_NO_ID ), scroll_x(0.0f), scroll_y(0.0f), nonclient(false),
	rx0(0.0f), rx1(0.0f), ry0(0.0f), ry1(0.0f),
	_updatingLayout(false), _roundedCoords(true),
	mouseOn(false), clicked(0), keyboardFocus(false)
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

int UIControl::niEvent( UIEvent* e, bool only )
{
	int orig = sgs_StackSize( C );
	sgs_PushVar( C, Handle( C, m_sgsObject ) );
	if( only )
		e->target = Handle( C, m_sgsObject );
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
	e->target = Handle( C, m_sgsObject );
	while( cc )
	{
		if( !cc->niEvent( e ) )
			break;
		cc = cc->parent;
	}
}

void UIControl::niRender()
{
	if( !get_visible() )
		return;
	
	if( get_renderfunc().not_null() )
	{
		sgs_StkIdx orig = sgs_StackSize( C );
		sgs_PushVar( C, Handle( C, m_sgsObject ) );
		get_renderfunc().push( C );
		sgs_ThisCall( C, 0, 0 );
		sgs_SetStackSize( C, orig );
	}
	
	bool overflow = get_overflow();
	if( overflow || frame->pushScissorRect( rx0 + get_nonClientLeft(), ry0 + get_nonClientTop(), rx1 - get_nonClientRight(), ry1 - get_nonClientBottom() ) )
	{
		for( HandleArray::iterator it = m_sorted.begin(), itend = m_sorted.end(); it != itend; ++it )
		{
			if( !(*it)->nonclient )
				(*it)->niRender();
		}
		if( !overflow )
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
	niEvent( &e, true );
	_updatingLayout = false;
}

void UIControl::updateTheme()
{
	UIEvent e;
	e.type = EV_ChgTheme;
	niEvent( &e, true );
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
		get_font().push( C );
		sgs_PushReal( C, get_fontSize() );
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
	ch->parent = Handle( C, m_sgsObject );
	sortChildren();
	
	frame->_updateStyles( ch );
	
	UIEvent e;
	e.type = EV_AddChild;
	niEvent( &e, true );
	
	e.type = EV_Attach;
	ch->niEvent( &e, true );
	
	puts(ch->classes.c_str());
	
	return true;
}

bool UIControl::removeChild( UIControl::Handle ch )
{
	UIEvent e;
	e.type = EV_Detach;
	ch->niEvent( &e, true );
	
	e.type = EV_RemChild;
	niEvent( &e, true );
	
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
		ch->frame->handleMouseMove( false );
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

void UIControl::detach()
{
	if( parent.object )
		parent->removeChild( Handle( this ) );
}

void UIControl::destroy( bool hard )
{
	stop( true );
	detach();
	destroyAllChildren( hard );
	if( hard )
	{
		m_events = sgsVariable();
		callback = sgsVariable();
		data = sgsVariable();
		_interface = sgsVariable();
	}
	else
		unbindEverything();
}

void UIControl::destroyAllChildren( bool hard )
{
	while( m_children.size() )
		m_children[0]->destroy( hard );
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
			it->push( C );
	}
	sgs_PushArray( C, sgs_StackSize( C ) );
	return sgsVariable( C, -1 );
}


bool UIControl_chSortCmpFunc( const UIControl::Handle& h1, const UIControl::Handle& h2 )
{
	bool h1topmost = h1->get_topmost(), h2topmost = h2->get_topmost();
	if( h1topmost != h2topmost ) return h1topmost < h2topmost;
	int h1index = h1->get_index(), h2index = h2->get_index();
	if( h1index != h2index ) return h1index < h2index;
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
	if( mode & UI_Anchor_Left )
	{
		style.q0x.set( 0 );
		if( mode & UI_Anchor_Right )
			style.q1x.set( 1 );
		else
			style.q1x.set( 0 );
	}
	else if( mode & UI_Anchor_Right )
	{
		style.q0x.set( 1 );
		style.q1x.set( 1 );
	}
	if( mode & UI_Anchor_Top )
	{
		style.q0y.set( 0 );
		if( mode & UI_Anchor_Bottom )
			style.q1y.set( 1 );
		else
			style.q1y.set( 0 );
	}
	else if( mode & UI_Anchor_Bottom )
	{
		style.q0y.set( 1 );
		style.q1y.set( 1 );
	}
	_remergeStyle();
}

void UIControl::setAnchorRect( float x0, float y0, float x1, float y1 )
{
	style.q0x.set( x0 );
	style.q0y.set( y0 );
	style.q1x.set( x1 );
	style.q1y.set( y1 );
	_remergeStyle();
}


UIControl* UIControl::_getPrev()
{
	if( !parent.object )
		return NULL;
	size_t pos = VFIND( parent->m_children, Handle( C, m_sgsObject ) );
	if( pos < 1 || pos >= parent->m_children.size() )
		return NULL;
	return parent->m_children[ pos ];
}

UIControl* UIControl::_getNext()
{
	if( !parent.object )
		return NULL;
	size_t pos = VFIND( parent->m_children, Handle( C, m_sgsObject ) );
	if( pos >= parent->m_children.size() - 1 )
		return NULL;
	return parent->m_children[ pos ];
}


bool UIControl::hasEventBinding( sgsString name, sgsVariable callable )
{
	name.push( C );
	if( SGS_FAILED( sgs_PushIndexPI( C, &m_events.var, -1, 0 ) ) )
		return false;
	callable.push();
	// check if item isn't already in the array
	sgs_SizeVal ii = sgs_ObjectAction( C, -2, SGS_ACT_ARRAY_FIND, -1 );
	return ii == SGS_SUCCESS;
}

UIControl::Handle UIControl::bindEvent( sgsString name, sgsVariable callable )
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
			return Handle( this );
		// append new item
		sgs_ObjectAction( C, -2, SGS_ACT_ARRAY_PUSH, 1 );
	}
	return Handle( this );
}

UIControl::Handle UIControl::unbindEvent( sgsString name, sgsVariable callable )
{
	// check if there's an entry for the event
	name.push( C );
	if( SGS_FAILED( sgs_PushIndexPI( C, &m_events.var, -1, 0 ) ) )
		return Handle( this );
	// remove item from array
	callable.push();
	sgs_ObjectAction( C, -2, SGS_ACT_ARRAY_RM_ONE, -1 );
	return Handle( this );
}

UIControl::Handle UIControl::unbindEventAll( sgsString name )
{
	// check if there's an entry for the event
	name.push( C );
	if( SGS_FAILED( sgs_PushIndexPI( C, &m_events.var, -1, 0 ) ) )
		return Handle( this );
	sgs_ObjectAction( C, -1, SGS_ACT_ARRAY_POP, sgs_ArraySize( C, -1 ) );
	return Handle( this );
}

void UIControl::unbindEverything()
{
	sgs_PushDict( C, 0 );
	m_events = sgsVariable( C, -1 );
	sgs_Pop( C, 1 );
}

bool UIControl::callEvent( sgsString name, sgsVariable data )
{
	sgs_StkIdx orig = sgs_StackSize( C );
	// check if there's an entry for the event
	name.push( C );
	if( SGS_FAILED( sgs_PushIndexPI( C, &m_events.var, -1, 0 ) ) )
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
		data.push( C );
		sgs_IterPushData( C, -3, 0, 1 );
		sgs_ThisCall( C, 1, 0 );
		
		sgs_SetStackSize( C, ssz );
	}
	sgs_SetStackSize( C, orig );
	return true;
}

bool UIControl::_callEvent( sgsString name, UIEvent* e )
{
	UI_PushEvent( C, e );
	bool res = callEvent( name, sgsVariable( C, -1 ) );
	sgs_Pop( C, 1 );
	return res;
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
	UIControl::Handle( this ).push( C );
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
	
	if( m_animQueue.size() == 1 )
		_startCurAnim();
	
	return Handle( this );
}

UIControl::Handle UIControl::stop( bool nofinish )
{
	if( !nofinish && m_animQueue.size() && m_animQueue[0].time > 0 )
		_finishCurAnim();
	
	m_animQueue.clear();
	return Handle( this );
}

UIControl::Handle UIControl::dequeue()
{
	if( m_animQueue.size() && m_animQueue[0].time > 0 )
		m_animQueue.erase( m_animQueue.begin() + 1, m_animQueue.end() );
	else
		m_animQueue.clear();
	return Handle( this );
}

UIControl::Handle UIControl::skip( bool nofinish )
{
	if( !nofinish && m_animQueue.size() && m_animQueue[0].time > 0 )
		_finishCurAnim();
	
	return Handle( this );
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
			int ty = sgs_ItemType( C, -1 );
			if( ty == SGS_VT_STRING || ty == SGS_VT_NULL )
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
			}
			
			UIControl::Handle( this ).push( C );
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
		UIControl::Handle( this ).push( C );
		A.oncomplete.push( C );
		sgs_ThisCall( C, 0, 0 );
		sgs_SetStackSize( C, orig );
	}
	
	m_animQueue.erase( m_animQueue.begin() );
	
	if( m_animQueue.size() )
		_startCurAnim();
}

void UIControl::_startCurAnim()
{
	Animation& A = m_animQueue[0];
	
	UIControl::Handle( this ).push( C );
	sgsVariable me( C, -1 );
	sgs_Pop( C, 1 );
	
	// reinitialize startup state to fit in changes
	sgs_StkIdx orig = sgs_StackSize( C );
	if( SGS_SUCCEEDED( sgs_PushIteratorP( C, &A.currState.var ) ) )
	{
		while( sgs_IterAdvance( C, -1 ) > 0 )
		{
			sgs_StkIdx orig2 = sgs_StackSize( C );
			
			sgs_IterPushData( C, -1, 1, 0 ); /* name only */
			sgs_ToString( C, -1 );
			if( SGS_SUCCEEDED( sgs_PushIndexPI( C, &me.var, -1, 1 ) ) )
				sgs_StoreIndexPI( C, &A.prevState.var, -2, 0 );
			
			sgs_SetStackSize( C, orig2 );
		}
	}
	sgs_SetStackSize( C, orig );
}


int UIControl::sgs_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop )
{
	UIControl* ctrl = static_cast<UIControl*>(obj->data);
	int32_t el = sgs_Cntl( C, SGS_CNTL_APILEV, SGS_ERROR );
	SGSRESULT ret = sgs_PushIndexPP( C, &ctrl->_interface.var, key, isprop );
	sgs_Cntl( C, SGS_CNTL_APILEV, el );
	if( SGS_SUCCEEDED( ret ) )
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
	ctrl->_cachedFont.gcmark();
	ctrl->filteredStyle.gcmark();
	ctrl->style.gcmark();
	ctrl->callback.gcmark();
	ctrl->data.gcmark();
	ctrl->_interface.gcmark();
	ctrl->m_events.gcmark();
	for( AnimArray::iterator it = ctrl->m_animQueue.begin(), itend = ctrl->m_animQueue.end(); it != itend; ++it )
		it->gcmark();
	return SGS_SUCCESS;
}


void UIControl::_setClasses3( const char* str1, size_t size1, const char* str2, size_t size2, const char* str3, size_t size3 )
{
	// assume that one of these pointers may be from the current class string
	sgsString _c = classes;
	// allocate a string big enough for all those substrings
	sgs_PushStringBuf( C, NULL, size1 + size2 + size3 );
	// copy substrings to string
	char* out = sgs_GetStringPtr( C, -1 );
	if( size1 ) memcpy( out, str1, size1 );
	if( size2 ) memcpy( out + size1, str2, size2 );
	if( size3 ) memcpy( out + size1 + size2, str3, size3 );
	// retrieve new class string
	classes = sgsString( C, -1 );
	// old one is automatically freed
}

bool UIControl::addClass( const char* str, size_t size )
{
	_trimClass( &str, &size );
	if( !size )
		return false;
	if( hasClass( str, size ) )
		return true;
	_setClasses3( classes.c_str(), classes.size(), " ", classes.size() != 0 ? 1 : 0, str, size );
	frame->_updateStyles( this );
	return true;
}

bool UIControl::removeClass( const char* str, size_t size )
{
	_trimClass( &str, &size );
	if( !size )
		return false;
	size_t cpos = _findClassAt( str, size );
	if( cpos >= classes.size() )
		return false;
	if( classes.size() == size )
		_setClasses3( NULL, 0, NULL, 0, NULL, 0 );
	else if( cpos == 0 )
		_setClasses3( classes.c_str() + size + 1, classes.size() - size - 1, NULL, 0, NULL, 0 );
	else if( cpos == classes.size() - size )
		_setClasses3( classes.c_str(), classes.size() - size - 1, NULL, 0, NULL, 0 );
	else
		_setClasses3( classes.c_str(), cpos, classes.c_str() + cpos + size + 1, classes.size() - cpos - size - 1, NULL, 0 );
	frame->_updateStyles( this );
	return true;
}

bool UIControl::hasClass( const char* str, size_t size )
{
	_trimClass( &str, &size );
	if( !size )
		return false;
	return _findClassAt( str, size ) < classes.size();
}

bool UIControl::hasClassPartBegin( const char* str, size_t size )
{
	_trimClass( &str, &size );
	if( !size )
		return false;
	return _findClassAt( str, size, 1 ) < classes.size();
}

bool UIControl::hasClassPartEnd( const char* str, size_t size )
{
	_trimClass( &str, &size );
	if( !size )
		return false;
	return _findClassAt( str, size, 2 ) < classes.size();
}

bool UIControl::hasClassPart( const char* str, size_t size )
{
	_trimClass( &str, &size );
	if( !size )
		return false;
	return _findClassAt( str, size, 0 ) < classes.size();
}

size_t UIControl::_findClassAt( const char* str, size_t size, int checksides )
{
	size_t cpos = 0;
	if( size > classes.size() )
		return classes.size();
	while( cpos <= classes.size() - size )
	{
		if( memcmp( classes.c_str() + cpos, str, size ) == 0 &&
			( !(checksides&1) || cpos == 0 || classes.c_str()[ cpos - 1 ] == ' ' ) &&
			( !(checksides&2) || cpos + size >= classes.size() || classes.c_str()[ cpos + size ] == ' ' ) )
			return cpos;
		cpos++;
	}
	return classes.size();
}

void UIControl::_trimClass( const char** str, size_t* size )
{
	const char* s1 = *str;
	const char* s2 = s1 + *size;
	while( s1 < s2 && *s1 == ' ' )
		s1++;
	while( s1 < s2 && *(s2-1) == ' ' )
		s2--;
	*str = s1;
	*size = s2 - s1;
}


void UIControl::_refilterStyles( UIFilteredStyleArray& styles )
{
	if( !frame.object )
		return;
	
	styles.clear();
	// gather all matching styles
	StyleSheetArray& SSA = frame->m_styleSheets;
	for( size_t i = 0; i < SSA.size(); ++i )
	{
		UIStyleSheet* SSH = SSA[ i ];
		for( size_t j = 0; j < SSH->rules.size(); ++j )
		{
			UIStyleRule* SR = SSH->rules[ j ];
			for( size_t k = 0; k < SR->selectors.size(); ++k )
			{
				if( UI_SelectorTestControl( &SR->selectors[ k ], this ) )
				{
					UIFilteredStyle fs = { SR, k };
					styles.push_back( fs );
					
					// insertion-sort by importance
					size_t fspos = styles.size() - 1;
					while( fspos > 0 && UI_CompareSelectors(
						&styles[ fspos - 1 ].rule->selectors[ styles[ fspos - 1 ].which_sel ],
						&styles[ fspos ].rule->selectors[ styles[ fspos ].which_sel ] ) < 0 )
					{
						VSWAP( styles, fspos - 1, fspos );
						fspos--;
					}
					
					break;
				}
			}
		}
	}
	
	// save generated style
	filteredStyle = UIStyle();
	for( size_t i = 0; i < styles.size(); ++i )
		UI_StyleMerge( &filteredStyle, &styles[ i ].rule->style );
	
	_remergeStyle();
}

void UIControl::_remergeStyle()
{
	UIStyle ns = style;
	UI_StyleMerge( &ns, &filteredStyle );
	UIStyleCache nsc;
	UI_ToStyleCache( &nsc, &ns );
	_applyStyle( nsc );
}

void UIControl::_applyStyle( const UIStyleCache& nsc )
{
	// diff
#define NEQ( name ) computedStyle.name != nsc.name
	bool updatedFont = NEQ( font ) || NEQ( fontSize );
	bool updatedCursor = NEQ( cursor );
	bool updatedOrder = NEQ( index ) || NEQ( topmost );
	bool updatedBox = NEQ( x ) || NEQ( y ) || NEQ( width ) || NEQ( height )
		|| NEQ( q0x ) || NEQ( q0y ) || NEQ( q1x ) || NEQ( q1y )
		|| NEQ( nonClientTop ) || NEQ( nonClientLeft ) || NEQ( nonClientRight ) || NEQ( nonClientBottom )
		|| NEQ( minWidth ) || NEQ( maxWidth ) || NEQ( minHeight ) || NEQ( maxHeight )
		|| NEQ( marginLeft ) || NEQ( marginRight ) || NEQ( marginTop ) || NEQ( marginBottom )
		|| NEQ( paddingLeft ) || NEQ( paddingRight ) || NEQ( paddingTop ) || NEQ( paddingBottom )
	;
	// TODO: event-based rendering
//	bool updatedRenderFunc = NEQ( renderfunc );
	bool updatedLayout = updatedFont || updatedCursor || updatedOrder || updatedBox;
#undef NEQ
	
	// apply new style
	computedStyle = nsc;
	
	// act on diffs
	if( updatedFont ) updateFont();
	if( updatedCursor ) updateCursor();
	if( updatedOrder ) sortSiblings();
	if( updatedLayout ) updateLayout();
	if( ( updatedBox || updatedOrder ) && frame.not_null() )
		frame->handleMouseMove( true );
}

void UIControl::_changedFullRect()
{
	if( _roundedCoords )
	{
		rx0 = round( rx0 );
		rx1 = round( rx1 );
		ry0 = round( ry0 );
		ry1 = round( ry1 );
	}
	cx0 = rx0 + get_nonClientLeft();
	cx1 = rx1 - get_nonClientRight();
	cy0 = ry0 + get_nonClientTop();
	cy1 = ry1 - get_nonClientBottom();
	if( _roundedCoords )
	{
		cx0 = round( cx0 );
		cx1 = round( cx1 );
		cy0 = round( cy0 );
		cy1 = round( cy1 );
	}
	px0 = cx0 + get_paddingLeft();
	px1 = cx1 - get_paddingRight();
	py0 = cy0 + get_paddingTop();
	py1 = cy1 - get_paddingBottom();
	if( _roundedCoords )
	{
		px0 = round( px0 );
		px1 = round( px1 );
		py0 = round( py0 );
		py1 = round( py1 );
	}
}


const char* UIQuery::IteratorTypeName = "UIQueryIterator";

int UIQuery::sgs_convert( SGS_CTX, sgs_VarObj* obj, int type )
{
	if( type == SGS_CONVOP_TOITER )
	{
		UIQuery* Q = (UIQuery*) obj->data;
		SGS_PUSHLITECLASS( C, sgsArrayIterator<UIQuery>, ( Q ) );
		return SGS_SUCCESS;
	}
	return SGS_ENOTSUP;
}

int UIQuery::sgs_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop )
{
	char* name;
	sgs_Int idx;
	UIQuery* Q = (UIQuery*) obj->data;
	if( isprop && sgs_ParseStringP( C, key, &name, NULL ) )
	{
		if( 0 == strcmp( name, "size" ) ) SGS_RETURN_INT( Q->m_items.size() )
	}
	else if( !isprop && sgs_ParseIntP( C, key, &idx ) )
	{
		if( idx < 0 || idx >= Q->m_items.size() )
			return SGS_EBOUNDS;
		Q->m_items[ idx ].push( C );
		return SGS_SUCCESS;
	}
	return UIQuery::_sgs_getindex( C, obj, key, isprop );
}

int UIQuery::sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	UIQuery* Q = (UIQuery*) obj->data;
	Q->m_frame.gcmark();
	for( size_t i = 0; i < Q->m_items.size(); ++i )
		Q->m_items[ i ].gcmark();
	return SGS_SUCCESS;
}

bool UIQuery::_parseArgs( sgs_StkIdx stacksize )
{
	for( sgs_StkIdx i = 0; i < stacksize; ++i )
	{
		if( sgs_IsObject( C, i, UIControl::_sgs_interface ) )
		{
			UIControl::Handle hctrl = sgs_GetVar<UIControl::Handle>()( C, i );
			if( !_hasCtrl( hctrl ) )
				m_items.push_back( hctrl );
		}
		else if( sgs_ItemType( C, i ) == SGS_VT_STRING )
		{
			sgsString str( C, i );
			const char* err = StyleArr_addSelectors( m_selectors, str );
			if( err )
			{
				sgs_Msg( C, SGS_WARNING, "failed to compile selector (error at position %d)", err - str.c_str() );
				return false;
			}
		}
		else
		{
			sgs_Msg( C, SGS_WARNING, "unexpected type passed (not UIControl / selector string) as argument %d", i + 1 );
			return false;
		}
	}
	return true;
}

bool UIQuery::_checkCtrl( UIControl* ctrl )
{
	for( size_t i = 0; i < m_selectors.size(); ++i )
		if( !UI_SelectorTestControl( &m_selectors[ i ], ctrl ) )
			return false;
	return true;
}

bool UIQuery::_hasCtrl( UIControl* ctrl )
{
	return VFIND( m_items, UIControl::Handle( ctrl ) ) < m_items.size();
}


UIQuery::Handle UIQuery::Create( /* args, */ UIFrame::Handle frame )
{
	if( !frame.object )
		return UIQuery::Handle();
	SGS_CTX = frame.C;
	sgs_StkIdx ssz = sgs_StackSize( C );
	
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	
	// ITERATE ALL CONTROLS
	std::vector< UIControl* > controls;
	controls.push_back( frame->root );
	while( controls.size() )
	{
		UIControl* ctrl = controls[0];
		VREMOVEAT( controls, 0 );
		
		if( Q->_checkCtrl( ctrl ) && !Q->_hasCtrl( ctrl ) )
			Q->m_items.push_back( UIControl::Handle( ctrl ) );
		
		for( size_t i = 0; i < ctrl->m_children.size(); ++i )
			controls.push_back( ctrl->m_children[ i ] );
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::find( /* args */ )
{
	sgs_StkIdx ssz = sgs_StackSize( C );
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	
	// ITERATE ALL SUB-CONTROLS
	std::vector< UIControl* > controls;
	for( size_t i = 0; i < m_items.size(); ++i )
		controls.push_back( m_items[ i ] );
	while( controls.size() )
	{
		UIControl* ctrl = controls[0];
		VREMOVEAT( controls, 0 );
		
		if( Q->_checkCtrl( ctrl ) && !Q->_hasCtrl( ctrl ) )
			Q->m_items.push_back( UIControl::Handle( ctrl ) );
		
		for( size_t i = 0; i < ctrl->m_children.size(); ++i )
			controls.push_back( ctrl->m_children[ i ] );
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::children( /* args */ )
{
	sgs_StkIdx ssz = sgs_StackSize( C );
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	
	// ITERATE ALL CHILD CONTROLS
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		UIControl* ctrl = m_items[ i ];
		for( size_t j = 0; j < ctrl->m_children.size(); ++j )
		{
			if( Q->_checkCtrl( ctrl->m_children[ j ] ) && !Q->_hasCtrl( ctrl->m_children[ j ] ) )
				Q->m_items.push_back( ctrl->m_children[ j ] );
		}
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::parent()
{
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	
	// RETRIEVE ALL PARENT CONTROLS
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		UIControl* ctrl = m_items[ i ];
		if( ctrl->parent.object )
		{
			if( !Q->_hasCtrl( ctrl->parent ) )
				Q->m_items.push_back( ctrl->parent );
		}
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::closest( /* args */ )
{
	sgs_StkIdx ssz = sgs_StackSize( C );
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	
	// ITERATE ALL ANCESTOR CONTROLS UNTIL MATCH IS FOUND (starting from self)
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		UIControl* ctrl = m_items[ i ];
		while( ctrl )
		{
			if( Q->_checkCtrl( ctrl ) )
			{
				if( !Q->_hasCtrl( m_items[i] ) )
					Q->m_items.push_back( m_items[i] );
				break;
			}
			ctrl = ctrl->parent;
		}
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::prev( /* args */ )
{
	sgs_StkIdx ssz = sgs_StackSize( C );
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	
	// CHECK ONE SIBLING CONTROL BEFORE CURRENT
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		UIControl* ctrl = m_items[ i ]->_getPrev();
		if( ctrl && Q->_checkCtrl( ctrl ) && !Q->_hasCtrl( ctrl ) )
			Q->m_items.push_back( m_items[i] );
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::next( /* args */ )
{
	sgs_StkIdx ssz = sgs_StackSize( C );
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	
	// CHECK ONE SIBLING CONTROL AFTER CURRENT
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		UIControl* ctrl = m_items[ i ]->_getNext();
		if( ctrl && Q->_checkCtrl( ctrl ) && !Q->_hasCtrl( ctrl ) )
			Q->m_items.push_back( m_items[i] );
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::prevAll( /* args */ )
{
	sgs_StkIdx ssz = sgs_StackSize( C );
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	
	// ITERATE ALL SIBLING CONTROLS BEFORE CURRENT
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		UIControl* ctrl = m_items[ i ]->_getPrev();
		while( ctrl )
		{
			if( Q->_checkCtrl( ctrl ) && !Q->_hasCtrl( ctrl ) )
				Q->m_items.push_back( m_items[i] );
			ctrl = ctrl->_getPrev();
		}
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::nextAll( /* args */ )
{
	sgs_StkIdx ssz = sgs_StackSize( C );
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	
	// ITERATE ALL SIBLING CONTROLS AFTER CURRENT
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		UIControl* ctrl = m_items[ i ]->_getNext();
		while( ctrl )
		{
			if( Q->_checkCtrl( ctrl ) && !Q->_hasCtrl( ctrl ) )
				Q->m_items.push_back( m_items[i] );
			ctrl = ctrl->_getNext();
		}
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::filter( /* args */ )
{
	sgs_StkIdx ssz = sgs_StackSize( C );
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( !Q->_parseArgs( ssz ) )
		return UIQuery::Handle();
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		UIControl* ctrl = m_items[ i ];
		if( Q->_checkCtrl( ctrl ) && !Q->_hasCtrl( ctrl ) )
			Q->m_items.push_back( m_items[i] );
	}
	
	return Handle( Q );
}

UIQuery::Handle UIQuery::first()
{
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( m_items.size() )
		Q->m_items.push_back( m_items[0] );
	return Handle( Q );
}

UIQuery::Handle UIQuery::last()
{
	UIQuery* Q = SGS_PUSHCLASS( C, UIQuery, () );
	Q->m_frame = m_frame;
	if( m_items.size() )
		Q->m_items.push_back( VLASTOF( m_items ) );
	return Handle( Q );
}

sgsVariable UIQuery::getAttr( sgsString key )
{
	if( !m_items.size() )
		return sgsVariable();
	m_items[0].push( C );
	int32_t prevlev = sgs_Cntl( C, SGS_CNTL_APILEV, 0 ); // enable error dumping to avoid rewriting errors
	SGSRESULT res = sgs_PushIndexII( C, -1, 0, 1 );
	sgs_Cntl( C, SGS_CNTL_APILEV, prevlev );
	return SGS_FAILED( res ) ? sgsVariable() : sgsVariable( C, -1 );
}

UIQuery::Handle UIQuery::setAttr( sgsString key, sgsVariable value )
{
	for( size_t i = 0; i < m_items.size(); ++i )
	{
		m_items[ i ].push( C );
		int32_t prevlev = sgs_Cntl( C, SGS_CNTL_APILEV, 0 ); // enable error dumping to avoid rewriting errors
		SGSRESULT res = sgs_SetIndexIII( C, -1, 0, 1, 1 );
		sgs_Cntl( C, SGS_CNTL_APILEV, prevlev );
		if( SGS_FAILED( res ) )
			return Handle();
	}
	return Handle( this );
}

UIQuery::Handle UIQuery::addClass( const sgsString& ss )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->addClass( ss );
	return Handle( this );
}

UIQuery::Handle UIQuery::removeClass( const sgsString& ss )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->removeClass( ss );
	return Handle( this );
}

UIQuery::Handle UIQuery::bindEvent( sgsString name, sgsVariable callable )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->bindEvent( name, callable );
	return Handle( this );
}

UIQuery::Handle UIQuery::unbindEvent( sgsString name, sgsVariable callable )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->unbindEvent( name, callable );
	return Handle( this );
}

UIQuery::Handle UIQuery::unbindEventAll( sgsString name )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->unbindEventAll( name );
	return Handle( this );
}

UIQuery::Handle UIQuery::callEvent( sgsString name, sgsVariable data )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->callEvent( name, data );
	return Handle( this );
}

UIQuery::Handle UIQuery::animate( sgsVariable state, float length, sgsVariable func, sgsVariable oncomplete )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->animate( state, length, func, oncomplete );
	return Handle( this );
}

UIQuery::Handle UIQuery::stop( bool nofinish )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->stop( nofinish );
	return Handle( this );
}

UIQuery::Handle UIQuery::dequeue()
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->dequeue();
	return Handle( this );
}

UIQuery::Handle UIQuery::skip( bool nofinish )
{
	for( size_t i = 0; i < m_items.size(); ++i )
		m_items[ i ]->skip( nofinish );
	return Handle( this );
}


