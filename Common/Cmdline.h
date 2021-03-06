/*
 Legal Notice: The source code contained in this file has been derived from
 the source code of Encryption for the Masses 2.02a, which is Copyright (c)
 Paul Le Roux and which is covered by the 'License Agreement for Encryption
 for the Masses'. Modifications and additions to that source code contained
 in this file are Copyright (c) TrueCrypt Foundation and are covered by the
 TrueCrypt License 2.3 the full text of which is contained in the file
 License.txt included in TrueCrypt binary and source code distribution
 packages. */

#define HAS_ARGUMENT	1
#define HAS_NO_ARGUMENT !HAS_ARGUMENT

typedef struct argument_t
{
	char long_name[16];
	char short_name[8];
} argument;

typedef struct argumentspec_t
{
	argument *args;
	int		 arg_cnt;
} argumentspec;

BOOL CALLBACK CommandHelpDlgProc ( HWND hwndDlg , UINT msg , WPARAM wParam , LPARAM lParam );
int Win32CommandLine ( char *lpszCommandLine , char ***lpszArgs );
int GetArgSepPosOffset ( char *lpszArgument );
int GetArgumentID ( argumentspec *as , char *lpszArgument , int *nArgPos );
int GetArgumentValue ( char **lpszCommandLineArgs , int nArgPos , int *nArgIdx , int nNoCommandLineArgs , char *lpszValue , int nValueSize );
