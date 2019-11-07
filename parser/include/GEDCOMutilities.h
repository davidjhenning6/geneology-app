#ifndef GEDCOMUTILITIES_H
#define GEDCOMUTILITIES_H

#include"GEDCOMparser.h"

//typedef enum month {JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC} Month;

typedef struct{

char tag[255];
char value[1000];
char level[255];

}MyLine;

typedef struct{

	char code[27];
	Individual* myIndi;

}MyIndividuals;

typedef struct{

	char type[5];
	char date[256];
	char place[256];

}MyEvent;

typedef struct{

	Family* myFam;
	char famCode[27];
	char husbCode[27];
	char wifeCode[27];
	char childCodes[20][27];
	int childCount;
	int eventCount;
	MyEvent event[10];

}MyFamilies;

typedef struct{

	int code;
	Individual* myIndi;

}writeIndiCount;

typedef struct{

	int code;
	Family* myFam;

}writeFamCount;




//checks file validity returns 0 on success and -1 on failure
int fileValidity(char* fileName);

void gedcomValidity(FILE *fp, GEDCOMerror* gedErr);

GEDCOMerror lineParse(FILE *fp, MyLine* myline, int lineNum);

int levelCheck(MyLine* myline, char prevLineLevel[]);

GEDCOMerror checkCONXNextLine(FILE *fp, MyLine* myline, int lineNum, char current);

int getMonth(char* month1);

Header* initializeHeader();

Individual* initializeIndividual();

Family* initializeFamily();

GEDCOMobject* initializeGEDCOM();

Submitter* initializeSubmitter();

Event* initializeEvent();

Field* initializefield();

int checkForNewLine(FILE *fp);

int checkNumberOfLines(FILE *fp);

void getChildren(List fams, Individual* person, List* Descend);

Individual* copyPerson(Individual* person);

void deleteList(void* toBeDeleted);

int compareLists(const void* first,const void* second);

char* printList(void* toBePrinted);

List* getNextGen(List* unSorted, List* currentGen);

char* writeHeader(GEDCOMobject* obj);

char* writeSubmitter(GEDCOMobject* obj);

char* writeIndividual(Individual* indi, int iCount, writeFamCount fTracker[]);

char* writeFamily(Family* fam, int fCount, writeIndiCount iTracker[]);

char* gedTableRow(char* fileName);

char* gedContents(char* fileName);

char* sqlFILERow(char* fileName);

char* sqlINDIRow(char* fileName);




#endif