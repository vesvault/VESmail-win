#pragma once

#include "resource.h"

#define VMW_CLASS	L"VESMAILWIN"

#define VMW_ID_BTN_PROF		0x8100
#define VMW_ID_BTN_UPROF	0x8101

#define VMW_MSG_SIGNAL		0x80e0
#define VMW_MSG_NOTIFY		0x80e1
#define VMW_MSG_USCROLL		0x80e2
#define VMW_MSG_UHOVER		0x80e3
#define VMW_MSG_SNIFAUTH	0x80e4

#define VMW_ID_TIMER		0x80f0

#define	VMWF_ACTIVE	0x01
#define VMWF_BLINK	0x10
#define VMWF_SKIP	0x20

#define VMW_PAD_L	8
#define VMW_PAD_T	8
#define VMW_PAD_B	8
#define VMW_PAD_R	8
#define VMW_LOGO_Y	8
#define VMW_BTN_W	120
#define VMW_BTN_Y	56
#define VMW_BTN_H	16
#define	VMW_VER_Y	4
#define	VMW_VER_R	0
#define	VMW_VER_H	16

#define VMW_USER_W	208
#define VMW_USER_ST_X	2
#define VMW_USER_ST_Y	80
#define VMW_USER_ST_H	10
#define VMW_USER_ST_R	0
#define VMW_USER_H	96
#define VMW_USER_PAD_L	2
#define VMW_USER_PAD_T	2
#define VMW_USER_PAD_R	2
#define VMW_USER_PAD_B	2
#define VMW_USER_LOGIN_X	13
#define VMW_USER_LOGIN_H	10
#define VMW_USER_LOGIN_R	10
#define VMW_USER_PROF_DX	-8
#define VMW_USER_PROF_Y	1
#define VMW_USER_STAT_X	3
#define VMW_USER_STAT_Y	2
#define	VMW_USER_SPN_Y	24
#define VMW_USER_PMT_Y	24
#define VMW_USER_PMT_H	80

extern HINSTANCE vmInst;
