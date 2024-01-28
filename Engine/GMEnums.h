//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEnums.h
/// @brief		Galaxy-Music Engine - GMEnums
/// @version	1.0
/// @author		LiuTao
/// @date		2021.09.04
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace GM
{ 
	/*************************************************************************
	 Enums
	*************************************************************************/

	/*!
	 *  @enum EGMMouseButton
	 *  @brief 鼠标按键
	 */
	enum EGMMouseButton
	{
		EGM_MB_LBUTTON				= 0x0001,	//!< 左键
		EGM_MB_MBUTTON				= 0x0002,	//!< 中键
		EGM_MB_RBUTTON				= 0x0004,	//!< 右键
		EGM_MB_MAX
	};

	/*!
	 *  @enum EGMKeyCode
	 *  @brief 按键代码
	 */
	enum EGMKeyCode
	{
		EGM_KC_Space				= 0x20,		//!< 空格

		EGM_KC_0					= '0',
		EGM_KC_1					= '1',
		EGM_KC_2					= '2',
		EGM_KC_3					= '3',
		EGM_KC_4					= '4',
		EGM_KC_5					= '5',
		EGM_KC_6					= '6',
		EGM_KC_7					= '7',
		EGM_KC_8					= '8',
		EGM_KC_9					= '9',
		EGM_KC_A					= 'a',
		EGM_KC_B					= 'b',
		EGM_KC_C					= 'c',
		EGM_KC_D					= 'd',
		EGM_KC_E					= 'e',
		EGM_KC_F					= 'f',
		EGM_KC_G					= 'g',
		EGM_KC_H					= 'h',
		EGM_KC_I					= 'i',
		EGM_KC_J					= 'j',
		EGM_KC_K					= 'k',
		EGM_KC_L					= 'l',
		EGM_KC_M					= 'm',
		EGM_KC_N					= 'n',
		EGM_KC_O					= 'o',
		EGM_KC_P					= 'p',
		EGM_KC_Q					= 'q',
		EGM_KC_R					= 'r',
		EGM_KC_S					= 's',
		EGM_KC_T					= 't',
		EGM_KC_U					= 'u',
		EGM_KC_V					= 'v',
		EGM_KC_W					= 'w',
		EGM_KC_X					= 'x',
		EGM_KC_Y					= 'y',
		EGM_KC_Z					= 'z',

		EGM_KC_Exclaim				= 0x21,		//!< !
		EGM_KC_Quotedbl				= 0x22,
		EGM_KC_Hash					= 0x23,
		EGM_KC_Dollar				= 0x24,		//!< $
		EGM_KC_Ampersand			= 0x26,		//!< &
		EGM_KC_Quote				= 0x27,
		EGM_KC_Leftparen			= 0x28,
		EGM_KC_Rightparen			= 0x29,
		EGM_KC_Asterisk				= 0x2A,		//!< *
		EGM_KC_Plus					= 0x2B,		//!< +
		EGM_KC_Comma				= 0x2C,
		EGM_KC_Minus				= 0x2D,		//!< -
		EGM_KC_Period				= 0x2E,
		EGM_KC_Slash				= 0x2F,
		EGM_KC_Colon				= 0x3A,		//!< :
		EGM_KC_Semicolon			= 0x3B,		//!< ;
		EGM_KC_Less					= 0x3C,		//!< <
		EGM_KC_Equals				= 0x3D,		//!< =
		EGM_KC_Greater				= 0x3E,		//!< >
		EGM_KC_Question				= 0x3F,		//!< ?
		EGM_KC_At					= 0x40,		//!< @
		EGM_KC_Leftbracket			= 0x5B,		//!< [
		EGM_KC_Backslash			= 0x5C,
		EGM_KC_Rightbracket			= 0x5D,		//!< ]
		EGM_KC_Caret				= 0x5E,
		EGM_KC_Underscore			= 0x5F,		//!< _
		EGM_KC_Backquote			= 0x60,

		EGM_KC_BackSpace			= 0xFF08,
		EGM_KC_Tab					= 0xFF09,
		EGM_KC_Linefeed				= 0xFF0A,        /* 换行(区别于回车) */
		EGM_KC_Clear				= 0xFF0B,
		EGM_KC_Return				= 0xFF0D,        /* 回车(区别于换行) */
		EGM_KC_Pause				= 0xFF13,
		EGM_KC_Scroll_Lock			= 0xFF14,
		EGM_KC_Sys_Req				= 0xFF15,
		EGM_KC_Escape				= 0xFF1B,
		EGM_KC_Delete				= 0xFFFF,

		EGM_KC_Home					= 0xFF50,
		EGM_KC_Left					= 0xFF51,
		EGM_KC_Up					= 0xFF52,
		EGM_KC_Right				= 0xFF53,
		EGM_KC_Down					= 0xFF54,
		EGM_KC_Prior				= 0xFF55,
		EGM_KC_Page_Up				= 0xFF55,
		EGM_KC_Next					= 0xFF56,
		EGM_KC_Page_Down			= 0xFF56,
		EGM_KC_End					= 0xFF57,
		EGM_KC_Begin				= 0xFF58,

		/* Misc Functions */
		EGM_KC_Select				= 0xFF60,        /* Select, mark */
		EGM_KC_Print				= 0xFF61,
		EGM_KC_Execute				= 0xFF62,        /* Execute, run, do */
		EGM_KC_Insert				= 0xFF63,        /* Insert, insert here */
		EGM_KC_Undo					= 0xFF65,        /* Undo, oops */
		EGM_KC_Redo					= 0xFF66,        /* redo, again */
		EGM_KC_Menu					= 0xFF67,        /* On Windows, this is VK_APPS, the context-menu key */
		EGM_KC_Find					= 0xFF68,        /* Find, search */
		EGM_KC_Cancel				= 0xFF69,        /* Cancel, stop, abort, exit */
		EGM_KC_Help					= 0xFF6A,        /* Help */
		EGM_KC_Break				= 0xFF6B,
		EGM_KC_Mode_switch			= 0xFF7E,        /* Character set switch */
		EGM_KC_Script_switch		= 0xFF7E,        /* Alias for mode_switch */
		EGM_KC_Num_Lock				= 0xFF7F,

		/* Keypad Functions, keypad numbers cleverly chosen to map to ascii */
		EGM_KC_KP_Space				= 0xFF80,        /* space */
		EGM_KC_KP_Tab				= 0xFF89,
		EGM_KC_KP_Enter				= 0xFF8D,        /* enter */
		EGM_KC_KP_F1				= 0xFF91,        /* PF1, KP_A, ... */
		EGM_KC_KP_F2				= 0xFF92,
		EGM_KC_KP_F3				= 0xFF93,
		EGM_KC_KP_F4				= 0xFF94,
		EGM_KC_KP_Home				= 0xFF95,
		EGM_KC_KP_Left				= 0xFF96,
		EGM_KC_KP_Up				= 0xFF97,
		EGM_KC_KP_Right				= 0xFF98,
		EGM_KC_KP_Down				= 0xFF99,
		EGM_KC_KP_Prior				= 0xFF9A,
		EGM_KC_KP_Page_Up			= 0xFF9A,
		EGM_KC_KP_Next				= 0xFF9B,
		EGM_KC_KP_Page_Down			= 0xFF9B,
		EGM_KC_KP_End				= 0xFF9C,
		EGM_KC_KP_Begin				= 0xFF9D,
		EGM_KC_KP_Insert			= 0xFF9E,
		EGM_KC_KP_Delete			= 0xFF9F,
		EGM_KC_KP_Equal				= 0xFFBD,        /* equals */
		EGM_KC_KP_Multiply			= 0xFFAA,
		EGM_KC_KP_Add				= 0xFFAB,
		EGM_KC_KP_Separator			= 0xFFAC,        /* separator, often comma */
		EGM_KC_KP_Subtract			= 0xFFAD,
		EGM_KC_KP_Decimal			= 0xFFAE,
		EGM_KC_KP_Divide			= 0xFFAF,
		EGM_KC_KP_0					= 0xFFB0,
		EGM_KC_KP_1					= 0xFFB1,
		EGM_KC_KP_2					= 0xFFB2,
		EGM_KC_KP_3					= 0xFFB3,
		EGM_KC_KP_4					= 0xFFB4,
		EGM_KC_KP_5					= 0xFFB5,
		EGM_KC_KP_6					= 0xFFB6,
		EGM_KC_KP_7					= 0xFFB7,
		EGM_KC_KP_8					= 0xFFB8,
		EGM_KC_KP_9					= 0xFFB9,

		/*
		 * Auxiliary Functions; note the duplicate definitions for left and right
		 * function keys;  Sun keyboards and a few other manufactures have such
		 * function key groups on the left and/or right sides of the keyboard.
		 * We've not found a keyboard with more than 35 function keys total.
		 */
		EGM_KC_F1					= 0xFFBE,
		EGM_KC_F2					= 0xFFBF,
		EGM_KC_F3					= 0xFFC0,
		EGM_KC_F4					= 0xFFC1,
		EGM_KC_F5					= 0xFFC2,
		EGM_KC_F6					= 0xFFC3,
		EGM_KC_F7					= 0xFFC4,
		EGM_KC_F8					= 0xFFC5,
		EGM_KC_F9					= 0xFFC6,
		EGM_KC_F10					= 0xFFC7,
		EGM_KC_F11					= 0xFFC8,
		EGM_KC_F12					= 0xFFC9,
		EGM_KC_F13					= 0xFFCA,
		EGM_KC_F14					= 0xFFCB,
		EGM_KC_F15					= 0xFFCC,
		EGM_KC_F16					= 0xFFCD,
		EGM_KC_F17					= 0xFFCE,
		EGM_KC_F18					= 0xFFCF,
		EGM_KC_F19					= 0xFFD0,
		EGM_KC_F20					= 0xFFD1,
		EGM_KC_F21					= 0xFFD2,
		EGM_KC_F22					= 0xFFD3,
		EGM_KC_F23					= 0xFFD4,
		EGM_KC_F24					= 0xFFD5,
		EGM_KC_F25					= 0xFFD6,
		EGM_KC_F26					= 0xFFD7,
		EGM_KC_F27					= 0xFFD8,
		EGM_KC_F28					= 0xFFD9,
		EGM_KC_F29					= 0xFFDA,
		EGM_KC_F30					= 0xFFDB,
		EGM_KC_F31					= 0xFFDC,
		EGM_KC_F32					= 0xFFDD,
		EGM_KC_F33					= 0xFFDE,
		EGM_KC_F34					= 0xFFDF,
		EGM_KC_F35					= 0xFFE0,

		/* Modifiers */
		EGM_KC_Shift_L				= 0xFFE1,        /* Left shift */
		EGM_KC_Shift_R				= 0xFFE2,        /* Right shift */
		EGM_KC_Control_L			= 0xFFE3,        /* Left control */
		EGM_KC_Control_R			= 0xFFE4,        /* Right control */
		EGM_KC_Caps_Lock			= 0xFFE5,        /* Caps lock */
		EGM_KC_Shift_Lock			= 0xFFE6,        /* Shift lock */
		EGM_KC_Meta_L				= 0xFFE7,        /* Left meta */
		EGM_KC_Meta_R				= 0xFFE8,        /* Right meta */
		EGM_KC_Alt_L				= 0xFFE9,        /* Left alt */
		EGM_KC_Alt_R				= 0xFFEA,        /* Right alt */
		EGM_KC_Super_L				= 0xFFEB,        /* Left super */
		EGM_KC_Super_R				= 0xFFEC,        /* Right super */
		EGM_KC_Hyper_L				= 0xFFED,        /* Left hyper */
		EGM_KC_Hyper_R				= 0xFFEE,        /* Right hyper */
		EGM_KC_Max
	};

	/**
	* 画面渲染品质
	*/
	enum EGMRENDER_QUALITY
	{
		/** 低画质 */
		EGMRENDER_LOW,
		/** 普通画质 */
		EGMRENDER_NORMAL,
		/** 高画质 */
		EGMRENDER_HIGH,
	};
}	// GM
