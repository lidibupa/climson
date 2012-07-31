/**
 * Copyright 2012, Willam Dignazio
 * 
 * This file is part of climson.
 *
 * climson is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * climson is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with climson.  If not, see <http://www.gnu.org/licenses/>.kh
 *
 */

/*
 * =====================================================================================
 *
 *       Filename:  sms.c
 *
 *    Description:  SMS test app
 *
 *        Version:  0.1.2
 *        Created:  07/28/2012 06:55:31 PM
 *       Revision:  none
 *       Compiler:  clang
 *
 *         Author:  William Dignazio (slackwill), slackwill@csh.rit.edu
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <unistd.h> 
#include <pwd.h> 
#include <argp.h> 
#include <twilio.h> 

/* TODO: Change to makefile version macro */ 
const char *version = 
	"climson-v0.1.2"; 
const char *maintainer = 
	"<slackwill@csh.rit.edu>"; 
const char doc[] = 
	"\nclimson command line utility --- a sms utility to send texts from cli.\n"
	"In order to use this utility, you need to have an ~/.smsrc file.\n" 
	"This is simply done by making a .smsrc file with three lines in your home dir.\n"
	"	1. Your twilio SID\n"
	" 	2. Your twilio Token\n" 
	"	3. Your twilio phone #\n";

static char args_doc[] = "'<TO#>' '<TXT>'";

static struct argp_option options[] = { 
  	{"Recipient",'r',"Recipient",0,"'Recipients Number'"}, 
	{"Text", 't', "Text", 0, "'Text Body'"}, 
	{0}
};

/* For every argument added to the options struct, 
 * you need to add it's pointer/variable here */ 
struct arguments { 
	char *recipient; 
	char *text; 
};

/* Grab/Parse arguments 
 * Both passing in arguments, or directly specifying 
 * the recipient and text body. If one parameter is 
 * passed in, it will be recipient. If two are passed
 * in they will be recipient and text body respectively.
 * An alternative that will override the arguments is 
 * the -r and -t options, which are optional and 
 * specify the recipient and text body respectively.
 */
static int
parse_opt (int key, char *arg, struct argp_state *state) { 
	struct arguments *arguments = state->input; 
	switch(key) { 
	  	case 'r': 
		  	arguments->recipient = arg; 
			break; 
		case 't': 
			arguments->text = arg; 
			break; 
		case ARGP_KEY_ARG: 
			if(state->arg_num >= 2)
				argp_usage(state);
			else 
			  	switch(state->arg_num) {
			  		case 0: 
			  			arguments->recipient = arg;
						break;
			  		case 1: 
			  			arguments->text = arg;
						break;
			  	}
			break; 
		default: 
			return ARGP_ERR_UNKNOWN; 
	}
	return 0; 
}

static struct argp argp = {options, parse_opt, args_doc, doc}; 

/* This is a simple sms utility for sending out text messages 
 * through the command line. You need to have a file in your 
 * home directory with three lines, one for sid, token, and 
 * sending phone number, in that order. The text will be 
 * charged from your climson account. 
 */
int main(int argc, char *argv[]) { 
	
  	struct arguments arguments; 
	arguments.recipient = NULL; 
	arguments.text = NULL; 

	argp_parse(&argp, argc, argv, 0, 0, &arguments); 

	char *sid; 
	char *token;
	char *from_number; 
	char text[160]; 
	char number[20]; 
	
	struct passwd *pw = getpwuid(getuid());
	char path[strlen(pw->pw_dir)+strlen("/.smsrc")]; 
	sprintf(path, "%s%s", pw->pw_dir, "/.smsrc"); 
	FILE *fp = fopen(path, "r"); 
	if(fp == NULL) { 
		printf("~/.smsrc does not exist.\n"); 
		printf("Read --help for more information\n"); 
		exit(1);
	}

	char buffer[50]; 
	int stage; 
	for(stage=0; fgets(buffer, 80, fp); stage++) { 
	  	char *newseek; 
	  	switch(stage) { 
		  	case 0: // SID
			  	sid = malloc(strlen(buffer));
				sprintf(sid, "%s", buffer);
				if((newseek = strchr(sid, '\n')))
				  	*newseek = '\0';
				//printf("Read sid: %s\n", sid);
				break; 
			case 1: // Token
				token = malloc(strlen(buffer)); 
				sprintf(token, "%s", buffer); 
				if((newseek = strchr(token, '\n')))
				  	*newseek = '\0';
				//printf("Read token: %s\n", token); 
				break;
			case 2: // Sending phone number
				from_number = malloc(strlen(buffer)); 
				sprintf(from_number, "%s", buffer);
				if((newseek = strchr(from_number, '\n')))
				  	*newseek = '\0'; 
				//printf("Read number: %s\n", buffer); 
				break;
		}
		/* Zero out the buffer for the next run. */ 
		bzero(buffer, sizeof(buffer)); 
	}

	/* Initialize the C twilio library */
  	init_twilio_api(sid, token); 

	if(!arguments.recipient) {
		printf("To: "); 
		if(fgets(number, sizeof(number), stdin) != NULL) { 
			char *newline = strchr(number, '\n'); 
			if(newline != NULL) { 
				*newline = '\0'; 
			}
		}
	} else { // User has passed in an argument for recipient
	  	strcpy(number, arguments.recipient);
	}

	if(!arguments.text) {
		printf("Text: "); 
		if(fgets(text, sizeof(text), stdin) != NULL) { 
			char *newline = strchr(text, '\n'); 
			if(newline != NULL) { 
				*newline = '\0'; 
			}
		}
	} else { // User had passed in an argument for text
	  	strcpy(text, arguments.text); 
	}

	/* Twilio only accepts encoded numbers, maybe later 
	 * I will make an encoded number a structure itself. 
	 * That way you'd have to TRY to screw up your 
	 * numbers. 
	 */
	char *number_encoded; 
	html_encode(number, &number_encoded); 
	//printf("Encoded To Number: %s\n", number_encoded); 

	char *from_encoded; 
	html_encode(from_number, &from_encoded); 
	//printf("From Encoded: %s\n", from_encoded); 

	char *encoded_text; 
	html_encode(text, &encoded_text); 
	//printf("Text Encoded: %s\n", encoded_text); 

	/* The api makes it easy, the arguments, and user input 
	 * are the dirty bits. Go figure. */ 
	post_sms(from_encoded, number_encoded, encoded_text);

	free(token); 
	free(sid); 
	free(number_encoded); 
	free(from_encoded); 
	free(encoded_text);

	printf("\n"); //TODO: Remove when json is added in

    return 0; 
}

