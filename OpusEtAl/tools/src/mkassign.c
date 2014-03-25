/* M K A S S I G N . C */
/* given:
	onnames.lst (a list of NETNAMEs among which to distribute tests)
	onoper.lst  (a list of tests to be assigned)
	onspec.lst  (a list of NETNAMEs with special assignments (in finished form))
	generate:
	onassign.bat (a batchfile that will have the users execute the tests)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define fTrue 1
#define fFalse 0

#define ichMaxLine 256

FILE *fpNames = NULL;
FILE *fpOper = NULL;
FILE *fpSpec = NULL;
FILE *fpAssign = NULL;

FILE *fopen();

int rgnTest[2000];
int cTests, cNames, cTestsEach, iName = -1, cTestsCur;

main(cArg, rgszArg)
int cArg;
char *rgszArg[];
{
	if ((fpNames = fopen("ONNAMES.LST", "rt")) == NULL)
		Error ("Cannot open ONNAMES.LST");

	if ((fpOper = fopen("ONOPER.LST", "rt")) == NULL)
		Error ("Cannot open ONOPER.LST");

	if ((fpSpec = fopen("ONSPEC.LST", "rt")) == NULL)
		Error ("Cannot open ONSPEC.LST");

	if ((fpAssign = fopen("ONASSIGN.BAT", "wt")) == NULL)
		Error ("Cannot create ONASSIGN.BAT");

	fprintf(fpAssign, "rem  GENERATED FILE -- DO NOT EDIT\n");
	fprintf(fpAssign, "rem  run MKASSIGN.EXE to generate\n");

	ReadTests();
	CountNames();
	/* average - 3.5 executions per test (then gets truncated) */
	cTestsEach = (cTests * 35) / (cNames * 10);

	if (cTestsEach < 2 || cTests < 2 || cNames < 2)
		Error("number of tests or names too small");

	fprintf(stdout, "Assigning %d tests to %d names, %d each (2 x %d, 3 x %d, 4 x %d)\n",
			cTests, cNames, cTestsEach, max(0,(cTests*3)-(cNames*cTestsEach)),
			cTests - (max(0,(cTests*3)-(cNames*cTestsEach)) + max(0,
			(cNames*cTestsEach)-(cTests*3))),
			max(0, (cNames*cTestsEach)-(cTests*3)));

	fprintf(fpAssign, "\nrem Assigning %d tests to %d names, %d each (2 x %d, 3 x %d, 4 x %d)\n",
			cTests, cNames, cTestsEach, max(0,(cTests*3)-(cNames*cTestsEach)),
			cTests - (max(0,(cTests*3)-(cNames*cTestsEach)) + max(0,
			(cNames*cTestsEach)-(cTests*3))),
			max(0, (cNames*cTestsEach)-(cTests*3)));

	fprintf(fpAssign, "\nrem  Generated from ONNAMES.LST and ONOPER.LST:\n");

	NextName();
	AssignTests();

	fprintf(fpAssign, "\n\nrem  Copied from ONSPEC.LST:\n\n");

	while (!feof(fpSpec))
		{
		char rgchLine[128];
		fgets(rgchLine, 128, fpSpec);
		fputs(rgchLine, fpAssign);
		}

	fprintf(stdout, "\n");

	fcloseall();
}


/* assignment order:
	assign all tests once in ascending order
	assign even tests in ascending order (to fill out last name)
	assign all tests once in decending order
	assign all odd tests in decending order
	assign remaining even tests in ascending order
	every third starting with third, ascending
	every third starting with second, ascending
	every third starting with first, ascending
	test 1001, once per user
*/
AssignTests()
{
	int iTestCur, iTestEven = 0;

	fprintf(stdout, "Pass 1: iName = %d; ", iName);

/* Pass 1 */
	/* all in assending order */
	for (iTestCur = 0; iTestCur < cTests; iTestCur++)
		Assign(rgnTest[iTestCur]);

/* Pass 3, part 1 */
	while (cTestsCur && iTestEven < cTests)
		{
		Assign(rgnTest[iTestEven]);
		iTestEven += 2;
		}

	fprintf(stdout, "2: %d; ", iName);

/* Pass 2 */
	/* all in descending order */
	for (iTestCur = cTests-1; iTestCur >= 0; iTestCur--)
		Assign(rgnTest[iTestCur]);

	fprintf(stdout, "3: %d; ", iName);

/* Pass 3, part 2 */
	/* all odd tests in decending order */
	for (iTestCur = cTests - 1 - (cTests&1); iTestCur >= 1; iTestCur -= 2)
		Assign(rgnTest[iTestCur]);

	/* remaining even tests in ascending order */
	while (iTestEven < cTests)
		{
		Assign(rgnTest[iTestEven]);
		iTestEven += 2;
		}

	fprintf(stdout, "4: %d; ", iName);

/* Pass 4 */
	/* every third starting with third */
	for (iTestCur = 2; iTestCur < cTests; iTestCur += 3)
		Assign(rgnTest[iTestCur]);

	/* every third starting with second */
	for (iTestCur = 1; iTestCur < cTests; iTestCur += 3)
		Assign(rgnTest[iTestCur]);

	/* every third starting with first */
	for (iTestCur = 0; iTestCur < cTests; iTestCur += 3)
		Assign(rgnTest[iTestCur]);

	fprintf(stdout, "5: %d; ", iName);

/* Pass 5 */
	/* every forth starting with forth */
	for (iTestCur = 3; iTestCur < cTests; iTestCur += 4)
		Assign(rgnTest[iTestCur]);

	/* every forth starting with third */
	for (iTestCur = 2; iTestCur < cTests; iTestCur += 4)
		Assign(rgnTest[iTestCur]);

	/* every forth starting with second */
	for (iTestCur = 1; iTestCur < cTests; iTestCur += 4)
		Assign(rgnTest[iTestCur]);

	/* every forth starting with first */
	for (iTestCur = 0; iTestCur < cTests; iTestCur += 4)
		Assign(rgnTest[iTestCur]);

	fprintf(stdout, "6: %d; ", iName);

/* Pass 6 */
	/* every fifth starting with fifth */
	for (iTestCur = 4; iTestCur < cTests; iTestCur += 5)
		Assign(rgnTest[iTestCur]);

	/* every fifth starting with forth */
	for (iTestCur = 3; iTestCur < cTests; iTestCur += 5)
		Assign(rgnTest[iTestCur]);

	/* every fifth starting with third */
	for (iTestCur = 2; iTestCur < cTests; iTestCur += 5)
		Assign(rgnTest[iTestCur]);

	/* every fifth starting with second */
	for (iTestCur = 1; iTestCur < cTests; iTestCur += 5)
		Assign(rgnTest[iTestCur]);

	/* every fifth starting with first */
	for (iTestCur = 0; iTestCur < cTests; iTestCur += 5)
		Assign(rgnTest[iTestCur]);


/* remaining names are assigned 1001 */
	if (cTestsCur)
		NextName();

	while (iName < cNames)
		{
		Assign(1001);
		NextName();
		}
}


Assign(n)
int n;
{
	if (iName >= cNames)
		return;

	fprintf (fpAssign, " %d", n);

	if (++cTestsCur >= cTestsEach)
		NextName();
}


NextName()
{
	char szName[20];

	if (++iName >= cNames)
		return;

	do
		fscanf(fpNames, "%20s\n", szName);
	while (*szName < 'A' || *szName > 'Z');

	fprintf(fpAssign, "\n  if \"%%NETNAME%%\"==\"%s\" test ONSETUP", szName);

	cTestsCur = 0;
}


ReadTests()
{
	int nTest;
	char szLine[256];

	while (!feof(fpOper))
		{
		szLine[0] = 0;
		fgets(szLine, 256, fpOper);
		if (*szLine != '\'' && (nTest = atoi(szLine)) >= 1000)
			{
			if (cTests >= 2000)
				Error("too many tests");
			rgnTest[cTests++] = nTest;
			}
		}
}


CountNames()
{
	char szName[20];

	while (!feof(fpNames))
		{
		fscanf(fpNames, "%20s\n", szName);
		if (*szName >= 'A' && *szName <= 'Z')
			cNames++;
		}
	fseek(fpNames, 0L, SEEK_SET);
}


Error(sz)
char *sz;
{
	fprintf(stderr, "mkassign: error: %s\n", sz);
	exit(1);
}


