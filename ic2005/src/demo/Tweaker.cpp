#include "stdafx.h"
#include "Tweaker.h"
#include <dingus/gfx/gui/Gui.h>
#include <dingus/renderer/EffectParamsNotifier.h>


namespace {

	
	CUIDialog*	dialog;
	bool		visible = false;

	tweaker::SOptions	options;

}; // anonymous namespace


// --------------------------------------------------------------------------


tweaker::SOptions::SOptions()
:	wireframe(false)
,	shadows(true)
,	reflections(true)
,	dof(true)
,	normalmaps(true)
,	ao(true)
{
}


void tweaker::SOptions::apply()
{
	CEffectBundle& bfx = CEffectBundle::getInstance();
	bfx.setMacro( "D_SHADOWS", shadows ? "1" : "0" );
	bfx.setMacro( "D_REFLECTIONS", reflections ? "1" : "0" );
	bfx.setMacro( "D_NORMALMAPS", normalmaps ? "1" : "0" );
	bfx.setMacro( "D_AO", ao ? "1" : "0" );
	bfx.reload();
	CEffectParamsNotifier::getInstance().notify();
}



enum eTweakControls {
	GID_CHK_WIREFRAME = 5000,
	GID_CHK_SHADOWS,
	GID_CHK_REFLECTIONS,
	GID_CHK_DOF,
	GID_CHK_NORMALMAPS,
	GID_CHK_AO,
};


// --------------------------------------------------------------------------

namespace {

void CALLBACK dlgCallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
	if( evt == UIEVENT_CHECKBOX_CHANGED ) {
		CUICheckBox* cbox = (CUICheckBox*)ctrl;
		bool checked = cbox->isChecked();
		switch( ctrlID ) {
		case GID_CHK_WIREFRAME:
			options.wireframe = checked;
			break;
		case GID_CHK_SHADOWS:
			options.shadows = checked;
			options.apply();
			break;
		case GID_CHK_REFLECTIONS:
			options.reflections = checked;
			options.apply();
			break;
		case GID_CHK_DOF:
			options.dof = checked;
			break;
		case GID_CHK_NORMALMAPS:
			options.normalmaps = checked;
			options.apply();
			break;
		case GID_CHK_AO:
			options.ao = checked;
			options.apply();
			break;
		}
	}
	if( evt == UIEVENT_BUTTON_CLICKED && ctrlID == IDOK ) {
		assert( visible );
		visible = false;
	}
}

}


void tweaker::init()
{
	visible = false;

	dialog = new CUIDialog();
	dialog->enableNonUserEvents( true );
	dialog->setCallback( dlgCallback );
	dialog->setBackgroundColors( 0x80303030 );
	dialog->setLocation( 120, 70 );
	dialog->setSize( 420, 340 );
	dialog->setFont( 1, "Verdana", 22, 50 );
	dialog->setFont( 2, "Verdana", 14, 50 );

	int yline;
	const float HC = 18;
	const float DYLINE = HC+2;

	CUIStatic* lab;

	const int xcol = 30;
	yline = 10;
	dialog->addStatic( 0, "Graphics options", xcol-10, yline, 200, 22, false, &lab );
	lab->getElement(0)->setFont( 1, false, DT_LEFT | DT_VCENTER );

	dialog->addCheckBox( GID_CHK_WIREFRAME, "Wireframe", xcol, yline+=DYLINE, 130, HC, options.wireframe );
	dialog->addCheckBox( GID_CHK_SHADOWS, "Shadows", xcol, yline+=DYLINE, 130, HC, options.shadows );
	dialog->addCheckBox( GID_CHK_REFLECTIONS, "Reflections", xcol, yline+=DYLINE, 130, HC, options.reflections );
	dialog->addCheckBox( GID_CHK_DOF, "Depth of field", xcol, yline+=DYLINE, 130, HC, options.dof );
	dialog->addCheckBox( GID_CHK_NORMALMAPS, "Normal mapping", xcol, yline+=DYLINE, 130, HC, options.normalmaps );
	dialog->addCheckBox( GID_CHK_AO, "Ambient occlusion", xcol, yline+=DYLINE, 130, HC, options.ao );
	
	// buttons
	dialog->addButton( IDOK, "Close", 340, dialog->getHeight()-35, 58, 20 );
}

void tweaker::shutdown()
{
	delete dialog;
}


void tweaker::show()
{
	assert( !visible );
	visible = true;
}

void tweaker::hide()
{
	visible = false;
}

bool tweaker::isVisible()
{
	return visible;
}

CUIDialog& tweaker::getDlg()
{
	assert( dialog );
	return *dialog;
}

const tweaker::SOptions& tweaker::getOptions()
{
	return options;
}
