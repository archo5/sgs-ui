

#include "ui_control.h"

#define FN( x ) { #x, x }



int UI_CreateEvent( SGS_CTX )
{
	UIEvent* ev = SGS_PUSHLITECLASS( C, UIEvent, () );
	ev->type = sgs_GetInt( C, 0 );
	return 1;
}

int UI_CreateStyle( SGS_CTX )
{
	SGS_PUSHLITECLASS( C, UIStyle, () );
	return 1;
}

int UI_CreateStyleRule( SGS_CTX )
{
	SGS_PUSHCLASS( C, UIStyleRule, () );
	return 1;
}

int UI_CreateStyleSheet( SGS_CTX )
{
	SGS_PUSHCLASS( C, UIStyleSheet, () );
	return 1;
}

int UI_CreateFrame( SGS_CTX )
{
	SGS_PUSHCLASS( C, UIFrame, () );
	return 1;
}


int UI_EasingFunction_linear( SGS_CTX )
{
	SGSFN( "UI_EasingFunction_linear" );
	float v0 = sgs_GetReal( C, 0 );
	float v1 = sgs_GetReal( C, 1 );
	float q = sgs_GetReal( C, 2 );
	sgs_PushReal( C, v0 * (1-q) + v1 * q );
	return 1;
}

int UI_EasingFunction_smooth( SGS_CTX )
{
	SGSFN( "UI_EasingFunction_smooth" );
	float v0 = sgs_GetReal( C, 0 );
	float v1 = sgs_GetReal( C, 1 );
	float q = sgs_GetReal( C, 2 );
	q = q*q*(3 - 2*q);
	sgs_PushReal( C, v0 * (1-q) + v1 * q );
	return 1;
}

int UI_EasingFunction_smoother( SGS_CTX )
{
	SGSFN( "UI_EasingFunction_smoother" );
	float v0 = sgs_GetReal( C, 0 );
	float v1 = sgs_GetReal( C, 1 );
	float q = sgs_GetReal( C, 2 );
	q = q*q*q*(q*(q*6 - 15) + 10);
	sgs_PushReal( C, v0 * (1-q) + v1 * q );
	return 1;
}


sgs_RegIntConst g_iconsts[] =
{
	FN( Anchor_Top    ),
	FN( Anchor_Bottom ),
	FN( Anchor_Left   ),
	FN( Anchor_Right  ),
	FN( Anchor_Hor    ),
	FN( Anchor_Vert   ),
	FN( Anchor_All    ),
	
	FN( EV_Paint      ),
	FN( EV_Layout     ),
	FN( EV_ChgTheme   ),
	FN( EV_KeyDown    ),
	FN( EV_KeyUp      ),
	FN( EV_Char       ),
	FN( EV_FocusEnter ),
	FN( EV_FocusLeave ),
	FN( EV_NeedFocus  ),
	FN( EV_ButtonDown ),
	FN( EV_ButtonUp   ),
	FN( EV_MouseMove  ),
	FN( EV_MouseWheel ),
	FN( EV_MouseEnter ),
	FN( EV_MouseLeave ),
	FN( EV_Attach     ),
	FN( EV_Detach     ),
	FN( EV_HitTest    ),
	FN( EV_AddChild   ),
	FN( EV_RemChild   ),
	
	FN( Mouse_ButtonL ),
	FN( Mouse_ButtonR ),
	FN( Mouse_ButtonM ),
	
	FN( Key_Left      ),
	FN( Key_Right     ),
	FN( Key_Up        ),
	FN( Key_Down      ),
	FN( Key_DelLeft   ),
	FN( Key_DelRight  ),
	FN( Key_Tab       ),
	FN( Key_Cut       ),
	FN( Key_Copy      ),
	FN( Key_Paste     ),
	FN( Key_Undo      ),
	FN( Key_Redo      ),
	FN( Key_SelectAll ),
	FN( Key_PageUp    ),
	FN( Key_PageDown  ),
	
	FN( Hit_Client    ),
	FN( Hit_NonClient ),
	FN( Hit_None      ),
	
	FN( UI_Cursor_Arrow     ),
	FN( UI_Cursor_Text      ),
	FN( UI_Cursor_Wait      ),
	FN( UI_Cursor_Crosshair ),
	FN( UI_Cursor_WaitArrow ),
	FN( UI_Cursor_SizeNWSE  ),
	FN( UI_Cursor_SizeNESW  ),
	FN( UI_Cursor_SizeWE    ),
	FN( UI_Cursor_SizeNS    ),
	FN( UI_Cursor_SizeAll   ),
	FN( UI_Cursor_No        ),
	FN( UI_Cursor_Hand      ),
	
	FN( KeyMod_Filter ),
	FN( KeyMod_Shift  ),
	
	{NULL,0},
};

sgs_RegRealConst g_rconsts[] =
{
	FN( UI_MaxValue ),
	{NULL,0},
};

sgs_RegFuncConst g_fconsts[] =
{
	FN( UI_CreateEvent ),
	FN( UI_CreateStyle ),
	FN( UI_CreateStyleRule ),
	FN( UI_CreateStyleSheet ),
	FN( UI_CreateFrame ),
	FN( UI_EasingFunction_linear ),
	FN( UI_EasingFunction_smooth ),
	FN( UI_EasingFunction_smoother ),
	{NULL,NULL},
};


extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
int sgscript_main( SGS_CTX )
{
	sgs_RegIntConsts( C, g_iconsts, -1 );
	sgs_RegRealConsts( C, g_rconsts, -1 );
	sgs_RegFuncConsts( C, g_fconsts, -1 );
	
	sgs_StkIdx start = sgs_StackSize( C );
	sgs_PushString( C, "linear" );
	sgs_PushCFunction( C, UI_EasingFunction_linear );
	sgs_PushString( C, "smooth" );
	sgs_PushCFunction( C, UI_EasingFunction_smooth );
	sgs_PushString( C, "smoother" );
	sgs_PushCFunction( C, UI_EasingFunction_smoother );
	sgs_PushDict( C, sgs_StackSize( C ) - start );
	sgs_StoreGlobal( C, "UI_EasingFunctions" );
	
	return SGS_SUCCESS;
}

