

#include "ui_control.h"

#define FN( x ) { #x, x }



int UI_CreateColor( SGS_CTX )
{
	float r, g, b, a = 1;
	SGSFN( "UIColor" );
	sgs_StkIdx ssz = sgs_StackSize( C );
	if( !sgs_LoadArgs( C, "f|fff.", &r, &g, &b, &a ) )
		return 0;
	if( ssz == 1 ){ a = b = g = r; }
	else if( ssz == 2 ){ a = g; b = g = r; }
	
	sgs_PushVar( C, UIColor( r, g, b, a ) );
	return 1;
}

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

int UI_BuildStyleSheet( SGS_CTX )
{
	SGSFN( "UI_BuildStyleSheet" );
	if( !sgs_LoadArgs( C, "?v" ) )
		return 0;
	UIStyleSheet* ssh = SGS_PUSHCLASS( C, UIStyleSheet, () );
	ssh->build( sgsVariable( C, 0 ) );
	return 1;
}

int UI_CreateFrame( SGS_CTX )
{
	SGS_PUSHCLASS( C, UIFrame, () );
	return 1;
}


static int _UI_EasingFunction_Core( SGS_CTX, float q )
{
	if( sgs_IsObject( C, 0, UIColor::_sgs_interface ) || sgs_IsObject( C, 1, UIColor::_sgs_interface ) )
	{
		UIColor A = sgs_GetVar<UIColor>()( C, 0 );
		UIColor B = sgs_GetVar<UIColor>()( C, 1 );
		sgs_PushVar( C, A.scale( 1-q ) + B.scale( q ) );
	}
	else
	{
		float v0 = sgs_GetReal( C, 0 );
		float v1 = sgs_GetReal( C, 1 );
		sgs_PushReal( C, v0 * (1-q) + v1 * q );
	}
	return 1;
}

int UI_EasingFunction_linear( SGS_CTX )
{
	SGSFN( "UI_EasingFunction_linear" );
	float q = sgs_GetReal( C, 2 );
	return _UI_EasingFunction_Core( C, q );
}

int UI_EasingFunction_smooth( SGS_CTX )
{
	SGSFN( "UI_EasingFunction_smooth" );
	float q = sgs_GetReal( C, 2 );
	q = q*q*(3 - 2*q);
	return _UI_EasingFunction_Core( C, q );
}

int UI_EasingFunction_smoother( SGS_CTX )
{
	SGSFN( "UI_EasingFunction_smoother" );
	float q = sgs_GetReal( C, 2 );
	q = q*q*q*(q*(q*6 - 15) + 10);
	return _UI_EasingFunction_Core( C, q );
}


sgs_RegIntConst g_iconsts[] =
{
	FN( UI_Anchor_Top    ),
	FN( UI_Anchor_Bottom ),
	FN( UI_Anchor_Left   ),
	FN( UI_Anchor_Right  ),
	FN( UI_Anchor_Hor    ),
	FN( UI_Anchor_Vert   ),
	FN( UI_Anchor_All    ),
	
	FN( UI_Align_Left    ),
	FN( UI_Align_Center  ),
	FN( UI_Align_Right   ),
	FN( UI_Align_Justify ),
	FN( UI_Align_Top     ),
	FN( UI_Align_VCenter ),
	FN( UI_Align_Bottom  ),
	
	FN( UI_Pos_Abs   ),
	FN( UI_Pos_Line  ),
	FN( UI_Pos_SideA ),
	FN( UI_Pos_SideB ),
	
	FN( UI_Stack_Top    ),
	FN( UI_Stack_Left   ),
	FN( UI_Stack_Bottom ),
	FN( UI_Stack_Right  ),
	FN( UI_Stack_AutoSize       ),
	FN( UI_Stack_AutoSizeTop    ),
	FN( UI_Stack_AutoSizeLeft   ),
	FN( UI_Stack_AutoSizeBottom ),
	FN( UI_Stack_AutoSizeRight  ),
	
	FN( EV_Changed      ),
	FN( EV_ChgTheme     ),
	FN( EV_KeyDown      ),
	FN( EV_KeyUp        ),
	FN( EV_Char         ),
	FN( EV_FocusEnter   ),
	FN( EV_FocusLeave   ),
	FN( EV_NeedFocus    ),
	FN( EV_ButtonDown   ),
	FN( EV_ButtonUp     ),
	FN( EV_MouseMove    ),
	FN( EV_MouseWheel   ),
	FN( EV_MouseEnter   ),
	FN( EV_MouseLeave   ),
	FN( EV_Attach       ),
	FN( EV_Detach       ),
	FN( EV_HitTest      ),
	FN( EV_AddChild     ),
	FN( EV_RemChild     ),
	{ "EV_RemoveChild", EV_RemChild },
	FN( EV_AddComponent ),
	FN( EV_RemComponent ),
	{ "EV_RemoveComponent", EV_RemComponent },
	
	FN( EV_Changed_Font   ),
	FN( EV_Changed_Image  ),
	FN( EV_Changed_Icon   ),
	FN( EV_Changed_Cursor ),
	FN( EV_Changed_Order  ),
	FN( EV_Changed_Box    ),
	FN( EV_Changed_Layout ),
	
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
	FN( Key_Enter     ),
	
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
	{ "UIColor", UI_CreateColor },
	FN( UI_CreateEvent ),
	FN( UI_CreateStyle ),
	FN( UI_CreateStyleRule ),
	FN( UI_CreateStyleSheet ),
	FN( UI_BuildStyleSheet ),
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
	SGS_MODULE_CHECK_VERSION( C );
	
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
	sgs_PushDict( C, 0 );
	sgs_StoreGlobal( C, "UI_Controls" );
	
	return SGS_SUCCESS;
}

