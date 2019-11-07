/*****************************************************************************
 * CIS2750 W2018
 * Assignment 1
 * David Henning 0921760
 *****************************************************************************/

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "GEDCOMutilities.h"
#include "GEDCOMparser.h"
#include "LinkedListAPI.h"

//***************************************** GEDCOOM object functions *****************************************

/** Function to create a GEDCOM object based on the contents of an GEDCOM file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ged extension.
 File represented by this name must exist and must be readable.
 *@post Either:
 A valid GEDCOM has been created, its address was stored in the variable obj, and OK was returned
 or
 An error occurred, the GEDCOM was not created, all temporary memory was freed, obj was set to NULL, and the
 appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the GEDCOM
 *@param fileName - a string containing the name of the GEDCOM file
 *@param a double pointer to a GEDCOMobject struct that needs to be allocated
 **/
GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj){
	//printf("File Name:%s\n", fileName);
	int isHeader = 0;
	int numIndi = 0;
	int numFami = 0;
	//int numLines = 0;
	int submitFound = 0;
	int sourceFound = 0;
	int gedcFound = 0;
	int encodeFound = 0;
	int trailerFound = 0;
	char submitStr[26];
	GEDCOMerror ret;
	fpos_t pos;
	MyLine myline;
	memset(myline.tag, '\0', 255);
	memset(myline.value, '\0', 255);
	memset(myline.level, '\0', 255);
	memset(submitStr, '\0', 26);

	MyIndividuals indiList[1000];
	MyFamilies famList[100];

	
	FILE *fp;

	if(fileValidity(fileName) == -1){
		ret.type = INV_FILE;
		ret.line = -1;
		//printf("why!!%s\n", fileName);
		return ret;
		
	}
	GEDCOMobject* myTemp = (GEDCOMobject*)malloc(sizeof(GEDCOMobject));
	myTemp->individuals = initializeList(printIndividual, deleteIndividual, compareIndividuals);
	myTemp->families = initializeList(printFamily, deleteFamily, compareFamilies);



	fp = fopen(fileName, "r");
	if(fp == NULL){
		//printf("invaldid file ptr!!%s\n", fileName);
		ret.type = INV_FILE;
		ret.line = -1;
		return ret;
	}

	//fgetpos(fp, &start);
	//numLines = checkNumberOfLines(fp);

	//printf("number of lines in file:%d\n", numLines);
	
	int lineNum = 0;
	
	do {
		//current = fgetc(fp);
		ret = lineParse(fp, &myline, lineNum);//if returns other error determine between in head or not to determine whether to throw inv head or inv record if returns ok continue
		//printf("%s %s %s\n", myline.level, myline.tag, myline.value);

		if(lineNum == 0 && strcmp(myline.level, "0") == 0 && strcmp(myline.tag, "HEAD") == 0){
			myTemp->header = (Header*)malloc(sizeof(Header));
			myTemp->header->otherFields = initializeList(printField, deleteField, compareFields);
			//myTemp->header->encoding = ASCII;
			//printf("is it being malloced?\n");

		}

		//error handling

		if(ret.type == INV_HEADER){
			return ret;
		}

		if(lineNum == 0 && strcmp(myline.tag, "HEAD") != 0){
			ret.type = INV_GEDCOM;
			ret.line = -1;
			//printf("here error returned:%u\n", ret.type);
			return ret;

		}

		if(lineNum == 0 && strcmp(myline.level, "0") != 0){
			ret.type = INV_HEADER;
			ret.line = lineNum + 1;
			//printf("error returned:%u\n", ret.type);
			return ret;

		}


		if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "SUBM") == 0 && isHeader == 0){
			submitFound = 1;
			strcpy(submitStr, myline.value);
			//printf("submit is incremented:%s\n", submitStr);
		}

		if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "CHAR") == 0 && isHeader == 0){
			encodeFound = 1;

			if(strcmp(myline.value, "ANSEL") == 0){
				myTemp->header->encoding = ANSEL;
			}
			if(strcmp(myline.value, "UTF-8") == 0){
				myTemp->header->encoding = UTF8;
			}
			if(strcmp(myline.value, "UNICODE") == 0){
				myTemp->header->encoding = UNICODE;
			}
			if(strcmp(myline.value, "ASCII") == 0){
				myTemp->header->encoding = ASCII;
			}
			
		}

		if(strcmp(myline.level, "0") == 0 && strcmp(myline.tag, "TRLR") == 0 && isHeader > 0){
			trailerFound = 1;

			//strcpy(submitStr, myline.value);
			//printf("submit is incremented:%s\n", submitStr);
		}

		if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "SOUR") == 0 && isHeader == 0){
			sourceFound = 1;
			strcpy(myTemp->header->source, myline.value);
			//printf("submit is incremented:%s\n", submitStr);
			fgetpos(fp, &pos);
			do{
				ret = lineParse(fp, &myline, lineNum);
				//printf("%s %s %s\n", myline.level, myline.tag, myline.value);
				lineNum++;
				if(ret.type == INV_HEADER){
					return ret;
				}
				if(ret.type == OTHER_ERROR && isHeader <= 1){
					ret.type = INV_HEADER;
					ret.line = lineNum + 1;
					//printf("INV_HEADER returned line number:%d\n", ret.line);
					return ret;
				}
				if(ret.type == OTHER_ERROR && isHeader >= 1){
					ret.type = INV_RECORD;
					ret.line = lineNum + 1;
					//printf("INV_RECORD returned line number:%d\n", ret.line);
					return ret;
				}

				//if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "NAME") == 0){
				//	strcpy(myTemp->header->source, myline.value);
				//}

				if(strcmp(myline.level, "1") != 0){
					fgetpos(fp, &pos);
				}
				

			} while(strcmp(myline.level, "1") != 0);
			fsetpos(fp, &pos);
			lineNum = lineNum - 1;
		}


		if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "GEDC") == 0 && isHeader == 0){
			gedcFound = 1;
			//printf("submit is incremented:%s\n", submitStr);
			fgetpos(fp, &pos);
			do{
				ret = lineParse(fp, &myline, lineNum);
				//printf("%s %s %s\n", myline.level, myline.tag, myline.value);
				lineNum++;
				if(ret.type == INV_HEADER){
					return ret;
				}
				if(ret.type == OTHER_ERROR && isHeader <= 1){
					ret.type = INV_HEADER;
					ret.line = lineNum + 1;
					//printf("INV_HEADER returned line number:%d\n", ret.line);
					return ret;
				}
				if(ret.type == OTHER_ERROR && isHeader >= 1){
					ret.type = INV_RECORD;
					ret.line = lineNum + 1;
					//printf("INV_RECORD returned line number:%d\n", ret.line);
					return ret;
				}


				if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "VERS") == 0){
					myTemp->header->gedcVersion = atof(myline.value);
				}

				if(strcmp(myline.level, "1") != 0){
					fgetpos(fp, &pos);
				}
				

			} while(strcmp(myline.level, "1") != 0  && !feof(fp));
			fsetpos(fp, &pos);
			lineNum = lineNum - 1;
		}

		if(isHeader < 1 && strcmp(myline.level,"0") == 0 && lineNum > 0){
			isHeader = isHeader + 1;
			//when checking to ensure header is read check if the pointer to the submitter for the header truct == NULL or not!!!!!!!
			if(submitFound == 0){
				ret.type = INV_HEADER;
				ret.line = lineNum + 1;
				//printf("here error returned:%u\n", ret.type);
				return ret;
			}
		}

		if(strcmp(myline.level,"0") == 0 && strcmp(myline.tag, submitStr) == 0 && strcmp(myline.value,"SUBM") == 0){
			submitFound = submitFound + 1;
			myTemp->submitter = (Submitter*)malloc(sizeof(Submitter)+sizeof(char)*300);
			myTemp->submitter->otherFields = initializeList(printField, deleteField, compareFields);
			strcpy(myTemp->submitter->address, "");
			
		}


		if(strcmp(myline.level, "0") == 0 && strcmp(myline.value, "SUBM") == 0){
			//printf("submit is incremented:%s\n", submitStr);
			fgetpos(fp, &pos);
			do{
				ret = lineParse(fp, &myline, lineNum);
				//printf("//%s %s %s\n", myline.level, myline.tag, myline.value);
				lineNum++;
				if(ret.type == INV_HEADER){
					return ret;
				}
				if(ret.type == OTHER_ERROR && isHeader <= 1){
					ret.type = INV_HEADER;
					ret.line = lineNum + 1;
					//printf("INV_HEADER returned line number:%d\n", ret.line);
					return ret;
				}
				if(ret.type == OTHER_ERROR && isHeader >= 1){
					ret.type = INV_RECORD;
					ret.line = lineNum + 1;
					//printf("INV_RECORD returned line number:%d\n", ret.line);
					return ret;
				}


				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "NAME") == 0){
					strcpy(myTemp->submitter->submitterName, myline.value);

				}
				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "ADDR") == 0){
					strcpy(myTemp->submitter->address, myline.value);
					

				}

				if(strcmp(myline.level, "0") != 0){
					fgetpos(fp, &pos);
				}
				

			} while(strcmp(myline.level, "0") != 0 && !feof(fp));
			fsetpos(fp, &pos);
			//printf("where is this\n");
			lineNum = lineNum - 1;
			myTemp->header->submitter = myTemp->submitter;

		}


		
		
		else if(strcmp(myline.level,"0") == 0 && strcmp(myline.value,"INDI") == 0){//individual Parsing
			//printf("made it into individual pre creation\n");
			Individual* indi = initializeIndividual();
			memset((indiList[numIndi]).code, '\0', 27);
			strcpy((indiList[numIndi]).code, myline.tag);
			//printf("\n\ncode:%s at %d\n", indiList[numIndi].code, numIndi);
			//strcpy(indi->address, "");

			//printf("made it into individual\n");
			//loop till end of individual chunk reached
			fgetpos(fp, &pos);
			do{
				ret = lineParse(fp, &myline, lineNum);
				//printf("%s %s %s\n", myline.level, myline.tag, myline.value);
				lineNum++;
				if(ret.type == INV_HEADER){
					return ret;
				}

				if(ret.type == OTHER_ERROR && isHeader <= 1){
					ret.type = INV_HEADER;
					ret.line = lineNum + 1;
					//printf("INV_HEADER returned line number:%d\n", ret.line);
					return ret;
				}

				if(ret.type == OTHER_ERROR && isHeader >= 1){
					ret.type = INV_RECORD;
					ret.line = lineNum + 1;
					//printf("INV_RECORD returned line number:%d\n", ret.line);
					return ret;
				}

				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "NAME") == 0){
					char tempName[1000];
					char giv[1000];
					char sur[1000];
					char sur2[1000];
					int i = 0;
					int t = 0;
					memset(tempName, '\0', 1000);
					memset(giv, '\0', 1000);
					memset(sur, '\0', 1000);
					memset(sur2, '\0', 1000);
					
					//printf("check123\n");
					strcpy(tempName, myline.value);
					
					sscanf(tempName,"%s %s", giv, sur);

					int surLen = strlen(sur);

					//printf("\ncreation flaw '%s' '%s'\n", giv, sur);
					if(strcmp(sur, "") == 0){
						surLen = strlen(giv);
						for(i = 0; i < surLen; i++){
							if(giv[i] != '/'){
								sur2[t] = giv[i];
								t++;
							}
						}
						memset(giv, '\0', 1000);

					}
					else{
						for(i = 0; i < surLen; i++){
							if(sur[i] != '/'){
								sur2[t] = sur[i];
								t++;
							}
						}
					}
					//printf("\ncreation flaw2 '%s' '%s'\n", giv, sur2);
					//strcpy(indi->givenName, giv);
					//strcpy(indi->surname, sur);
					
					//printf("stuck");

					indi->givenName = (char*)malloc(sizeof(char)*strlen(giv)+2);
					indi->surname = (char*)malloc(sizeof(char)*strlen(sur2)+2);

					strcpy(indi->givenName, giv);
					strcpy(indi->surname, sur2);
				
					//printf("given:%s, surname:%s\n", indi->givenName, indi->surname);//indi->givenName, indi->surname);
				}

				//sex is put into other fields
				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "SEX") == 0){
					//char tempName[1000];
					
					//memset(tempName, '\0', 1000);
					Field* sex = initializefield();

					sex->tag = (char*)malloc(sizeof(char)*strlen(myline.tag)+2);
					sex->value = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
					
					strcpy(sex->tag, myline.tag);
					strcpy(sex->value, myline.value);
					
					insertBack(&indi->otherFields, sex);

					//printf("tag:%s, value:%s\n", sex->tag, sex->value);//indi->givenName, indi->surname);
				}


				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "BIRT") == 0){
					//char tempName[1000];
					
					//memset(tempName, '\0', 1000);
					Event* birth = initializeEvent();
					strcpy(birth->type, myline.tag);

					//birth->date = (char*)malloc(sizeof(char)*strlen(myline.tag)+2);
					//birth->place = (char*)malloc(sizeof(char)*strlen(myline.value)+2);


					do{
						ret = lineParse(fp, &myline, lineNum);
						lineNum++;
						if(ret.type == INV_HEADER){
							
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader <= 1){
							ret.type = INV_HEADER;
							ret.line = lineNum + 1;
							//printf("INV_HEADER returned line number:%d\n", ret.line);
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader >= 1){
							ret.type = INV_RECORD;
							ret.line = lineNum + 1;
							//printf("INV_RECORD returned line number:%d\n", ret.line);
							return ret;
						}

						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "DATE") == 0){
							birth->date = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
							strcpy(birth->date, myline.value);

						}
						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "PLAC") == 0){
							birth->place = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
							strcpy(birth->place, myline.value);

						}

						if(strcmp(myline.level, "1") != 0){
							fgetpos(fp, &pos);
						}

					}while(strcmp(myline.level, "1") != 0 && !feof(fp));
					fsetpos(fp, &pos);
					//printf("where is this\n");
					lineNum = lineNum - 1;
					//printf("type:%s date:%s place:%s\n", birth->type, birth->date, birth->place);
					insertBack(&indi->events, birth);
					//printf("tag:%s, value:%s\n", sex->tag, sex->value);//indi->givenName, indi->surname);
				}


				else if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "DEAT") == 0){
					//char tempName[1000];
					
					//memset(tempName, '\0', 1000);
					Event* death = initializeEvent();
					strcpy(death->type, myline.tag);
					//birth->date = (char*)malloc(sizeof(char)*strlen(myline.tag)+2);
					//birth->place = (char*)malloc(sizeof(char)*strlen(myline.value)+2);


					do{
						ret = lineParse(fp, &myline, lineNum);
						lineNum++;
						if(ret.type == INV_HEADER){
							
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader <= 1){
							ret.type = INV_HEADER;
							ret.line = lineNum + 1;
							//printf("INV_HEADER returned line number:%d\n", ret.line);
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader >= 1){
							ret.type = INV_RECORD;
							ret.line = lineNum + 1;
							//printf("INV_RECORD returned line number:%d\n", ret.line);
							return ret;
						}

						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "DATE") == 0){
							death->date = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
							strcpy(death->date, myline.value);

						}
						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "PLAC") == 0){
							death->place = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
							strcpy(death->place, myline.value);

						}

						if(strcmp(myline.level, "1") != 0){
							fgetpos(fp, &pos);
						}

					}while(strcmp(myline.level, "1") != 0 && !feof(fp));
					fsetpos(fp, &pos);
					//printf("where is this\n");
					lineNum = lineNum - 1;
					insertBack(&indi->events, death);
					//printf("type:%s date:%s place:%s\n", death->type, death->date, death->place);
					//printf("tag:%s, value:%s\n", sex->tag, sex->value);//indi->givenName, indi->surname);
				}


				else if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "CHR") == 0){
					//char tempName[1000];
					
					//memset(tempName, '\0', 1000);
					Event* chr = initializeEvent();
					strcpy(chr->type, myline.tag);
					//birth->date = (char*)malloc(sizeof(char)*strlen(myline.tag)+2);
					//birth->place = (char*)malloc(sizeof(char)*strlen(myline.value)+2);


					do{
						ret = lineParse(fp, &myline, lineNum);
						lineNum++;
						if(ret.type == INV_HEADER){
							
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader <= 1){
							ret.type = INV_HEADER;
							ret.line = lineNum + 1;
							//printf("INV_HEADER returned line number:%d\n", ret.line);
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader >= 1){
							ret.type = INV_RECORD;
							ret.line = lineNum + 1;
							//printf("INV_RECORD returned line number:%d\n", ret.line);
							return ret;
						}

						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "DATE") == 0){
							chr->date = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
							strcpy(chr->date, myline.value);

						}
						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "PLAC") == 0){
							chr->place = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
							strcpy(chr->place, myline.value);

						}

						if(strcmp(myline.level, "1") != 0){
							fgetpos(fp, &pos);
						}

					}while(strcmp(myline.level, "1") != 0 && !feof(fp));
					fsetpos(fp, &pos);
					//printf("where is this\n");
					lineNum = lineNum - 1;
					insertBack(&indi->events, chr);
					//printf("type:%s date:%s place:%s\n", chr->type, chr->date, chr->place);
					//printf("tag:%s, value:%s\n", sex->tag, sex->value);//indi->givenName, indi->surname);
				}


				else if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "BURI") == 0){
					//char tempName[1000];
					
					//memset(tempName, '\0', 1000);
					Event* burial = initializeEvent();
					strcpy(burial->type, myline.tag);
					//birth->date = (char*)malloc(sizeof(char)*strlen(myline.tag)+2);
					//birth->place = (char*)malloc(sizeof(char)*strlen(myline.value)+2);


					do{
						ret = lineParse(fp, &myline, lineNum);
						lineNum++;
						if(ret.type == INV_HEADER){
							
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader <= 1){
							ret.type = INV_HEADER;
							ret.line = lineNum + 1;
							//printf("INV_HEADER returned line number:%d\n", ret.line);
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader >= 1){
							ret.type = INV_RECORD;
							ret.line = lineNum + 1;
							//printf("INV_RECORD returned line number:%d\n", ret.line);
							return ret;
						}

						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "DATE") == 0){
							burial->date = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
							strcpy(burial->date, myline.value);

						}
						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "PLAC") == 0){
							burial->place = (char*)malloc(sizeof(char)*strlen(myline.value)+2);
							strcpy(burial->place, myline.value);

						}

						if(strcmp(myline.level, "1") != 0){
							fgetpos(fp, &pos);
						}

					}while(strcmp(myline.level, "1") != 0 && !feof(fp));
					fsetpos(fp, &pos);
					//printf("where is this\n");
					lineNum = lineNum - 1;
					insertBack(&indi->events, burial);
					//printf("type:%s date:%s place:%s\n", burial->type, burial->date, burial->place);
					//printf("tag:%s, value:%s\n", sex->tag, sex->value);//indi->givenName, indi->surname);
				}



				if(strcmp(myline.level, "0") != 0){
					fgetpos(fp, &pos);
				}
				

			} while(strcmp(myline.level, "0") != 0 && !feof(fp));
			fsetpos(fp, &pos);
			lineNum = lineNum - 1;
			//printf("indi added to list::%s\n", indi->givenName);
			insertBack(&myTemp->individuals, indi);
			indiList[numIndi].myIndi = (Individual*)malloc(sizeof(indi)+2);
			indiList[numIndi].myIndi = indi;
			numIndi = numIndi + 1;
			//printf("where is this indi\n");
		}
		
		else if(strcmp(myline.level,"0") == 0 && strcmp(myline.value,"FAM") == 0){//family parsing

			Family* fam = initializeFamily();
			famList[numFami].childCount = 0;
			famList[numFami].eventCount = 0;
			memset(famList[numFami].wifeCode,'\0', 27);
			memset(famList[numFami].husbCode,'\0', 27);
			strcpy(famList[numFami].famCode, myline.tag);
			//printf("---------------------------------------Fam::%s Fam number::%d\n", famList[numFami].famCode, numFami);
			//loop till end of family chunk reached
			fgetpos(fp, &pos);
			do{
				ret = lineParse(fp, &myline, lineNum);
				//printf("%s %s %s\n", myline.level, myline.tag, myline.value);
				lineNum++;
				if(ret.type == INV_HEADER){
					return ret;
				}
				if(ret.type == OTHER_ERROR && isHeader <= 1){
					ret.type = INV_HEADER;
					ret.line = lineNum + 1;
					//printf("INV_HEADER returned line number:%d\n", ret.line);
					return ret;
				}
				if(ret.type == OTHER_ERROR && isHeader >= 1){
					ret.type = INV_RECORD;
					ret.line = lineNum + 1;
					//printf("INV_RECORD returned line number:%d\n", ret.line);
					return ret;
				}


				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "HUSB") == 0){
					memset(famList[numFami].husbCode,'\0', 27);
					strcpy(famList[numFami].husbCode, myline.value);
					//printf("husb::%s\n", famList[numFami].husbCode);
					//cmp myline.value to code in my array of structs that will contain a string with the code ie:@..@ and a pointer to the individual
					//printf("husband::%s ", indiList[cmpTag].code);

				}
				
				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "WIFE") == 0){
					memset(famList[numFami].wifeCode,'\0', 27);
					strcpy(famList[numFami].wifeCode, myline.value);
					//printf("wife::%s\n", famList[numFami].wifeCode);
					//int cmpTag = 0;
					
				}

				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "CHIL") == 0){
					strcpy(famList[numFami].childCodes[famList[numFami].childCount], myline.value);
					//printf("%s\n", myline.value);
					//printf("child::%s\n", famList[numFami].childCodes[famList[numFami].childCount]);
					//printf("the cookie monster");
					famList[numFami].childCount++;
					
				}





				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "MARR") == 0){
					strcpy(famList[numFami].event[famList[numFami].eventCount].type, myline.tag);
					memset(famList[numFami].event[famList[numFami].eventCount].type, '\0', 5);
					memset(famList[numFami].event[famList[numFami].eventCount].date, '\0', 256);
					memset(famList[numFami].event[famList[numFami].eventCount].place, '\0', 256);
					strcpy(famList[numFami].event[famList[numFami].eventCount].type, myline.tag);
					do{
						ret = lineParse(fp, &myline, lineNum);
						lineNum++;
						if(ret.type == INV_HEADER){
							
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader <= 1){
							ret.type = INV_HEADER;
							ret.line = lineNum + 1;
							//printf("INV_HEADER returned line number:%d\n", ret.line);
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader >= 1){
							ret.type = INV_RECORD;
							ret.line = lineNum + 1;
							//printf("INV_RECORD returned line number:%d\n", ret.line);
							return ret;
						}

						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "DATE") == 0){
							
							strcpy(famList[numFami].event[famList[numFami].eventCount].date, myline.value);

						}
						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "PLAC") == 0){
							
							strcpy(famList[numFami].event[famList[numFami].eventCount].place, myline.value);

						}

						if(strcmp(myline.level, "1") != 0 && strcmp(myline.level, "0") != 0){
							fgetpos(fp, &pos);
						}

					}while(strcmp(myline.level, "1") != 0 && strcmp(myline.level, "0") != 0 && !feof(fp));
					fsetpos(fp, &pos);
					//printf("where is this\n");
					famList[numFami].eventCount++;
					lineNum = lineNum - 1;
				}



















				if(strcmp(myline.level, "1") == 0 && strcmp(myline.tag, "MARR") == 0){
					strcpy(famList[numFami].event[famList[numFami].eventCount].type, myline.tag);
					memset(famList[numFami].event[famList[numFami].eventCount].type, '\0', 5);
					memset(famList[numFami].event[famList[numFami].eventCount].date, '\0', 256);
					memset(famList[numFami].event[famList[numFami].eventCount].place, '\0', 256);
					strcpy(famList[numFami].event[famList[numFami].eventCount].type, myline.tag);
					do{
						ret = lineParse(fp, &myline, lineNum);
						lineNum++;
						if(ret.type == INV_HEADER){
							
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader <= 1){
							ret.type = INV_HEADER;
							ret.line = lineNum + 1;
							//printf("INV_HEADER returned line number:%d\n", ret.line);
							return ret;
						}
						if(ret.type == OTHER_ERROR && isHeader >= 1){
							ret.type = INV_RECORD;
							ret.line = lineNum + 1;
							//printf("INV_RECORD returned line number:%d\n", ret.line);
							return ret;
						}

						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "DATE") == 0){
							
							strcpy(famList[numFami].event[famList[numFami].eventCount].date, myline.value);

						}
						if(strcmp(myline.level, "2") == 0 && strcmp(myline.tag, "PLAC") == 0){
							
							strcpy(famList[numFami].event[famList[numFami].eventCount].place, myline.value);

						}

						if(strcmp(myline.level, "1") != 0 && strcmp(myline.level, "0") != 0){
							fgetpos(fp, &pos);
						}

					}while(strcmp(myline.level, "1") != 0 && strcmp(myline.level, "0") != 0 && !feof(fp));
					fsetpos(fp, &pos);
					//printf("where is this\n");
					famList[numFami].eventCount++;
					lineNum = lineNum - 1;
				}
				if(strcmp(myline.level, "0") != 0){
					fgetpos(fp, &pos);
				}
			} while(strcmp(myline.level, "0") != 0 && !feof(fp));
			fsetpos(fp, &pos);
			lineNum = lineNum - 1;
			famList[numFami].myFam = (Family*)malloc(sizeof(fam)+2);
			famList[numFami].myFam = fam;
			numFami++;
			//insertBack(&myTemp->families, fam);
		}


		if(ret.type == OTHER_ERROR && isHeader <= 1){
			ret.type = INV_HEADER;
			ret.line = lineNum + 1;
			//printf("INV_HEADER returned line number:%d\n", ret.line);
			return ret;
		}
		if(ret.type == OTHER_ERROR && isHeader >= 1){
			ret.type = INV_RECORD;
			ret.line = lineNum + 1;
			//printf("INV_RECORD returned line number:%d\n", ret.line);
			return ret;
		}
		

		//printf("//parsed line =>  %s || %s || %s \n", myline.level, myline.tag, myline.value);
		lineNum = lineNum + 1;
		
		
		
		
	} while(!feof(fp));
	int insert = 0;
	int insertChild = 0;
	int cmpTag = 0;
	int husbExists = 0;
	int wifeExists = 0;

	
	for(insert = 0; insert < numFami; insert++){
		Family* tempFam = initializeFamily();
		//printf("\n\nFamCode %s\n", famList[insert].famCode);

		int eventCycle = 0;
		
		for(eventCycle = 0; eventCycle < famList[insert].eventCount; eventCycle++){
			Event* event = initializeEvent();

			event->date = (char*)malloc(sizeof(char)*strlen(famList[insert].event[eventCycle].date)+2);
			event->place = (char*)malloc(sizeof(char)*strlen(famList[insert].event[eventCycle].place)+2);
			strcpy(event->type, famList[insert].event[eventCycle].type);
			strcpy(event->date, famList[insert].event[eventCycle].date);
			strcpy(event->place, famList[insert].event[eventCycle].place);
			insertBack(&tempFam->events, event);
		}






		husbExists = 0;
		wifeExists = 0;

		//memset(famList[insert].husbCode, '\0', 27);
		//memset(famList[insert].wifeCode, '\0', 27);

		//fill the pointer to husband for my family struct without the individual pointing to 
		if(strlen(famList[insert].husbCode) != 0){
			cmpTag = 0;
			husbExists = 1;
			while(strcmp(famList[insert].husbCode, indiList[cmpTag].code) != 0){
				cmpTag++;
			}
			//famList[insert].myFam->husband = indiList[cmpTag].myIndi;
			tempFam->husband = indiList[cmpTag].myIndi;
			//printf("        HusbandCode %s\n", indiList[cmpTag].code);
		}

		if(strlen(famList[insert].wifeCode) != 0 && strlen(indiList[cmpTag].code) != 0 ){
			cmpTag = 0;
			wifeExists = 1;
			while(strcmp(famList[insert].wifeCode, indiList[cmpTag].code) != 0){
				cmpTag++;
			}
			//famList[insert].myFam->wife = indiList[cmpTag].myIndi;
			tempFam->wife = indiList[cmpTag].myIndi;
			//printf("        WifeCode %s\n", indiList[cmpTag].code);
		}
		insertChild = 0;
		//if(famList[numFami].childCount > 0){
			for(insertChild = 0; insertChild < famList[insert].childCount; insertChild++){
				cmpTag = 0;
				while(strcmp(famList[insert].childCodes[insertChild], indiList[cmpTag].code) != 0){
					cmpTag++;
				}
				//printf("Count Dracula");
				insertBack(&tempFam->children,indiList[cmpTag].myIndi);
				//printf("child%s\n", indiList[cmpTag].code);
				//printf("        childCode %s\n", indiList[cmpTag].code);
			}
		//}

		if(husbExists == 1){
			insertBack(&tempFam->husband->families, tempFam);
		}
		if(wifeExists == 1){
			insertBack(&tempFam->wife->families, tempFam);
		}

		//if(famList[numFami].childCount > 0){
			ListIterator iter = createIterator(tempFam->children);
			Individual* found = (Individual*)nextElement(&iter);

			while(found != NULL){
				insertBack(&found->families, tempFam);
				found = (Individual*)nextElement(&iter);
			}
		//}




		insertBack(&myTemp->families, tempFam);

	}

	/*if(strcmp(myline.level, "0") != 0 || strcmp(myline.tag, "TRLR") != 0){//end of file reached check for TRLR
		//printf("print level:%s || print tag:%s", myline.level, myline.tag);
		ret.type = INV_GEDCOM;
		ret.line = -1;
		//printf("INV_GEDCOM!!%s\n", fileName);
		return ret;
	}*/
	
	//printf("opened it!\n");
	if(submitFound != 2){
		ret.type = INV_GEDCOM;
		ret.line = -1;
		//printf("here error returned:%u\n", ret.type);
		return ret;
	}

	if(gedcFound != 1){
		ret.type = INV_HEADER;
		ret.line = lineNum;
		//printf("here error returned:%u\n", ret.type);
		return ret;
	}
	if(sourceFound != 1){
		ret.type = INV_HEADER;
		ret.line = lineNum;
		//printf("here error returned:%u\n", ret.type);
		return ret;
	}
	if(encodeFound != 1){
		ret.type = INV_HEADER;
		ret.line = lineNum;
		//printf("here error returned:%u\n", ret.type);
		return ret;
	}
	if(trailerFound != 1){
		ret.type = INV_GEDCOM;
		ret.line = -1;
		//printf("here error returned:%u\n", ret.type);
		return ret;
	}


	fclose(fp);


	*obj = myTemp;
	ret.type = OK;
	ret.line = -1;
	//printf("\nOk returned!!%s\n", fileName);
	return ret;
}


/** Function to create a string representation of a GEDCOMobject.
 *@pre GEDCOMobject object exists, is not null, and is valid
 *@post GEDCOMobject has not been modified in any way, and a string representing the GEDCOM contents has been created
 *@return a string contaning a humanly readable representation of a GEDCOMobject
 *@param obj - a pointer to a GEDCOMobject struct
 **/
char* printGEDCOM(const GEDCOMobject* obj){
	char* str = "hicheck";
	return str;
}


/** Function to delete all GEDCOM object content and free all the memory.
 *@pre GEDCOM object exists, is not null, and has not been freed
 *@post GEDCOM object had been freed
 *@return none
 *@param obj - a pointer to a GEDCOMobject struct
 **/
void deleteGEDCOM(GEDCOMobject* obj){

	return;
}


/** Function to "convert" the GEDCOMerror into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code
 *@param err - an error struct
 **/
char* printError(GEDCOMerror err){
	char* str = NULL;
	switch(err.type){
		case OK:
			str = (char*)malloc(sizeof(char)*strlen("Okay")+1);
			strcpy(str, "Okay");
			break;
		case INV_FILE:
			str = (char*)malloc(sizeof(char)*strlen("Invalid file")+1);
			strcpy(str, "Invalid file");
			break;
		case INV_GEDCOM:
			str = (char*)malloc(sizeof(char)*strlen("Invalid GEDCOM object")+1);
			strcpy(str, "Invalid GEDCOM object");
			break;
		case INV_HEADER:
			str = (char*)malloc(sizeof(char)*strlen("Invalid header")+1);
			strcpy(str, "Invalid header");
			break;
		case INV_RECORD:
			str = (char*)malloc(sizeof(char)*strlen("Invalid record")+1);
			strcpy(str, "Invalid record");
			break;
		case OTHER_ERROR:
			str = (char*)malloc(sizeof(char)*strlen("Error")+1);
			strcpy(str, "Error");
			break;
		case WRITE_ERROR:
			str = (char*)malloc(sizeof(char)*strlen("Write error")+1);
			strcpy(str, "Write error");
			break;
		default:
			str = (char*)malloc(sizeof(char)*strlen("Error")+1);
			strcpy(str, "Error");
			break;
	}
	return str;
}


/** Function that searches for an individual in the list using a comparator function.
 * If an individual is found, a pointer to the Individual record
 * Returns NULL if the individual is not found.
 *@pre GEDCOM object exists,is not NULL, and is valid.  Comparator function has been provided.
 *@post GEDCOM object remains unchanged.
 *@return The Individual record associated with the person that matches the search criteria.  If the Individual record is not found, return NULL.
 *If multiple records match the search criteria, return the first one.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param compare - a pointer to comparator fuction for customizing the search
 *@param person - a pointer to search data, which contains seach criteria
 *Note: while the arguments of compare() and person are all void, it is assumed that records they point to are
 *      all of the same type - just like arguments to the compare() function in the List struct
 **/
Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person){
	Individual* search = (Individual*)person;

	//for fail return NULL
	if(familyRecord == NULL){
		return NULL;
	}
	if(search == NULL){
		return NULL;
	}
	//printf("\nfind Person--------------------------------------------------------\n");
	ListIterator iter = createIterator(familyRecord->individuals);
	Individual* found = (Individual*)nextElement(&iter);
	if(found == NULL){
		return NULL;
	}
	//printf("called it\n");
	//char* string = printIndividual(found);
	//printf("found individual: %s\n", string);
	//printf("\nfind Person1--------------------------------------------------------\n");
	
	//upon success return ptr to individual
	if(compareIndividuals(search, found) == 0){
		//printf("person found\n");
		return found;
	}
	//printf("\nfind Person2--------------------------------------------------------\n");
	while(found != NULL){

		found = (Individual*)nextElement(&iter);
		//upon success return ptr to individual
		if(compareIndividuals(search, found) == 0){
			//printf("person found2\n");
			return found;
		}
	}

	//printf("\nfind Person3--------------------------------------------------------\n");
	//return null upon failing to find person
	return NULL;
}


/** Function to return a list of all descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of descendants has been created
 *@return a list of descendants.  The list may be empty.  All list members must be of type Individual, and can appear in any order.
 *All list members must be COPIES of the Individual records in the GEDCOM file.  If the returned list is freed, the original GEDCOM
 *must remain unaffected.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 **/
List getDescendants(const GEDCOMobject* familyRecord, const Individual* person){


	Individual* myPerson = (Individual*) person;
	List descend = initializeList(printIndividual, deleteIndividual, compareIndividuals);
	if(familyRecord == NULL || person == NULL ){
		return descend;
	}

	int len = getLength(myPerson->families);
	if(len == 0){
		return descend;
	}

	ListIterator iter = createIterator(myPerson->families);
	Family* fam = (Family*)nextElement(&iter);

	while(fam != NULL){

		if((fam->husband != NULL && compareIndividuals(myPerson, fam->husband) == 0 )|| (fam->wife != NULL && compareIndividuals(myPerson, fam->wife) == 0)){
			ListIterator kiderate = createIterator(fam->children);
			Individual* child = (Individual*)nextElement(&kiderate);

			while(child != NULL){
				//printf("child%s, %s, %d\n", child->givenName, child->surname, count);
				Individual* myChild = copyPerson(child);
				insertBack(&descend, myChild);
				getChildren(child->families, child, &descend);
				child = (Individual*)nextElement(&kiderate);
			}
		}
		fam = (Family*)nextElement(&iter);
	}
                                                                                                                                                                             
	return descend;
}






// ****************************** A2 functions ******************************

/** Function to writing a GEDCOMobject into a file in GEDCOM format.
 *@pre GEDCOMobject object exists, is not null, and is valid
 *@post GEDCOMobject has not been modified in any way, and a file representing the
 GEDCOMobject contents in GEDCOM format has been created
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param obj - a pointer to a GEDCOMobject struct
 **/
GEDCOMerror writeGEDCOM(char* fileName, const GEDCOMobject* obj){
	//printf("%s\n", fileName);
	if( fileName == NULL || strlen(fileName) == 0 || obj == NULL ){
		GEDCOMerror ret;
		ret.type = WRITE_ERROR;
		ret.line = 0;
		return ret;
	}
	
	int iCount = 0;
	int fCount = 0;
	int ilen = 0;
	int flen = 0;

	GEDCOMobject* myObj = (GEDCOMobject*)obj;

	FILE* fp = fopen(fileName, "w");

	fprintf(fp, "%s", writeHeader(myObj) );


	if(getLength(myObj->individuals) != 0){
		ilen = getLength(myObj->individuals);
	}
	else{
		ilen = 20;
	}

	writeIndiCount iTracker[ilen];

	if(getLength(myObj->families) != 0){
		flen = getLength(myObj->families);
	}
	else{
		flen = 20;
	}

	writeFamCount fTracker[flen];

	ListIterator fpop = createIterator(myObj->families);
	Family* famPop = (Family*)nextElement(&fpop);
	while(famPop != NULL){

		fTracker[fCount].myFam = famPop;
		fTracker[fCount].code = fCount;
		fCount++;
		famPop = (Family*)nextElement(&fpop);
	}

	


	ListIterator iter = createIterator(myObj->individuals);
	Individual* indi = (Individual*)nextElement(&iter);
	while(indi != NULL){
		fprintf(fp, "%s", writeIndividual(indi, iCount, fTracker));

		iTracker[iCount].myIndi = indi;
		iTracker[iCount].code = iCount;
		//printf("iCount%d\n", iTracker[iCount].code);



		iCount++;
		indi = (Individual*)nextElement(&iter);
	}

	ListIterator fiter = createIterator(myObj->families);
	Family* fam = (Family*)nextElement(&fiter);
	fCount = 0;
	while(fam != NULL){
		fprintf(fp, "%s", writeFamily(fam, fCount, iTracker));
		fCount++;
		fam = (Family*)nextElement(&fiter);
	}


	fprintf(fp, "%s", writeSubmitter(myObj) );
	fprintf(fp, "%s", "0 TRLR" );
  	fclose(fp);



  	//printf("hi");


	GEDCOMerror ret;
	ret.type = OK;
	ret.line = 0;
	return ret;
}

/** Function for validating an existing GEDCOM object
 *@pre GEDCOM object exists and is not null
 *@post GEDCOM object has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the GEDCOM
 *@param obj - a pointer to a GEDCOMobject struct
 **/
ErrorCode validateGEDCOM(const GEDCOMobject* obj){
	//typedef enum eCode {OK, INV_FILE, INV_GEDCOM, INV_HEADER, INV_RECORD, OTHER_ERROR, WRITE_ERROR} ErrorCode;
	if(obj == NULL){
		return INV_GEDCOM;
	}
	if(obj->header == NULL){
		return INV_GEDCOM;
	}
	if(obj->submitter == NULL){
		return INV_GEDCOM;
	}
	if(obj->header->submitter == NULL){
		return INV_HEADER;
	}
	if(obj->header->submitter == NULL){
		return INV_RECORD;
	}
	if(strlen(obj->header->source) == 0){
		return INV_HEADER;
	}
	if(strlen(obj->submitter->submitterName) == 0){
		return INV_RECORD;
	}

	//**this only passes test** create a validate HEADER func, INDIVIDUAL func, EVENT func, FAMILY func(just ensure a person exists in a family and that the events are valid)


	

	return OK;
}



/** Function to return a list of up to N generations of descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of descendants has been created
 *@return a list of descendants.  The list may be empty.  All list members must be of type List.    *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen){

	if(maxGen == 0){
		maxGen = 100;
	}
	int i = 1;
	int baseCase = 0;
	//.int baseCaseCount = 0;
	List Ldescend = initializeList(printList, deleteList, compareLists);

	if(familyRecord == NULL || person ==  NULL || getLength(person->families) == 0){
		return Ldescend;
	}

	List unSorted = getDescendants(familyRecord, person);
	//printf("%s\n", iListToJSON(unSorted));
	List* LgenOne = (List*) malloc(sizeof(List));
	LgenOne -> printData = printIndividual;
	LgenOne -> deleteData = deleteIndividual;
	LgenOne -> compare = compareIndividuals;
	LgenOne -> head = NULL;
	LgenOne -> tail = NULL;
	LgenOne -> length = 0;


	Individual* myPerson = (Individual*)person;

	ListIterator check = createIterator(myPerson->families);
	Family* check2 = (Family*)nextElement(&check);
	while(check2 != NULL){

		if((check2->husband != NULL && compareIndividuals(myPerson, check2->husband) == 0 )|| (check2->wife != NULL && compareIndividuals(myPerson, check2->wife) == 0)){
			if(getLength(check2->children) == 0){
				return Ldescend;
			}
		}

		check2 = (Family*)nextElement(&check);
	}

	//printf("%s\n", indToJSON(myPerson));
	ListIterator iter = createIterator(unSorted);
	Individual* ret = (Individual*)nextElement(&iter);
	//printf("%s\n", ret->givenName);

	while(ret != NULL){

		ListIterator familate = createIterator(ret->families);
		Family* fam = (Family*)nextElement(&familate);
		
		while(fam != NULL){
			//printf("__________%s\n", fam->husband->givenName);
			if((fam->husband != NULL && compareIndividuals(myPerson, fam->husband) == 0 )|| (fam->wife != NULL && compareIndividuals(myPerson, fam->wife) == 0)){
				Individual* myRet = copyPerson(ret);
				insertSorted(LgenOne, myRet);
				//printf("hi\n");

			}
			fam = (Family*)nextElement(&familate);
		}
		//printf("%s\n", ret->givenName);
		ret = (Individual*)nextElement(&iter);	
	}
	//printf("print me please\n");

	List* LgenY = (List*) malloc(sizeof(List));
	LgenY -> printData = printIndividual;
	LgenY -> deleteData = deleteIndividual;
	LgenY -> compare = compareIndividuals;
	LgenY -> head = NULL;
	LgenY -> tail = NULL;
	LgenY -> length = 0;
	LgenY = LgenOne;

	while(i < maxGen){
		List* LgenX = (List*) malloc(sizeof(List));
		LgenX -> printData = printIndividual;
		LgenX -> deleteData = deleteIndividual;
		LgenX -> compare = compareIndividuals;
		LgenX -> head = NULL;
		LgenX -> tail = NULL;
		LgenX -> length = 0;
		LgenX = getNextGen(&unSorted, LgenY);

		ListIterator baseCheck = createIterator(*LgenX);
		Individual* baseCheck2 = (Individual*)nextElement(&baseCheck);
		baseCase = 0;
		while(baseCheck2 != NULL){
			if( getLength(baseCheck2->families) == 1 ){
				//printf("they only have one fam\n");
				baseCase++;
			}
			else{
				List blank = initializeList(printList, deleteList, compareLists);
				List tempDescend = initializeList(printList, deleteList, compareLists);
				tempDescend = getDescendants(familyRecord, baseCheck2);
				if(getLength(tempDescend) == getLength(blank)){
					//printf("they have more than one fam but no kids\n");
					baseCase++;
				}
			}
			//baseCaseCount++;
			//printf("they are in a list\n");
			baseCheck2 = (Individual*)nextElement(&baseCheck);
		}
		

		if(LgenX->head == NULL){
			//printf("im here\n");
			break;
		}
		//printf("\ngenX  %s\n\n", iListToJSON(*LgenX));
		insertBack(&Ldescend, LgenX);
		i++;



		LgenY = LgenX;
		//printf("genx lenght: %d == baseCase: %d\n\n\n", getLength(*LgenX), baseCase);
		if(getLength(*LgenX) == baseCase){
			//printf("why!!!!!!\n");
			break;
		}
			
	}
	insertFront(&Ldescend, (void*)LgenOne);
	
	//printf("dont me please\n");

	//printf("%s\n\n\n", gListToJSON(Ldescend));
	
	return Ldescend;
}



/** Function to return a list of up to N generations of ancestors of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of ancestors has been created
 *@return a list of ancestors.  The list may be empty.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen){

	List Lascend = initializeList(printIndividual, deleteIndividual, compareIndividuals);

	
	if(familyRecord == NULL || person ==  NULL || getLength(person->families) == 0){
		return Lascend;
	}
	int husb = 0;
	int wif = 0;
	int i = 1;
	int baseCase = 0;
	int count = 0;
	if(maxGen == 0){
		maxGen = 100;
	}
	Individual* myPerson = (Individual*)person;

	ListIterator check = createIterator(myPerson->families);
	Family* check2 = (Family*)nextElement(&check);
	while(check2 != NULL){

		if(getLength(check2->children) == 0){
			count++;
		}

		
		check2 = (Family*)nextElement(&check);
	}

	if(count == getLength(myPerson->families)){
		return Lascend;
	}






	List* LgenM = (List*) malloc(sizeof(List));
	LgenM -> printData = printIndividual;
	LgenM -> deleteData = deleteIndividual;
	LgenM -> compare = compareIndividuals;
	LgenM -> head = NULL;
	LgenM -> tail = NULL;
	LgenM -> length = 0;

	
	ListIterator familate = createIterator(myPerson->families);
	Family* fam = (Family*)nextElement(&familate);

	while(fam != NULL){

		ListIterator kids = createIterator(fam->children);
		Individual* ruMyPerson = (Individual*)nextElement(&kids);
		while(ruMyPerson != NULL){
			if(compareIndividuals(ruMyPerson, myPerson) == 0){

				//iterate through LgenM
				ListIterator dup = createIterator(*LgenM);
				Individual* dupCheck = (Individual*)nextElement(&dup);
				husb = 0;
				while(dupCheck != NULL){
					if(compareIndividuals(dupCheck, fam->husband) == 0){
						husb++;
					}
					dupCheck = (Individual*)nextElement(&dup);
				}
				if(husb == 0){
					insertSorted(LgenM, fam->husband);
				}

				ListIterator dupf = createIterator(*LgenM);
				Individual* dupfCheck = (Individual*)nextElement(&dupf);
				wif = 0;
				while(dupfCheck != NULL){
					if(compareIndividuals(dupfCheck, fam->wife) == 0){
						wif++;
					}
					dupfCheck = (Individual*)nextElement(&dupf);
				}
				if(wif == 0){
					insertSorted(LgenM, fam->wife);
				}
			}
			ruMyPerson = (Individual*)nextElement(&kids);
		}
		fam = (Family*)nextElement(&familate);
	}

	insertFront(&Lascend, LgenM);

	

	List* LgenZ = (List*) malloc(sizeof(List));
	LgenZ -> printData = printIndividual;
	LgenZ -> deleteData = deleteIndividual;
	LgenZ -> compare = compareIndividuals;
	LgenZ -> head = NULL;
	LgenZ -> tail = NULL;
	LgenZ -> length = 0;


	while(i < maxGen){



		List* LgenB = (List*) malloc(sizeof(List));
		LgenB -> printData = printIndividual;
		LgenB -> deleteData = deleteIndividual;
		LgenB -> compare = compareIndividuals;
		LgenB -> head = NULL;
		LgenB -> tail = NULL;
		LgenB -> length = 0;





		ListIterator LprevGen = createIterator(*LgenM);
		Individual* prevGen = (Individual*)nextElement(&LprevGen);

		while(prevGen != NULL){

			ListIterator Xfamilate = createIterator(prevGen->families);
			Family* Xfam = (Family*)nextElement(&Xfamilate);

			while(Xfam != NULL){

				ListIterator Xchild = createIterator(Xfam->children);
				Family* ruXChild = (Family*)nextElement(&Xchild);
				while(ruXChild != NULL){

					if(compareIndividuals(ruXChild, prevGen) == 0){

						ListIterator dupX = createIterator(*LgenB);
						Individual* dupXCheck = (Individual*)nextElement(&dupX);
						husb = 0;
						while(dupXCheck != NULL){
							if(compareIndividuals(dupXCheck, Xfam->husband) == 0){
								husb++;
							}
							dupXCheck = (Individual*)nextElement(&dupX);
						}
						if(husb == 0){
							insertSorted(LgenB, Xfam->husband);
						}

						ListIterator dupXf = createIterator(*LgenM);
						Individual* dupXfCheck = (Individual*)nextElement(&dupXf);
						wif = 0;
						while(dupXfCheck != NULL){
							if(compareIndividuals(dupXfCheck, Xfam->wife) == 0){
								wif++;
							}
							dupXfCheck = (Individual*)nextElement(&dupXf);
						}
						if(wif == 0){
							insertSorted(LgenB, Xfam->wife);
						}

					}
					ruXChild = (Family*)nextElement(&Xchild);
				}
				Xfam = (Family*)nextElement(&Xfamilate);
			}
			prevGen = (Individual*)nextElement(&LprevGen);

		}

		if(getLength(*LgenB) != 0){
			insertBack(&Lascend, LgenB);
		}
		//printf("===> %s\n\n", iListToJSON(*LgenB));

		ListIterator baseCheck = createIterator(*LgenB);
		Individual* baseCheck2 = (Individual*)nextElement(&baseCheck);
		baseCase = 0;
		while(baseCheck2 != NULL){
			if( getLength(baseCheck2->families) == 1 ){
				//printf("they only have one fam\n");
				baseCase++;
			}
			else{
				List blank = initializeList(printList, deleteList, compareLists);
				List tempDescend = initializeList(printList, deleteList, compareLists);
				tempDescend = getDescendants(familyRecord, baseCheck2);
				if(getLength(tempDescend) == getLength(blank)){
					//printf("they have more than one fam but no kids\n");
					baseCase++;
				}
			}
			//baseCaseCount++;
			//printf("they are in a list\n");
			baseCheck2 = (Individual*)nextElement(&baseCheck);
		}
		if(getLength(*LgenB) == baseCase){
			//printf("why!!!!!!\n");
			break;
		}


		LgenM = LgenB;
		LgenB = LgenZ;
		i++;
	}




	



	//printf("%s\n\n", gListToJSON(Lascend));



	return Lascend;
}

/** Function for converting an Individual struct into a JSON string
 *@pre Individual exists, is not null, and is valid
 *@post Individual has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param ind - a pointer to an Individual struct
 **/
char* indToJSON(const Individual* ind){
	if(ind == NULL){
		char* str = "";
		return str;
	}

	if(ind->givenName == NULL || strlen(ind->givenName) == 0){
		char* givStr = (char*)malloc(sizeof(char)*3);
		strcpy(givStr, "");
	}
	else{
		char* givStr = (char*)malloc(sizeof(char)*strlen(ind->givenName)+2);
		strcpy(givStr, ind->givenName);
	}

	if(ind->surname == NULL || strlen(ind->surname) == 0){
		char* surStr = (char*)malloc(sizeof(char)*3);
		strcpy(surStr, "");
	}
	else{
		char* surStr = (char*)malloc(sizeof(char)*strlen(ind->surname)+2);
		strcpy(surStr, ind->surname);
	}

	//"{"givenName":"William","surname":"Shakespeare"}"

	//printf("%s\n", ind->givenName);
	//printf("%s\n", ind->surname);


	char* ret = (char*)malloc(sizeof(char)*(strlen("{\"givenName\":\"\",\"surname\":\"\"}") + strlen(ind->givenName) + strlen(ind->surname) + 20));
	strcpy(ret, "{\"givenName\":\"");
	strcat(ret, ind->givenName);
	strcat(ret, "\",\"surname\":\"");
	strcat(ret, ind->surname);
	strcat(ret, "\"}");

	//printf("ind to json::%s\n", ret);
	
	
	return ret;
}

/** Function for creating an Individual struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and an Individual struct has been created
 *@return a newly allocated Individual struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
Individual* JSONtoInd(const char* str){
	if(str == NULL || strlen(str) == 0){
		return NULL;
	}


	int i = 0;
	int quoteCount = 0;
	int semiCount = 0;
	int commaCount = 0;
	int givCount = 0;
	int surCount = 0;
	int len = strlen(str);

	char givTemp[len];
	char surTemp[len];
	memset(givTemp, '\0', len);
	memset(surTemp, '\0', len);

	char myStr[len];
	strcpy(myStr, str);

	//"{"givenName":"William","surname":"Shakespeare"}"

	for(i = 0; i < len; i++){
		if(myStr[i] == '"'){
			quoteCount++;
		}
		else if(myStr[i] == ':'){
			semiCount++;
		}
		else if(myStr[i] == ','){
			commaCount++;
		}
		else if(quoteCount == 3){
			givTemp[givCount] = myStr[i];
			givCount++;
		}
		else if(quoteCount == 7){
			surTemp[surCount] = myStr[i];
			surCount++;
		}
	}

	if(semiCount != 2){
		return NULL;
	}
	if(commaCount != 1){
		return NULL;
	}
	if(quoteCount != 8){
		return NULL;
	}



	//printf("giv:%d, sur:%d\n", givCount, surCount);
	Individual* indi = initializeIndividual();
	if(givCount > 1){
		indi->givenName = (char*)malloc(sizeof(char)*givCount+2);
		strcpy(indi->givenName, givTemp);
	}
	else{
		indi->givenName = (char*)malloc(sizeof(char)*2);

		strcpy(indi->givenName, "");
	}

	if(surCount > 1){
		indi->surname = (char*)malloc(sizeof(char)*surCount+3);
		strcpy(indi->surname, surTemp);
		//strcat(indi->surname, "\r\n");
	}
	else{
		indi->surname = (char*)malloc(sizeof(char)*2);
		strcpy(indi->surname, "");
	}

	//printf("giv '%s'   sur '%s' \n", indi->givenName, indi->surname);


	return indi;
}

/** Function for creating a GEDCOMobject struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and a GEDCOMobject struct has been created
 *@return a newly allocated GEDCOMobject struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
GEDCOMobject* JSONtoGEDCOM(const char* str){
	if(str == NULL || strlen(str) == 0){
		return NULL;
	}


	int i = 0;
	int quoteCount = 0;
	int semiCount = 0;
	int commaCount = 0;
	int srcCount = 0;
	int gedcVCount = 0;
	int encodingCount = 0;
	int subNameCount = 0;
	int subAddressCount = 0;
	int len = strlen(str);

	char srcTemp[len];
	char gedcVTemp[len];
	char encodingTemp[len];
	char subNameTemp[len];
	char subAddressTemp[len];

	memset(srcTemp, '\0', len);
	memset(gedcVTemp, '\0', len);
	memset(encodingTemp, '\0', len);
	memset(subNameTemp, '\0', len);
	memset(subAddressTemp, '\0', len);

	char myStr[len];
	strcpy(myStr, str);

	//"{"source":"Blah","gedcVersion":"5.5","encoding":"ASCII","subName":"Some dude","subAddress":"nowhere"}."

	for(i = 0; i < len; i++){
		if(myStr[i] == '"'){
			quoteCount++;
		}
		else if(myStr[i] == ':'){
			semiCount++;
		}
		else if(myStr[i] == ','){
			commaCount++;
		}
		else if(quoteCount == 3){
			srcTemp[srcCount] = myStr[i];
			srcCount++;
		}
		else if(quoteCount == 7){
			gedcVTemp[gedcVCount] = myStr[i];
			gedcVCount++;
		}
		else if(quoteCount == 11){
			encodingTemp[encodingCount] = myStr[i];
			encodingCount++;
		}
		else if(quoteCount == 15){
			subNameTemp[subNameCount] = myStr[i];
			subNameCount++;
		}
		else if(quoteCount == 19){
			subAddressTemp[subAddressCount] = myStr[i];
			subAddressCount++;
		}
	}

	if(semiCount != 5){
		return NULL;
	}
	if(commaCount != 4){
		return NULL;
	}
	if(quoteCount != 20){
		return NULL;
	}
	if(srcCount == 0){
		return NULL;
	}
	if(gedcVCount == 0){
		return NULL;
	}
	if(encodingCount == 0){
		return NULL;
	}
	if(quoteCount != 20){
		return NULL;
	}

	//printf("giv:%d, sur:%d\n", givCount, surCount);
	
	GEDCOMobject* obj = (GEDCOMobject*)malloc(sizeof(GEDCOMobject));

	obj->families = initializeList(printFamily, deleteFamily, compareFamilies);
	obj->individuals = initializeList(printIndividual, deleteIndividual, compareFamilies);


	Submitter* sub = (Submitter*)malloc(sizeof(Submitter)+1);
	strcpy(sub->submitterName, subNameTemp);
	strcpy(sub->address, subAddressTemp);
	sub->otherFields = initializeList(printField, deleteField, compareFields);

	obj->submitter = sub;


	Header* head = (Header*)malloc(sizeof(Header));

	strcpy(head->source, srcTemp);
	head->gedcVersion = atof(gedcVTemp);

	if(strcmp(encodingTemp, "ASCII") == 0){
		head->encoding = ASCII;
	}
	else if(strcmp(encodingTemp, "ANSEL") == 0){
		head->encoding = ANSEL;
	}
	else if(strcmp(encodingTemp, "UTF-8") == 0){
		head->encoding = UTF8;
	}
	else if(strcmp(encodingTemp, "UNICODE") == 0){
		head->encoding = UNICODE;
	}
	else{
		return NULL;
	}

	head->submitter = sub;
	head->otherFields = initializeList(printField, deleteField, compareFields);

	obj->header = head;

	//printf("giv '%s'   sur '%s' \n", indi->givenName, indi->surname);
	return obj;
}

/** Function for adding an Individual to a GEDCCOMobject
 *@pre both arguments are not NULL and valid
 *@post Individual has not been modified in any way, and its address had been added to GEDCOMobject's individuals list
 *@return void
 *@param obj - a pointer to a GEDCOMobject struct
 *@param toBeAdded - a pointer to an Individual struct
**/
void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded){
	if(toBeAdded == NULL){
		return;
	}
	if(obj == NULL){
		return;
	}
	Individual* temp = (Individual*)toBeAdded;
	insertBack(&obj->individuals, temp);

	return;
}

/** Function for converting a list of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param iList - a pointer to a list of Individual structs
 **/
char* iListToJSON(List iList){

	char tempi[2000];
	char* string;
	memset(tempi, '\0', 2000);

	ListIterator iter = createIterator(iList);
	
	Individual* found = (Individual*)nextElement(&iter);

	strcpy(tempi, indToJSON(found));
	//char* temp2 = indToJSON(found);
	string = (char*)malloc(sizeof(char)* (strlen(tempi) + 4));
	strcpy(string, "[");
	strcat(string, tempi);

	//free(temp2);


	if(strlen(string) == 1){
		string = (char*)realloc(string, sizeof(char)*4);
		strcat(string, "]");
		return string;
	}
	found = (Individual*)nextElement(&iter);

	while(found != NULL){
		memset(tempi, '\0', 2000);

		strcpy(tempi, indToJSON(found));

		string = (char*)realloc(string, (sizeof(char)* (strlen(tempi) + strlen(string) + 10)));

		strcat(string, ",");
		strcat(string, tempi);

		found = (Individual*)nextElement(&iter);
	}

	strcat(string, "]");

	//printf("%s\n", string);

	return string;
}

/** Function for converting a list of lists of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param gList - a pointer to a list of lists of Individual structs
 **/
char* gListToJSON(List gList){
	//printf("gListToJSON called\n");
	char temp[3000];
	char* string;
	char* str;
	//int len = 0;
	memset(temp, '\0', 3000);
	//printf("hi1\n");
	ListIterator iter = createIterator(gList);
	List* found = (List*)nextElement(&iter);
	//printf("hi2\n");
	if(found == NULL){
		str = (char*)malloc(sizeof(char)*3);
		strcat(str, "[]");
		return str;
	}
	//printf("here\n");
	strcpy(temp, iListToJSON(*found));
	string = (char*)malloc(sizeof(char) * (strlen(temp) + 3));
	//printf("hi3\n");
	strcpy(string, "[");
	strcat(string, temp);
	//strcpy(string, "\n");
	//printf("first\n\n%s\n\n", string);
	found = (List*)nextElement(&iter);
	while(found != NULL){
		strcat(string, ",");
		memset(temp, '\0', 3000);
		strcpy(temp, iListToJSON(*found));
		string = (char*)realloc(string, sizeof(char)* (strlen(temp) + strlen(string) + 3));
		//strcat(string, ",");
		strcat(string, temp);
		found = (List*)nextElement(&iter);
	}
	strcat(string, "]");

	//printf("%s\n", string);

	return string;
}


//****************************************** List helper functions added for A2 *******************************************
void deleteGeneration(void* toBeDeleted){

	return;
}

int compareGenerations(const void* first,const void* second){

	return 0;
}

char* printGeneration(void* toBePrinted){

	char* hi = "hi generation\n";
	return hi;
}






//************************************************************************************************************

//****************************************** List helper functions *******************************************

//free the records passed to them
void deleteEvent(void* toBeDeleted){
	return;
}

//returns 0 if dates are same -1 if first argument has an earlier date, 1 if first has a later date than second
//if one of the events being compared has no date perform a lexigraphical comparison using type field
int compareEvents(const void* first,const void* second){
	if(first == NULL || second == NULL){
		return 0;
	}
	Event* one = (Event*)first;
	Event* two = (Event*)second;
	if(one->date == NULL || two->date == NULL){
		//printf("got him\n");
		//cmp by type then return
		if(strcmp(one->type, two->type) < 0){
			return -1;
		}
		if(strcmp(one->type, two->type) == 0){
			return 0;
		}
		if(strcmp(one->type, two->type) > 0){
			return 1;
		}
	}else{
		int m1, m2, day1, day2, year1, year2, case1 = 5, case2 = 5;
		m1 = 20;
		m2 = 20;
		day1 = 0;
		day2 = 0;
		year1 = 0;
		year2 = 0;
		char month1[3];
		char month2[3];
		char xtra1[3];
		char xtra2[3];

		memset(month1, '\0', 3);
		memset(month2, '\0', 3);
		memset(xtra1, '\0', 3);
		memset(xtra2, '\0', 3);
		//4 cases for date: day month year || [month|other] year || other day month year || year
		//case 1 straight forward, case 2 handled by cmpMonth, case 3 ...

	//for date 1
		//case 1
		if(strlen(one->date) == 11){
			sscanf( one->date, "%d %s %d", &day1, month1, &year1);
			m1 = getMonth(month1);
			case1 = 3;
		}
		//case 2
		else if(strlen(one->date) == 8){
			sscanf( one->date, "%s %d", month1, &year1);
			m1 = getMonth(month1);
			case1 = 2;
		}
		//case 3
		else if(strlen(one->date) == 15){
			sscanf( one->date, "%s %d %s %d", xtra1, &day1, month1, &year1);
			m1 = getMonth(month1);
			case1 = 4;
		}
		//case 4
		else if(strlen(one->date) == 4){
			sscanf( one->date, "%d", &year1);
			case1 = 1;

		}

	//for date 2
		//case 1
		if(strlen(two->date) == 11){
			sscanf( two->date, "%d %s %d", &day2, month2, &year2);
			m2 = getMonth(month2);
			case2 = 3;
		}
		//case 2
		else if(strlen(two->date) == 8){
			sscanf( two->date, "%s %d", month2, &year2);
			m2 = getMonth(month2);
			case2 = 2;
		}
		//case 3
		else if(strlen(two->date) == 15){
			sscanf( two->date, "%s %d %s %d", xtra2, &day2, month2, &year2);
			m2 = getMonth(month2);
			case2 = 4;
		}
		//case 4
		else if(strlen(two->date) == 4){
			sscanf( two->date, "%d", &year2);
			case2 = 1;
		}

		//printf("month 1 is: %d, month 2 is: %d\n", m1, m2);

		if(case1 == 1 || case2 == 1){
			//printf("case1 ");
			if(year1 < year2){
				return -1;
			}
			if(year1 > year2){
				return 1;
			}
			if(year1 == year2){
				if(case1 == case2){
					return 0;
				}
				if(case1 < case2){
					return -1;
				}
				if(case1 > case2){
					return 1;
				}
			}
		}
		else if(case1 == 2 || case2 == 2){
			//printf("case2 ");
			if(year1 < year2){
				return -1;
			}
			if(year1 > year2){
				return 1;
			}
			if(year1 == year2){
				if(m1 < m2){
					return -1;
				}
				if(m1 > m2){
					return 1;
				}
				if(m1 == m2){
					if(case1 == case2){
						return 0;
					}
					if(case1 < case2){
						return -1;
					}
					if(case1 > case2){
						return 1;
					}
				}
			}
		}
		else if(case1 == 3 || case2 == 3){
			//printf("case3 ");
			if(year1 < year2){
				return -1;
			}
			if(year1 > year2){
				return 1;
			}
			if(year1 == year2){
				if(m1 < m2){
					return -1;
				}
				if(m1 > m2){
					return 1;
				}
				if(m1 == m2){
					if(day1 < day2){
						return -1;
					}
					if(day1 > day2){
						return 1;
					}
					if(day1 == day2){
						if(case1 == case2){
							return 0;
						}
						if(case1 < case2){
							return -1;
						}
						if(case1 > case2){
							return 1;
						}
					}
				}
			}
		}
		else if (case1 == 4 || case2 == 4){
			//printf("case4 ");
			if(year1 < year2){
				return -1;
			}
			if(year1 > year2){
				return 1;
			}
			if(year1 == year2){
				if(m1 < m2){
					return -1;
				}
				if(m1 > m2){
					return 1;
				}
				if(m1 == m2){
					if(day1 < day2){
						return -1;
					}
					if(day1 > day2){
						return 1;
					}
					if(day1 == day2){
						//int ret;
						if(strcmp(xtra1, xtra2) == 0){
							if(case1 == case2){
							return 0;
							}
							if(case1 < case2){
								return -1;
							}
							if(case1 > case2){
								return 1;
							}
						}
						if(strcmp(xtra1, xtra2) > 0){
							return 1;
						}
						if(strcmp(xtra1, xtra2) < 0){
							return -1;
						}
					}
				}
			}
		}
		else{

			//printf("Warning: this code should not execute\n");
			return 0;
		}
	}
	//printf("Warning: this code should not execute\n");
			return 0;
}

//return a newly allocated string with a humanly readable representation of each record
char* printEvent(void* toBePrinted){
	char* str = "return";
	return str;
}

//free the records passed to them
void deleteIndividual(void* toBeDeleted){

	return;
}

//lexicographic comparison by "givenName, Surname" ie. "Buggs, Bunny"
int compareIndividuals(const void* first,const void* second){
	if(first == NULL|| second == NULL){
		return 0;
	}
	Individual* one = (Individual*) first;
	Individual* two = (Individual*) second;
	
	char* str1 = (char*)malloc(sizeof(char)*(strlen(one->givenName)+strlen(one->surname)+4));

	if(strlen(one->surname) != 0){
		strcpy(str1, one->surname);
		strcat(str1, ", ");
		strcat(str1, one->givenName);

	}
	else{
		strcpy(str1, one->givenName);
		strcat(str1, ", ");
		strcat(str1, one->surname);
	}

	

	char* str2 = (char*)malloc(sizeof(char)*(strlen(two->givenName)+strlen(two->surname)+4));
	

	if(strlen(two->surname) != 0){
		strcpy(str2, two->surname);
		strcat(str2, ", ");
		strcat(str2, two->givenName);
	}
	else{
		strcpy(str2, two->givenName);
		strcat(str2, ", ");
		strcat(str2, two->surname);
	}





	//printf("first string:%s\nsecond string:%s\n", str1, str2);

	int cmp = strcmp(str1, str2); 
	if(cmp < 0){
		cmp = -1;
	}
	else if(cmp == 0){
		cmp = 0;
	}
	else if(cmp > 0){
		cmp = 1;
	}

	free(str1);
	free(str2);
	return cmp;
}

//return a newly allocated string with a humanly readable representation of each record
char* printIndividual(void* toBePrinted){
	Individual* indi = (Individual*) toBePrinted;
	char* str = (char*)malloc(sizeof(char) * (strlen(indi->givenName) + strlen(indi->surname) + 3));
	strcpy(str, indi->givenName);
	strcat(str, ", ");
	strcat(str, indi->surname);
	//printf("in function %s\n", str);
	return str;
}

//free the records passed to them
void deleteFamily(void* toBeDeleted){
	return;
}

//compare number of individuals in family return -1 if first family has fewer members 0 if they have the same and 1 if first has more members
int compareFamilies(const void* first,const void* second){
	if(first == NULL|| second == NULL){
		return 0;
	}
	Family* one = (Family*) first;
	Family* two = (Family*) second;
	int lenOne = 0;
	int lenTwo = 0;
	int ret = 0;

	if(one->husband != NULL){
		lenOne++;
	}
	if(one->wife != NULL){
		lenOne++;
	}
	if(two->husband != NULL){
		lenTwo++;
	}
	if(two->wife != NULL){
		lenTwo++;
	}

	lenOne = lenOne + getLength(one->children);
	lenTwo = lenTwo + getLength(two->children);


	if(lenOne < lenTwo){
		ret = -1;
	}
	if(lenOne == lenTwo){
		ret = 0;
	}
	if(lenOne > lenTwo){
		ret = 1;
	}

	return ret;
}

//return a newly allocated string with a humanly readable representation of each record
char* printFamily(void* toBePrinted){
	char* str = "return";
	return str;
}

//free the records passed to them
void deleteField(void* toBeDeleted){
	return;
}
int compareFields(const void* first,const void* second){
	return 0;
}

//return a newly allocated string with a humanly readable representation of each record
char* printField(void* toBePrinted){
	char* str = "return";
	return str;
}

void deleteList(void* toBeDeleted){
	return;
}

int compareLists(const void* first,const void* second){
	return 0;
}

char* printList(void* toBePrinted){
	char* ret = "hi";
	return ret;
}



//************************************************************************************************************



 
