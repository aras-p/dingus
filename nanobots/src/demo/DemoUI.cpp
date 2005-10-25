#include "stdafx.h"
#include "DemoUI.h"
#include "../resource.h"


// --------------------------------------------------------------------------

const int GID_SLD_GFX_DETAIL = 6000;
const int GID_CHK_LEVELCACHE = 6004;


// --------------------------------------------------------------------------


CDemoSettingsDialog* CDemoSettingsDialog::mSingleInstance = 0;


// --------------------------------------------------------------------------



CDemoSettingsDialog::CDemoSettingsDialog( const CD3DEnumeration& enumeration, const CD3DSettings& settings )
:	CAbstractD3DSettingsDialog( enumeration, settings ),
	mState( STATE_CANCEL )
{
	assert( !mSingleInstance );
	mSingleInstance = this;

	//
	// UI
	
	mDlg = new CUIDialog();
	mDlg->enableNonUserEvents( true );
	mDlg->setCallback( dialogCallback );
	mDlg->setBackgroundColors( 0xA0303030 );
	mDlg->setLocation( 50, 50 );
	mDlg->setSize( 540, 320 );
	mDlg->setFont( 1, "Arial", 22, 50 );
	mDlg->setFont( 2, "Arial", 14, 50 );

	int yline;
	const float HC = 18;
	const float DYLINE = HC+2;

	CUIStatic* lab;

	// video settings
	{
		const int xcol = 30;
		yline = 10;
		mDlg->addStatic( 0, "Video settings", xcol-10, yline, 200, 22, false, &lab );
		lab->getElement(0)->setFont( 1, false, DT_LEFT | DT_VCENTER );

		mDlg->addStatic( 0, "Adapter:", xcol, yline+=24, 45, HC );
		mDlg->addComboBox( IDC_ADAPTER_COMBO, xcol+47, yline, 170, HC );
		mDlg->addComboBox( IDC_DEVICE_COMBO, xcol+219, yline, 100, HC );

		mDlg->addStatic( 0, "Video mode:", xcol, yline+=DYLINE, 58, HC );
		mDlg->addCheckBox( IDC_CHK_FULLSCREEN, "Fullscreen", xcol+60, yline, 74, HC-2 );
		mDlg->addComboBox( IDC_RESOLUTION_COMBO, xcol+135, yline, 88, HC );
		mDlg->addComboBox( IDC_REFRESHRATE_COMBO, xcol+225, yline, 68, HC );
		mDlg->addComboBox( IDC_ADAPTERFORMAT_COMBO, xcol+295, yline, 93, HC );

		mDlg->addCheckBox( IDC_CHK_VSYNC, "Synchronize with monitor's refresh", xcol+10, yline+=DYLINE, 200, HC-2 );

		mDlg->addStatic( 0, "Antialiasing:", xcol+10, yline+=DYLINE, 58, HC );
		mDlg->addComboBox( IDC_MULTISAMPLE_COMBO, xcol+70, yline, 88, HC );
		mDlg->addComboBox( IDC_MULTISAMPLE_QUALITY_COMBO, xcol+165, yline, 38, HC );

		mDlg->addStatic( 0, "Graphics detail:", xcol, yline+=DYLINE, 78, HC );
		mDlg->addSlider( GID_SLD_GFX_DETAIL, xcol+80, yline, 60, HC, 0, GFX_DETAIL_LEVELS-1, GFX_DETAIL_LEVELS-1 );

		mDlg->addStatic( 0, "Advanced video settings", xcol, yline+=HC+HC, 200, 18, false, &lab );
		lab->getElement(0)->setFont( 2, false, DT_LEFT | DT_VCENTER );

		mDlg->addStatic( 0, "Backbuffer:", xcol+10, yline+=DYLINE, 58, HC );
		mDlg->addComboBox( IDC_BACKBUFFERFORMAT_COMBO, xcol+70, yline, 88, HC );
		mDlg->addStatic( 0, "Z/stencil:", xcol+165, yline, 48, HC );
		mDlg->addComboBox( IDC_DEPTHSTENCILBUFFERFORMAT_COMBO, xcol+210, yline, 78, HC );
		mDlg->addStatic( 0, "Vertex processing:", xcol+295, yline, 83, HC );
		mDlg->addComboBox( IDC_VERTEXPROCESSING_COMBO, xcol+380, yline, 98, HC );
	}
	// game settings
	{
		const int xcol = 30;
		yline += 40;
		mDlg->addStatic( 0, "Game settings", xcol-10, yline, 200, 22, false, &lab );
		lab->getElement(0)->setFont( 1, false, DT_LEFT | DT_VCENTER );

		mDlg->addCheckBox( GID_CHK_LEVELCACHE, "Cache tissue meshes", xcol, yline+=24, 150, HC, mAppSettings.cacheLevels );
	}

	// buttons
	mDlg->addButton( IDOK, "OK", 390, mDlg->getHeight()-35, 58, 20 );
	mDlg->addButton( IDCANCEL, "Cancel", 460, mDlg->getHeight()-35, 58, 20 );
}

CDemoSettingsDialog::~CDemoSettingsDialog()
{
	delete mDlg;
}


void CDemoSettingsDialog::showDialog( const CD3DSettings& settings )
{
	assert( mState != STATE_ACTIVE );
	mState = STATE_ACTIVE;
	mSettings = settings;

	// fill adapter combo box, updating the selected adapter will trigger
	// updates of the rest of the dialog.
	comboBoxClear( IDC_ADAPTER_COMBO );
	for( int iai = 0; iai < mEnumeration->mAdapterInfos.size(); ++iai ) {
		const SD3DAdapterInfo* adInfo = mEnumeration->mAdapterInfos[iai];
		comboBoxAdd( IDC_ADAPTER_COMBO, adInfo, adInfo->adapterID.Description );
		if( adInfo->adapterOrdinal == mSettings.getAdapterOrdinal() )
			comboBoxSelect( IDC_ADAPTER_COMBO, adInfo );
	}
	if( !comboBoxSomethingSelected( IDC_ADAPTER_COMBO ) && comboBoxCount( IDC_ADAPTER_COMBO ) > 0 )
		comboBoxSelectIndex( IDC_ADAPTER_COMBO, 0 );

	// settings to the UI
	mDlg->getSlider( GID_SLD_GFX_DETAIL )->setValue( mAppSettings.gfxDetail );
	mDlg->getCheckBox( GID_CHK_LEVELCACHE )->setChecked( mAppSettings.cacheLevels );
}


// --------------------------------------------------------------------------


void CDemoSettingsDialog::comboBoxAdd( int id, const void* pData, const TCHAR* pstrDesc )
{
	CUIComboBox* c = mDlg->getComboBox( id );
	assert( c );
	c->addItem( pstrDesc, pData, false );
}

void CDemoSettingsDialog::comboBoxSelect( int id, const void* pData )
{
	CUIComboBox* c = mDlg->getComboBox( id );
	assert( c );
	c->setSelectedByData( pData );
}

void CDemoSettingsDialog::comboBoxSelectIndex( int id, int index )
{
	CUIComboBox* c = mDlg->getComboBox( id );
	assert( c );
	c->setSelectedByIndex( index );
}

const void* CDemoSettingsDialog::comboBoxSelected( int id )
{
	CUIComboBox* c = mDlg->getComboBox( id );
	assert( c );
	return c->getSelectedData();
}

bool CDemoSettingsDialog::comboBoxSomethingSelected( int id )
{
	CUIComboBox* c = mDlg->getComboBox( id );
	assert( c );
	return c->getSelectedItem() ? true : false;
}

int CDemoSettingsDialog::comboBoxCount( int id )
{
	CUIComboBox* c = mDlg->getComboBox( id );
	assert( c );
	return c->getItemCount();
}

void CDemoSettingsDialog::comboBoxClear( int id )
{
	CUIComboBox* c = mDlg->getComboBox( id );
	assert( c );
	c->removeAllItems();
}

bool CDemoSettingsDialog::comboBoxContainsText( int id, const TCHAR* pstrText )
{
	CUIComboBox* c = mDlg->getComboBox( id );
	assert( c );
	return c->containsItem( pstrText );
}

void CDemoSettingsDialog::enableWindow( int id, bool enabled )
{
	CUIControl* c = mDlg->getControl( id );
	assert( c );
	c->setEnabled( enabled );
}

bool CDemoSettingsDialog::isChecked( int id )
{
	CUICheckBox* c = mDlg->getCheckBox( id );
	assert( c );
	return c->isChecked();
}

void CDemoSettingsDialog::checkButton( int id, bool checked )
{
	CUICheckBox* c = mDlg->getCheckBox( id );
	assert( c );
	c->setChecked( checked );
}

void CALLBACK CDemoSettingsDialog::dialogCallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
	CDemoSettingsDialog* inst = mSingleInstance;

	switch( evt ) {
	case UIEVENT_COMBOBOX_SELECTION_CHANGED:
		switch( ctrlID ) {
		case IDC_ADAPTER_COMBO:	inst->adapterChanged(); break;
		case IDC_DEVICE_COMBO:	inst->deviceChanged(); break;
		case IDC_ADAPTERFORMAT_COMBO:	inst->adapterFormatChanged(); break;
		case IDC_RESOLUTION_COMBO:		inst->resolutionChanged(); break;
		case IDC_REFRESHRATE_COMBO:		inst->refreshRateChanged(); break;
		case IDC_BACKBUFFERFORMAT_COMBO:			inst->backBufferFormatChanged(); break;
		case IDC_DEPTHSTENCILBUFFERFORMAT_COMBO:	inst->depthStencilBufferChanged(); break;
		case IDC_MULTISAMPLE_COMBO:			inst->multiSampleTypeChanged(); break;
		case IDC_MULTISAMPLE_QUALITY_COMBO:	inst->multiSampleQualityChanged(); break;
		case IDC_VERTEXPROCESSING_COMBO:	inst->vertexProcessingChanged(); break;
		}
		break;

	case UIEVENT_CHECKBOX_CHANGED:
		switch( ctrlID ) {
		case IDC_CHK_VSYNC:			inst->vsyncChanged(); break;
		case IDC_CHK_FULLSCREEN:	inst->windowedFullscreenChanged(); break;
		}
		break;

	case UIEVENT_BUTTON_CLICKED:
		switch( ctrlID ) {
		case IDOK:
			assert( inst->mState == STATE_ACTIVE );
			inst->mState = STATE_OK;
			inst->mAppSettings.gfxDetail = inst->mDlg->getSlider(GID_SLD_GFX_DETAIL)->getValue();
			inst->mAppSettings.cacheLevels = inst->mDlg->getCheckBox(GID_CHK_LEVELCACHE)->isChecked();
			break;
		case IDCANCEL:
			assert( inst->mState == STATE_ACTIVE );
			inst->mState = STATE_CANCEL;
			break;
		}
		break;
	}
}



// --------------------------------------------------------------------------


CDemoHelpDialog* CDemoHelpDialog::mSingleInstance = 0;




CDemoHelpDialog::CDemoHelpDialog()
:	mActive( false )
{
	assert( !mSingleInstance );
	mSingleInstance = this;

	//
	// UI
	
	mDlg = new CUIDialog();
	mDlg->enableNonUserEvents( true );
	mDlg->setCallback( dialogCallback );
	mDlg->setBackgroundColors( 0xA0303030 );
	mDlg->setLocation( 120, 70 );
	mDlg->setSize( 420, 340 );
	mDlg->setFont( 1, "Arial", 22, 50 );
	mDlg->setFont( 2, "Arial", 14, 50 );

	int yline;
	const float HC = 18;
	const float DYLINE = HC+2;

	CUIStatic* lab;

	const int xcol = 30;
	yline = 10;
	mDlg->addStatic( 0, "Controls", xcol-10, yline, 200, 22, false, &lab );
	lab->getElement(0)->setFont( 1, false, DT_LEFT | DT_VCENTER );

	mDlg->addStatic( 0,
		"Arrows\n"
		"Ctrl+Arrows\n"
		//"Space\n"
		"M\n"
		"N\n"
		"S\n"
		"E\n"
		"1/2\n"
		//"</>\n"
		"PgUp/PgDown\n"
		"Home/End\n"
		"Esc\n"
		, xcol+10, yline+=24, 70, HC*10, false, &lab );
	lab->getElement(0)->setFont( 0, false, DT_LEFT | DT_TOP );
	
	mDlg->addStatic( 0,
		"move & turn\n"
		"strafe\n"
		//"togge play/pause\n"
		"toggle megamap/3rdperson\n"
		"toggle minimap\n"
		"toggle game stats\n"
		"toggle selected entity stats\n"
		"select player 1/2 AI\n"
		//"scroll time backward/forward\n"
		"zoom in/out (megamap mode)\n"
		"tilt up/down (megamap mode)\n"
		"exit\n"
		, xcol+80, yline, 150, HC*10, false, &lab );
	lab->getElement(0)->setFont( 0, false, DT_LEFT | DT_TOP );
	
	mDlg->addStatic( 0, "Just in case: the keys are for QWERTY keyboard.", xcol, yline+=HC*7, 400, HC );
	mDlg->addStatic( 0, "Of course, most of the tasks can be controlled with the GUI!", xcol, yline+=HC-4, 400, HC );

	mDlg->addStatic( 0, "Credits", xcol-10, yline+=HC+HC/2, 200, 18, false, &lab );
	lab->getElement(0)->setFont( 1, false, DT_LEFT | DT_VCENTER );

	mDlg->addStatic( 0,
		"This 3D player (code/engine/artwork):\n"
		"        Aras Pranckevicius and Paulius Liekis [www.nesnausk.org]", xcol, yline+=24, 400, HC*5, false, &lab );
	lab->getElement(0)->setFont( 0, false, DT_LEFT | DT_TOP );
	mDlg->addStatic( 0,
		"Everything else (ideas, SDK, game logic, testing etc.):\n"
		"        Thomas Lucchini, Richard Clark, Benjamin Gauthey,\n"
		"        Cyril du Bois de Maquille, Vincent Lascaux"
		, xcol, yline+=2*14, 400, HC*5, false, &lab );
	lab->getElement(0)->setFont( 0, false, DT_LEFT | DT_TOP );
	
	// buttons
	mDlg->addButton( IDOK, "Close", 340, mDlg->getHeight()-35, 58, 20 );
}

CDemoHelpDialog::~CDemoHelpDialog()
{
	delete mDlg;
}


void CDemoHelpDialog::showDialog()
{
	assert( !mActive );
	mActive = true;
}


void CALLBACK CDemoHelpDialog::dialogCallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
	if( evt == UIEVENT_BUTTON_CLICKED && ctrlID == IDOK ) {
		CDemoHelpDialog* inst = mSingleInstance;
		assert( inst->mActive == true );
		inst->mActive = false;
	}
}
