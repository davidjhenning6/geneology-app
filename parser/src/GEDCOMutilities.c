/*****************************************************************************
 * CIS2750 W2018
 * Assignment 1
 * David Henning 0921760
 *****************************************************************************/


#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"


//checks file validity returns 0 on success and -1 on failure
int fileValidity(char* fileName){
	char* fileType = ".ged";
	char* ptr;
	int result;
	if(fileName == NULL || strlen(fileName) == 0 || strstr(fileName, fileType) == NULL){
		return -1;
	}
	ptr=strstr(fileName, fileType);
	if(ptr[4]!='\0'){
		return -1;
	}

	result = access(fileName, R_OK);

	if(result != 0){
		return -1;
	}

	return 0;
}

//checks gedcom object validity returns file pointer to original position upon returning and sets a 
/*void gedcomValidity(FILE *fp, GEDCOMerror* gedErr){
	fpos_t pos;
	fgetpos(fp, &pos);
	//char current = '\0';
	//current=
	//while(fgetc()){
	
	//}
	
	fsetpos(fp, &pos);
	return;
	
}*/

GEDCOMerror lineParse(FILE* fp, MyLine* myline, int lineNum){
	char current = '\0';
	int charCounter = 0;
	int structCounter = 0;
	int levelCounter = 0;
	int tagCounter = 0;
	int valueCounter = 0;
	char prevLineLevel[255];
	strcpy(prevLineLevel, myline->level);
	if(feof(fp)){
		GEDCOMerror ret;
      	ret.type = OK;
      	ret.line = lineNum;
		return ret;
    }
	
	memset(myline->tag,0, 255);
	memset(myline->level,0, 255);
	memset(myline->value,0, 1000);
	
	while(current != '\n' && current != '\r'){
		current = fgetc(fp);
		if(feof(fp)){
			break ;
      	}
		//printf("%c", current);
		//printf("%d\n", charCounter);
		//printf("\nstructCounter == |%d| \n", structCounter);
      	switch(structCounter){
      		case 0:
      			//get extraneous white space at front of line
      			while(current == ' ' && charCounter == 0 ){
      				current = fgetc(fp);
      			}
      			//scanned in number at front of line
      			if(isdigit(current)){
      				if(levelCounter == 2){
      					//return error
      					//printf("\nhi1\n");
      					GEDCOMerror ret;
      					ret.type = OTHER_ERROR;
      					ret.line = lineNum;
      					return ret;
      				}
      				myline->level[levelCounter] = current;
      				levelCounter = levelCounter + 1;
      				charCounter = charCounter + 1;
      			//if character or space found and char counter isn't
      			}
      			else if(current != ' ' && charCounter != 0){
      					//return error
      					//printf("\nhi2\n");
      					GEDCOMerror ret;
      					ret.type = OTHER_ERROR;
      					ret.line = lineNum;
      					return ret;
      			}
      			//if space found after number
      			if(current == ' '){
      				if(myline->level[0] == '0' && levelCounter != 1){
      					//return error for ex '01' level 
      					//printf("\nhi3\n");
      					GEDCOMerror ret;
      					ret.type = OTHER_ERROR;
      					ret.line = lineNum;
      					return ret;
      				}
      				if(myline->level[0] != '0' && lineNum == 0 ){
      					//return error if files first line has a level thats not 0
      					//printf("\nhi4\n");
      					GEDCOMerror ret;
      					ret.type = INV_HEADER;
      					ret.line = lineNum;
      					return ret;
      				}
      				if(myline->level[0] != '0' && lineNum !=0 ){
      					if(levelCheck(myline, prevLineLevel) == -1){//check previous line to ensure valid
      						//if fail return error
      						GEDCOMerror ret;
      						ret.type = OTHER_ERROR;
      						ret.line = lineNum;
      						return ret;
      					
      					}
      					//if pass increment structCounter
      					
      				}
      				structCounter = structCounter + 1;
      				charCounter = charCounter + 1;
      			}//else if(isalpha(current))
      			break;
      		case 1:
      			//printf("\ncase1\n");
      			while(current != ' ' && current != '\n' && current != '\r' && !feof(fp)){
      				//printf("%c\n", current);
      				myline->tag[tagCounter] = current;
      				tagCounter = tagCounter + 1;
      				charCounter = charCounter + 1;
      				current = fgetc(fp);
      			}
      			structCounter = structCounter + 1;
      			if(current == '\n' || current == '\r'){
      				//GEDCOMerror ret;
      				//ret.type = OK;
      				//ret.line = lineNum;
      				//return ret;
      			}
      			if(current == ' '){
      				//printf("%c\n", current);
      			}
      			break;
      		case 2:
      			//printf("\ncase2\n");
      			while(current != '\n' && current != '\r' && !feof(fp)){
      				//printf("%c\n", current);
      				myline->value[valueCounter] = current;
      				valueCounter = valueCounter + 1;
      				current = fgetc(fp);
      				charCounter = charCounter + 1;
      				
      			}
      			if(current == '\n' || current == '\r'){
      				//GEDCOMerror ret;
      				//ret.type = OK;
      				//ret.line = lineNum;
      				//return ret;
      			}
      			charCounter = charCounter + 1;
      			break;
      		default:
      			break;
      	}
	}
	if(!feof(fp)){
		checkForNewLine(fp);
	}
	//printf("\nhi5\n");
	if(charCounter > 255){
		GEDCOMerror ret;
      	ret.type = OTHER_ERROR;
      	ret.line = lineNum;
      	return ret;
	}
	
	//call check next line
	if(!feof(fp)){
		checkCONXNextLine(fp, myline, lineNum, current);
	}
	GEDCOMerror ret;
    ret.type = OK;
	ret.line = lineNum;
	//printf("charCount:|%d|\n", charCounter);
    return ret;
}

int levelCheck(MyLine* myline, char prevLineLevel[]){
	
	int cur = 0;
	int prev = 0;
	
	cur = atoi(myline->level);
	prev = atoi(prevLineLevel);
	
	if(cur > prev + 1){
		return -1;
	}
	return 0;
}


GEDCOMerror checkCONXNextLine(FILE *fp, MyLine* myline, int lineNum, char current){
	fpos_t pos;
	fgetpos(fp, &pos);
	int levelLen, i;
	char preScan;
	levelLen = strlen(myline->level);
	
	if(strcmp(myline->level, "9") == 0){
		levelLen = 2;
	}
	
	//fgetpos(fp, &pos);
	levelLen = levelLen + 5;
	char temp[8];
	memset(temp, '\0', 8);
	for(i=0; i <= levelLen; i++){
		//if()
		temp[i] = fgetc(fp);
	}
	
	if(strstr(temp, "CONC") != NULL){
		//printf("CONC entered\n");
		int len = strlen(myline->value);
		preScan = fgetc(fp);
		while(preScan != '\n' && preScan != '\r' && !feof(fp)){
			myline->value[len] = preScan;
			len = len + 1;
			preScan = fgetc(fp);
		}
	}else if(strstr(temp, "CONT") != NULL){
		//printf("CONT entered\n");
		fsetpos(fp, &pos);
		int len = strlen(myline->value);
		myline->value[len] = current;
		len = len + 1;
		int i;
		for(i=0; i<= levelLen; i++){
			fgetc(fp);
		}
		preScan = fgetc(fp);
		while(preScan != '\n' && preScan != '\r' && !feof(fp)){
			myline->value[len] = preScan;
			len = len + 1;
			preScan = fgetc(fp);
		}
		
		
	}else{
		fsetpos(fp, &pos);
	}
	GEDCOMerror ret;
    ret.type = OK;
	ret.line = lineNum;
	return ret;
}


int checkForNewLine(FILE *fp){
	fpos_t pos;
	char check;
	fgetpos(fp, &pos);
	check = fgetc(fp);

	while((check == '\n' || check == '\r') && !feof(fp)){
		fgetpos(fp, &pos);
		check = fgetc(fp);
		
	}

	fsetpos(fp, &pos);
	return 1;
}


int checkNumberOfLines(FILE *fp){
	fpos_t pos;
	char check;
	int lines = 0;
	fgetpos(fp, &pos);
	

	while(!feof(fp))
	{
	  check = fgetc(fp);
	  if(check == '\n')
	  {
	    lines++;
	  }
	}
	fsetpos(fp, &pos);
	lines = lines + 1;
	return lines;
}

//returns 0 a non month found else returns number of month
int getMonth(char* month){
	int m1;
	if(strcmp(month, "JAN") == 0){
		m1 = 1;
	}else if(strcmp(month, "FEB") == 0){
		m1 = 2;
	}else if(strcmp(month, "MAR") == 0){
		m1 = 3;
	}else if(strcmp(month, "APR") == 0){
		m1 = 4;
	}else if(strcmp(month, "MAY") == 0){
		m1 = 5;
	}else if(strcmp(month, "JUN") == 0){
		m1 = 6;
	}else if(strcmp(month, "JUL") == 0){
		m1 = 7;
	}else if(strcmp(month, "AUG") == 0){
		m1 = 8;
	}else if(strcmp(month, "SEP") == 0){
		m1 = 9;
	}else if(strcmp(month, "OCT") == 0){
		m1 = 10;
	}else if(strcmp(month, "NOV") == 0){
		m1 = 11;
	}else if(strcmp(month, "DEC") == 0){
		m1 = 12;
	}else{
		m1 = 0;
	}

	return m1;
}


Header* initializeHeader(){
	Header *header = (Header*)malloc(sizeof(Header));
	memset(header->source,0,249);
	header->gedcVersion = 0;
	header->encoding = ASCII;
	header->submitter = NULL;
	header->otherFields = initializeList(printField, deleteField, compareFields);
	return header;
}

Individual* initializeIndividual(){
	Individual *individual = (Individual*)malloc(sizeof(Individual));
	individual->givenName = NULL;
	individual->surname = NULL;
	individual->events = initializeList(printEvent, deleteEvent, compareEvents);
	individual->families = initializeList(printFamily, deleteFamily, compareFamilies);
	individual->otherFields = initializeList(printField, deleteField, compareFields);
	return individual;
}

Family* initializeFamily(){
	Family *family = (Family*)malloc(sizeof(Family));
	family->wife = NULL;
	family->husband = NULL;
	family->children = initializeList(printIndividual, deleteIndividual, compareIndividuals);
	family->events = initializeList(printEvent, deleteEvent, compareEvents);
	family->otherFields = initializeList(printField, deleteField, compareFields);
	return family;
}

GEDCOMobject* initializeGEDCOM(){
	GEDCOMobject *obj = (GEDCOMobject*)malloc(sizeof(GEDCOMobject));
	obj->header = NULL;
	obj->families = initializeList(printFamily, deleteFamily, compareFamilies);
	obj->individuals = initializeList(printIndividual, deleteIndividual, compareIndividuals);
	obj->submitter = NULL;
	return obj;
}

Submitter* initializeSubmitter(){
	Submitter *submitter = (Submitter*)malloc(sizeof(Submitter));
	memset(submitter->submitterName,0,61);
	submitter->otherFields = initializeList(printField, deleteField, compareFields);
	return submitter;
}

Event* initializeEvent(){
	Event *event = (Event*)malloc(sizeof(Event));
	memset(event->type,0,5);
	event->date = NULL;
	event->place = NULL;
	event->otherFields = initializeList(printField, deleteField, compareFields);
	return event;
}

Field* initializefield(){
	Field *field = (Field*)malloc(sizeof(Field));
	field->tag = NULL;
	field->value = NULL;
	return field;
}



void getChildren(List fams, Individual* person, List* descend){
	//int count = 0; 
	//printf("child%s, %s\n", person->givenName, person->surname);
	//Individual* ret = initializeIndividual();
	ListIterator iter = createIterator(fams);
	Family* foundFam = (Family*)nextElement(&iter);
	while(foundFam != NULL){
		//printf("get children\n");
		if(person == NULL){
			//printf("person is NULL\n");
		}
		if(foundFam->husband == NULL){
			//printf("husband is null\n");

		}
		if(foundFam->wife == NULL){
			//printf("wife is NULL\n");

		}

		if(getLength(fams) == 1){
			if((foundFam->husband != NULL && person != foundFam->husband )|| (foundFam->wife != NULL && person != foundFam->wife)){
				return;
			}
		}


		if((foundFam->husband != NULL && compareIndividuals(person, foundFam->husband) == 0 )|| (foundFam->wife != NULL && compareIndividuals(person, foundFam->wife) == 0)){
			//printf("im stuck in the middle of the world\n\n");
			ListIterator kiderate = createIterator(foundFam->children);
			Individual* kid = (Individual*)nextElement(&kiderate);
			while(kid != NULL){
				//printf("kid%s, %s, %d\n", kid->givenName, kid->surname, count);
				Individual* myKid = copyPerson(kid);
				insertBack(descend, myKid);
				getChildren(kid->families, kid, descend);
				kid = (Individual*)nextElement(&kiderate);
			}
		}


		foundFam = (Family*)nextElement(&iter);
	}


	return;
}



Individual* copyPerson(Individual* person){
	Individual* myPerson = initializeIndividual();

	myPerson->givenName = (char*)malloc(sizeof(char)*strlen(person->givenName)+1);
	myPerson->surname = (char*)malloc(sizeof(char)*strlen(person->surname)+1);

	myPerson->families = person->families;

	strcpy(myPerson->givenName, person->givenName);
	strcpy(myPerson->surname, person->surname);

	return myPerson;
}

List* getNextGen(List* unSorted, List* currentGen){
	
	ListIterator iter = createIterator(*currentGen);
	Individual* parent = (Individual*)nextElement(&iter);

	List* LNextGen = (List*) malloc(sizeof(List));
	LNextGen -> printData = printIndividual;
	LNextGen -> deleteData = deleteIndividual;
	LNextGen -> compare = compareIndividuals;
	LNextGen -> head = NULL;
	LNextGen -> tail = NULL;
	LNextGen -> length = 0;

	while(parent != NULL){
		
		ListIterator kiderate = createIterator(*unSorted);
		Individual* kid = (Individual*)nextElement(&kiderate);
		
		while(kid != NULL){

			ListIterator fam = createIterator(kid->families);
			Family* kidFam = (Family*)nextElement(&fam);

			while(kidFam != NULL){

				if(((kidFam->husband != NULL && compareIndividuals(parent, kidFam->husband) == 0 )|| (kidFam->wife != NULL && compareIndividuals(parent, kidFam->wife) == 0)) && compareIndividuals(parent, kid) != 0){
					//printf("im stuck in the middle of the world\n\n");

					//printf("insert into new gen ---------->%s, %s\n", kid->givenName, parent->givenName);
					Individual* myKid = copyPerson(kid);
					insertSorted(LNextGen, myKid);
					
					
				}
				kidFam = (Family*)nextElement(&fam);
			}
			kid = (Individual*)nextElement(&kiderate);
		}
		parent = (Individual*)nextElement(&iter);
	}
	//insertFront(Descend, (void*)currentGen);
	//printf("getNextGen::  %s\n", iListToJSON(*LNextGen));
	return LNextGen;

}


char* writeHeader(GEDCOMobject* obj){

	char* string = NULL;
	char concat[200];
	memset(concat, '\0', 200);

	string = (char*)malloc(sizeof(char)*strlen("HEAD 0\n")+1);

	strcpy(string, "0 HEAD\n");



	string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 SOUR  \n") + strlen(obj->header->source) + 1));
	strcat(string, "1 SOUR ");
	strcat(string, obj->header->source);
	strcat(string, "\n");
	




	string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 GEDC\n") + strlen("2 VERS \n") + 20));
	strcat(string, "1 GEDC\n2 VERS ");
	sprintf(concat, "%.1f", obj->header->gedcVersion);
	strcat(string, concat);
	memset(concat, '\0', 200);


	string = realloc(string, sizeof(char)*( strlen(string) + strlen("\n2 FORM LINEAGE-LINKED\n") +1));
	strcat(string, "\n2 FORM LINEAGE-LINKED\n");

	if(obj->header->encoding == ANSEL){
		string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 CHAR ANSEL\n") +1));
		strcat(string, "1 CHAR ANSEL\n");
	}
	else if(obj->header->encoding == ASCII){
		string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 CHAR ASCII\n") +1));
		strcat(string, "1 CHAR ASCII\n");
	}
	else if(obj->header->encoding == UTF8){
		string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 CHAR UTF-8\n") +1));
		strcat(string, "1 CHAR UTF-8\n");
	}
	else if(obj->header->encoding == UNICODE){
		string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 CHAR UNICODE\n") +1));
		strcat(string, "1 CHAR UNICODE\n");
	}


	string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 SUBM @S1@\n") + 1));
	strcat(string, "1 SUBM @S1@\n");
	


	return string;
}

char* writeSubmitter(GEDCOMobject* obj){

	char* string = NULL;
	char concat[200];
	memset(concat, '\0', 200);

	string = (char*)malloc(sizeof(char)*strlen("0 @S1@ SUBM\n")+1);
	strcpy(string, "0 @S1@ SUBM\n");

	if(strlen(obj->submitter->submitterName) != 0){
		string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 NAME  \n") + strlen(obj->submitter->submitterName) + 1));
		strcat(string, "1 NAME ");
		strcat(string, obj->submitter->submitterName);
		strcat(string, "\n");
	}

	if(strlen(obj->submitter->address) != 0){
		string = realloc(string, sizeof(char)*( strlen(string) + strlen("1 ADDR  \n") + strlen(obj->submitter->address) + 1));
		strcat(string, "1 ADDR ");
		strcat(string, obj->submitter->address);
		strcat(string, "\n");
	}

	return string;
}

char* writeIndividual(Individual* indi, int iCount, writeFamCount fTracker[]){

	char* string = NULL;
	char concat[200];
	memset(concat, '\0', 200);
	int i;


	sprintf(concat, "%d", iCount);
	string = (char*)malloc(sizeof(char)*( strlen("0 @I@ INDI\n") + strlen(concat) + 2));
	strcpy(string, "0 @I");
	strcat(string, concat);
	memset(concat, '\0', 200);
	strcat(string, "@ INDI\n");

	string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("1 NAME  //\n") + strlen(indi->givenName) + strlen(indi->surname) + 3));
	strcat(string, "1 NAME ");
	strcat(string, indi->givenName);
	strcat(string, " /");
	strcat(string, indi->surname);
	strcat(string, "/\n");

	string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("1 GIVN  //\n") + strlen(indi->givenName) + 3));
	if(strlen(indi->givenName) != 0 ){
		strcat(string, "1 GIVN ");
		strcat(string, indi->givenName);
		strcat(string, "\n");
	}
	
	
	string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("1 SURN  //\n") + strlen(indi->surname) + 3));
	strcat(string, "1 SURN ");
	strcat(string, indi->surname);
	strcat(string, "\n");

	ListIterator events = createIterator(indi->events);
	Event* event = (Event*)nextElement(&events);
	while(event != NULL){

		if(event->date != NULL && event->place != NULL){
			string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("122 DATE PLAC \n") + strlen(event->type) + strlen(event->date) + strlen(event->place) + 20));
			strcat(string, "1 ");
			strcat(string, event->type);
			strcat(string, "\n");
			if(strlen(event->date) != 0){
				strcat(string, "2 DATE ");
				strcat(string, event->date);
				strcat(string, "\n");
			}
			if(strlen(event->place) != 0){
				strcat(string, "2 PLAC ");
				strcat(string, event->place);
				strcat(string, "\n");
			}
		}
		else if(event->date == NULL && event->place != NULL){
			string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("122 DATE PLAC \n") + strlen(event->type) + strlen(event->place) + 20));
			strcat(string, "1 ");
			strcat(string, event->type);
			strcat(string, "\n");
			if(strlen(event->place) != 0){
				strcat(string, "2 PLAC ");
				strcat(string, event->place);
				strcat(string, "\n");
			}
		}
		else if(event->date != NULL && event->place == NULL){
			string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("122 DATE PLAC \n") + strlen(event->type) + strlen(event->date) + 20));
			strcat(string, "1 ");
			strcat(string, event->type);
			strcat(string, "\n");
			if(strlen(event->date) != 0){
				strcat(string, "2 DATE ");
				strcat(string, event->date);
				strcat(string, "\n");
			}
		}
		event = (Event*)nextElement(&events);		
	}



	ListIterator iter = createIterator(indi->otherFields);
	Field* field = (Field*)nextElement(&iter);
	while(field != NULL){

		string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("1  \n") + strlen(field->tag) + strlen(field->value) + 3));
		strcat(string, "1 ");
		strcat(string, field->tag);
		strcat(string, " ");
		strcat(string, field->value);
		strcat(string, "\n");
		field = (Field*)nextElement(&iter);		
	}


	ListIterator fams = createIterator(indi->families);
	Family* fam = (Family*)nextElement(&fams);
	while(fam != NULL){

		i = 0;
	
		while( fam != fTracker[i].myFam){
			i++;
		}
		//printf("when does it stop: %s\n", iTracker[i].myIndi->givenName);

		sprintf(concat, "%d", fTracker[i].code);
		string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("1 fams   @F@\n") + strlen(concat) + 3));

		if(indi == fam->husband || indi == fam->wife){
			strcat(string, "1 FAMS @F");
		}
		else{
			strcat(string, "1 FAMC @F");
		}
		strcat(string, concat);
		memset(concat, '\0', 200);
		strcat(string, "@\n");
		fam = (Family*)nextElement(&fams);

	}

	return string;
}

char* writeFamily(Family* fam, int fCount, writeIndiCount iTracker[]){

	char* string = NULL;
	char concat[200];
	memset(concat, '\0', 200);
	int i = 0;

	sprintf(concat, "%d", fCount);
	string = (char*)malloc(sizeof(char)*( strlen("0 @F@ FAM\n") + strlen(concat) + 2));
	strcpy(string, "0 @F");
	strcat(string, concat);
	memset(concat, '\0', 200);
	strcat(string, "@ FAM\n");

	i = 0;
	if(fam->husband != NULL){
		while(compareIndividuals(iTracker[i].myIndi, fam->husband) != 0){
			i++;
		}
		//printf("when does it stop: %d\n", i);

		sprintf(concat, "%d", iTracker[i].code);
		string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("1 HUSB   @I@\n") + strlen(concat) + 3));
		strcat(string, "1 HUSB @I");
		//printf("husb printing@%s@\n", concat);
		strcat(string, concat);
		memset(concat, '\0', 200);
		strcat(string, "@\n");
	}

	i = 0;
	if(fam->wife != NULL){
		while(compareIndividuals(iTracker[i].myIndi, fam->wife) != 0){
			i++;
		}
		//printf("when does it stop: %s\n", iTracker[i].myIndi->givenName);

		sprintf(concat, "%d", iTracker[i].code);
		string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("1 WIFE   @I@\n") + strlen(concat) + 3));
		strcat(string, "1 WIFE @I");
		//printf("wife printing@%s@\n", concat);
		strcat(string, concat);
		memset(concat, '\0', 200);
		strcat(string, "@\n");
	}

	

	ListIterator events = createIterator(fam->events);
	Event* event = (Event*)nextElement(&events);
	while(event != NULL){

		string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("122 DATE PLAC \n") + strlen(event->type) + strlen(event->date) + strlen(event->place) + 20));
		strcat(string, "1 ");
		strcat(string, event->type);
		strcat(string, "\n");
		if(strlen(event->date) != 0){
			strcat(string, "2 DATE ");
			strcat(string, event->date);
			strcat(string, "\n");
		}
		if(strlen(event->place) != 0){
			strcat(string, "2 PLAC ");
			strcat(string, event->place);
			strcat(string, "\n");
		}

		event = (Event*)nextElement(&events);		
	}

	ListIterator iter = createIterator(fam->children);
	Individual* kid = (Individual*)nextElement(&iter);
	while(kid != NULL){

		i = 0;
	
		while(iTracker[i].myIndi != kid){
			i++;
		}
		//printf("when does it stop: %s\n", iTracker[i].myIndi->givenName);

		sprintf(concat, "%d", iTracker[i].code);
		string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen("1 CHIL   @I@\n") + strlen(concat) + 3));
		strcat(string, "1 CHIL @I");
		strcat(string, concat);
		//printf("child printing@%s@\n", concat);
		memset(concat, '\0', 200);
		strcat(string, "@\n");
		kid = (Individual*)nextElement(&iter);

	}






	return string;
}




char* gedTableRow(char* fileName){

	GEDCOMobject* obj = (GEDCOMobject*)malloc(sizeof(GEDCOMobject));
	char extension[60] = "uploads/";
	strcat(extension, fileName);



	GEDCOMerror err = createGEDCOM(extension, &obj);

	if(err.type != OK){
		return "<tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>";
	}

	char* string = malloc(sizeof(char)* strlen("<tr><td id=\"fileName\" ><a href=\"/uploads/\"></a></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>") + strlen(fileName) + strlen(fileName) + strlen(obj->header->source) + 7 + 3 + 3 + strlen(obj->header->submitter->submitterName) + strlen(obj->header->submitter->address) + 10);

	strcpy(string, "<tr><td id=\"fileName\" ><a href=\"/uploads/");
	strcat(string, fileName);
	strcat(string, "\">");
	strcat(string, fileName);
	strcat(string, "</a></td><td>");
	strcat(string, obj->header->source);
	strcat(string, "</td><td>");

	char temp[20];
	sprintf(temp, "%.1f", obj->header->gedcVersion);
	strcat(string, temp);
	strcat(string, "</td><td>");


	//ANSEL, UTF8, UNICODE, ASCII} CharSet
	if(obj->header->encoding == ANSEL){
		strcat(string, "ANSEL</td><td>");
	}
	else if(obj->header->encoding == UTF8){
		strcat(string, "UTF-8</td><td>");
	}
	else if(obj->header->encoding == UNICODE){
		strcat(string, "UNICODE</td><td>");
	}
	else if(obj->header->encoding == ASCII){
		strcat(string, "ASCII</td><td>");
	}

	memset(temp, '\0', 20);
	sprintf(temp, "%d", getLength(obj->individuals));
	strcat(string, temp);
	strcat(string, "</td><td>");

	memset(temp, '\0', 20);
	sprintf(temp, "%d", getLength(obj->families));
	strcat(string, temp);
	strcat(string, "</td><td>");

	strcat(string, obj->submitter->submitterName);
	strcat(string, "</td><td>");
	strcat(string, obj->submitter->address);
	strcat(string, "</td>");


	strcat(string, "</tr>");



	return string;
}

char* gedContents(char* fileName){

	GEDCOMobject* obj = (GEDCOMobject*)malloc(sizeof(GEDCOMobject));
	char extension[60] = "uploads/";
	strcat(extension, fileName);



	GEDCOMerror err = createGEDCOM(extension, &obj);

	if(err.type != OK || getLength(obj->individuals) == 0){
		return "<tr><td></td><td></td><td></td><td></td></tr>";
	}
	char temp[20];
	memset(temp, '\0', 20);

	ListIterator iter = createIterator(obj->individuals);
	Individual* person = (Individual*)nextElement(&iter);

	char* string = malloc(sizeof(char)* strlen(" <tr><td></td><td></td><td></td><td></td></tr>") + strlen(person->givenName) + strlen(person->surname) + 10 );
	strcpy(string, " ");
	//int i = 0;
	while(person != NULL){


		strcat(string, "<tr><td>");
		strcat(string, person->givenName);
		strcat(string, "</td><td>");
		strcat(string, person->surname);
		strcat(string, "</td><td>");

		ListIterator Liter = createIterator(person->otherFields);
		Field* fields = (Field*)nextElement(&Liter);
		while(fields != NULL){
			if(strcmp(fields->tag, "SEX") == 0){
				strcat(string, fields->value);
			}
			fields = (Field*)nextElement(&Liter);
		}

		strcat(string, "</td><td>");
		strcat(string, "</td></tr>");

		// ListIterator Fiter = createIterator(person->otherFields);
		// Family* fam = (Family*)nextElement(&Fiter);
		// if(fam != NULL){
		// 	if(fam->husband != NULL){
		// 		i++;
		// 	}
		// 	if(fam->wife != NULL){
		// 		i++;
		// 	}
		// 	//i = i + getLength(fam->children);
		// 	sprintf(temp, "%d", i);
		// 	strcat(string, temp);
		// 	strcat(string, "</td></tr>");
		// 	memset(temp, '\0', 20);
		// }
		// else{
		// 	strcat(string, "</td></tr>");
		// }

		person = (Individual*)nextElement(&iter);
		if(person != NULL){
			string = (char*)realloc(string, sizeof(char)*( strlen(string) + strlen(" <tr><td></td><td></td><td></td><td></td></tr>") + strlen(person->givenName) + strlen(person->surname) + 10));
		}
		//i = 0;
	}
	//printf("%s\n", string);
	return string;
}


char* sqlFILERow(char* fileName){

	GEDCOMobject* obj = (GEDCOMobject*)malloc(sizeof(GEDCOMobject));
	char extension[60] = "uploads/";
	strcat(extension, fileName);



	GEDCOMerror err = createGEDCOM(extension, &obj);

	if(err.type != OK){
		return "error";
	}

	char* string = malloc(sizeof(char)* strlen(fileName) + strlen(obj->header->source) + 7 + 8 + 3 + 3 + 5 + strlen(obj->header->submitter->submitterName) + strlen(obj->header->submitter->address) + 10);

	
	strcpy(string, fileName);
	strcat(string, ",");
	strcat(string, obj->header->source);
	strcat(string, ",");

	char temp[20];
	sprintf(temp, "%.1f", obj->header->gedcVersion);
	strcat(string, temp);
	strcat(string, ",");


	//ANSEL, UTF8, UNICODE, ASCII} CharSet
	if(obj->header->encoding == ANSEL){
		strcat(string, "ANSEL,");
	}
	else if(obj->header->encoding == UTF8){
		strcat(string, "UTF-8,");
	}
	else if(obj->header->encoding == UNICODE){
		strcat(string, "UNICODE,");
	}
	else if(obj->header->encoding == ASCII){
		strcat(string, "ASCII,");
	}

	strcat(string, obj->submitter->submitterName);
	strcat(string, ",");
	if(strlen(obj->submitter->address) == 0){
		strcat(string, " ");
	}else{
		strcat(string, obj->submitter->address);
	}
	strcat(string, ",");
	memset(temp, '\0', 20);
	sprintf(temp, "%d", getLength(obj->individuals));
	strcat(string, temp);
	strcat(string, ",");

	memset(temp, '\0', 20);
	sprintf(temp, "%d", getLength(obj->families));
	strcat(string, temp);
	//strcat(string, ",");

	return string;
}

char* sqlINDIRow(char* fileName){	GEDCOMobject* obj = (GEDCOMobject*)malloc(sizeof(GEDCOMobject));	char extension[60] = "uploads/";	strcat(extension, fileName);	GEDCOMerror err = createGEDCOM(extension, &obj);	if(err.type != OK || getLength(obj->individuals) == 0){		return "error";	}	char temp[20];	memset(temp, '\0', 20);	ListIterator iter = createIterator(obj->individuals);	Individual* person = (Individual*)nextElement(&iter);	char* string = malloc(sizeof(char)* strlen(person->givenName) + strlen(person->surname) + 6 + 10 );	strcpy(string, " ");	//int i = 0;	while(person != NULL){		strcat(string, person->surname);		strcat(string, ",");		strcat(string, person->givenName);		strcat(string, ",");		ListIterator Liter = createIterator(person->otherFields);		Field* fields = (Field*)nextElement(&Liter);		while(fields != NULL){			if(strcmp(fields->tag, "SEX") == 0){				strcat(string, fields->value);			}			fields = (Field*)nextElement(&Liter);		}		strcat(string, ",0");		person = (Individual*)nextElement(&iter);		if(person != NULL){			string = (char*)realloc(string, sizeof(char)*( strlen(string) + 7 + strlen(person->givenName) + strlen(person->surname) + 10));			strcat(string, ";");		}	}	return string;}


