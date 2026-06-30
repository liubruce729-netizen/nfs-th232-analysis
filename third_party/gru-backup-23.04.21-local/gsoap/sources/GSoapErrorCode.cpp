
// File : GSoapErrorCode.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// List of error code
//
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/
#include "GSoapErrorCode.h"
#include "stdsoap2.h"
#include "GSoapErrorCodef.h"
#include "stdio.h"

void message_code(int rcode) {

	message_code(rcode,0);
};

char* message_code(int rcode,char* message) {

	switch (rcode) {
	case SOAP_OK: {
		if (message) sprintf(message, "OK");
		else printf("OK\n");
		break;
	}
	case RETOUR_SOAP_NOT_OK: {
		if (message)sprintf(message, "Not OK");
		else printf("Not OK");
		break;
	}
	case RETOUR_SOAP_OK: {
		if (message)sprintf(message, "OK");
		else printf("OK");
		break;
	}
	case RETOUR_SOAP_NOTREADY: {
		if (message)sprintf(message, "Not Ready");
		else printf("Not Ready");
		break;
	}
	case RETOUR_SOAP_TIMEOUT: {
		if (message)sprintf(message, "TimeOut");
		else printf("TimeOut");
		break;
	}
	case RETOUR_SOAP_NOT_GRU_COMMAND: {
		if (message)sprintf(message, "Command not recognized");
		else printf("Command not recognized");
		break;
	}
	default: {
		if (message)sprintf(message, "Return code not defined %d ", rcode);
		else printf("Return code not defined %d ", rcode);
		break;
	}

	}
	return message;
}

