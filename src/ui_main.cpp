

#include "ui_control.h"

#define FN( x ) { #x, x }



int ui_create_event( SGS_CTX )
{
	UIEvent* ev = SGS_PUSHCLASS( C, UIEvent, () );
	ev->type = sgs_GetInt( C, 0 );
	return 1;
}

int ui_create_frame( SGS_CTX )
{
	SGS_PUSHCLASS( C, UIFrame, () );
	return 1;
}


sgs_RegIntConst g_iconsts[] =
{
	FN( EV_Paint      ),
	FN( EV_Layout     ),
	FN( EV_KeyDown    ),
	FN( EV_KeyUp      ),
	FN( EV_Char       ),
	FN( EV_FocusEnter ),
	FN( EV_FocusLeave ),
	FN( EV_NeedFocus  ),
	FN( EV_ButtonDown ),
	FN( EV_ButtonUp   ),
	FN( EV_MouseMove  ),
	FN( EV_MouseEnter ),
	FN( EV_MouseLeave ),
	FN( EV_Attach     ),
	FN( EV_Detach     ),
	FN( EV_HitTest    ),
	
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
	
	FN( Hit_Client    ),
	FN( Hit_NonClient ),
	FN( Hit_None      ),
	
	FN( KeyMod_Filter ),
	FN( KeyMod_Shift  ),
	
	{NULL,0},
};

sgs_RegFuncConst g_fconsts[] =
{
	FN( ui_create_event ),
	FN( ui_create_frame ),
	{NULL,NULL},
};


extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
int sgscript_main( SGS_CTX )
{
	sgs_RegIntConsts( C, g_iconsts, -1 );
	sgs_RegFuncConsts( C, g_fconsts, -1 );
	
	return SGS_SUCCESS;
}

