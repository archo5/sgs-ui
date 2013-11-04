

#include "ui_control.h"

#define FN( x ) { #x, x }



int ui_create_event( SGS_CTX )
{
	SGS_PUSH_CLASS( C, UIEvent, () );
	return 1;
}


sgs_RegIntConst g_iconsts[] =
{
	FN( EV_Paint      ),
	FN( EV_Layout     ),
	FN( EV_KeyDown    ),
	FN( EV_KeyUp      ),
	FN( EV_Char       ),
	FN( EV_ButtonDown ),
	FN( EV_ButtonUp   ),
	FN( EV_MouseMove  ),
	{0,0}
};

sgs_RegFuncConst g_fconsts[] =
{
	FN( ui_create_event ),
	{0,0}
};


int sgscript_main( SGS_CTX )
{
	sgs_RegIntConsts( C, g_iconsts, -1 );
	sgs_RegFuncConsts( C, g_fconsts, -1 );
	
	return SGS_SUCCESS;
}

