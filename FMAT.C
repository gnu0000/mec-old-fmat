/* fmat.c */

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "csv.h"

#define STRSIZE        80
#define FIELDSIZE      512
#define FILENAMEBASE   "DESDD"
#define FILENAMEEXT    ".RTF"
#define HEADERFILENAME "Header.rtf"
#define ENTRYFILENAME  "Entry.rtf"
#define FIELDCOUNT     12

typedef char   *PSZ;

int   iEntityCount = 0;

/***********************************************************************/


void Error (PSZ pszStr1, PSZ pszStr2)
   {
   printf ("Error: %s %s\n", pszStr1, pszStr2);
   exit (1);
   }


/* force 3 decimal places */
PSZ itoa2 (int iNum,PSZ pszStr)
   {
   PSZ pszTmp = pszStr;

   *pszTmp++ = (char) ('0' + iNum / 100);
   *pszTmp++ = (char) ('0' + (iNum % 100) / 10);
   *pszTmp++ = (char) ('0' + iNum % 10);
   *pszTmp = '\0';
   return pszStr;
   }




/* returns 0 on blank line */
int ReadLine (FILE *fpIn, PSZ pszStr)
   {
   int c;
   PSZ pszTmpPtr = pszStr;

   for (c = getc (fpIn); c !='\n' && c != EOF; c = getc (fpIn))
      *(pszTmpPtr++) = (char) c;
   *pszTmpPtr = '\0';
   return (*pszStr != '\0');
   }


/* null terminated buffer */
PSZ LoadBuffer (PSZ pszFileName)
   {
   FILE  *fpIn;
   int   iHandle, i;
   unsigned  uFileLength;
   PSZ pszBuffPtr;

   fpIn = fopen (pszFileName, "r");
   if (fpIn == NULL)
      Error ("Unable to open template file", pszFileName);
   iHandle = fileno (fpIn);
   uFileLength = (unsigned) filelength (iHandle);
   pszBuffPtr = (PSZ) calloc (1,uFileLength + 2);
   if (pszBuffPtr == NULL)
      Error ("Unable to allocate buffer for template", pszFileName);
   if (!(i = fread (pszBuffPtr, 1, uFileLength, fpIn)))
       Error ("Unable to read full template", pszFileName);
   for (; i < uFileLength +1; i++)
      pszBuffPtr[i] = '\0';
   fclose (fpIn);
   return pszBuffPtr;
   }



void XlateBuffer (FILE *fpOut, char pszFieldStr[FIELDCOUNT][FIELDSIZE], PSZ pszBuffPtr)
   {
   int   c;

   while ((c = *pszBuffPtr++) != '\0')
      {
      if (c == '<')
         {
         c = *pszBuffPtr++;
         if (c == '#') 
            fprintf (fpOut, "2.%d", iEntityCount);
         else if ( c < 'A' || c >= 'A' + FIELDCOUNT)
            Error ("Illegal <x> value specified","");
         else
            fprintf (fpOut, "%s", pszFieldStr [c - 'A']);
         if ((c = *pszBuffPtr++) != '>')
            Error ("'>' expected in template","");
         }
      else
         fputc (c, fpOut);
      }
   }



main (int argc, PSZ argv[])
   {
   char     szFieldStr  [FIELDCOUNT][FIELDSIZE];
   char     szCurrentEntity [FIELDSIZE];
   char     szCSVLine [FIELDSIZE * 4];
   char     szOutFile [STRSIZE];
   char     szTmp [STRSIZE];
   PSZ      pszHeaderBuffer;
   PSZ      pszEntryBuffer;
   FILE     *fpOut;
   FILE     *fpDD;
   int      i;

   szCurrentEntity[0] ='\0';  
   if (argc != 2)
      Error ("\nUSAGE: FMAT DataDictionaryName\n","");

   pszHeaderBuffer = LoadBuffer (HEADERFILENAME);
   pszEntryBuffer  = LoadBuffer (ENTRYFILENAME);

   fpDD = fopen (argv[1], "r");
   if (fpDD == NULL)
      Error ("Can't open file", argv[1]);

   /* skip fields description line */
   ReadLine (fpDD, szCSVLine);

   while (ReadLine (fpDD, szCSVLine))
      {
      for (i = 0; i < FIELDCOUNT; i++)
         {
         GetCSVField (i+1, szCSVLine, szFieldStr[i]);
         }
      if (strcmp (szFieldStr[1], szCurrentEntity) != 0)
         {
         ++iEntityCount;
         if (szCurrentEntity[0] != '\0')
            fclose (fpOut);
         strcpy (szCurrentEntity, szFieldStr[1]);
         strcpy (szOutFile, FILENAMEBASE);
         strcat (szOutFile, itoa2 (iEntityCount, szTmp));
         strcat (szOutFile, FILENAMEEXT);
         fpOut = fopen (szOutFile, "w");
         if (fpOut == NULL)
            Error ("Unable to open output file",szOutFile);
         else
            printf ("Processing %s to %s\n", szCurrentEntity, szOutFile);
         XlateBuffer (fpOut, szFieldStr, pszHeaderBuffer);
         }
      XlateBuffer (fpOut, szFieldStr, pszEntryBuffer);
      }
   fputc ('}', fpOut);
   fclose (fpOut);
   fclose (fpDD);
   printf ("\nDone.\n");
   return 0;
   }
