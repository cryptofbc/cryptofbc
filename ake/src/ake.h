#pragma once

#include <windows.h>
#include "plugin.h"

#define defMAX_DATA_SIZE        8 * 1024 * 1024
#define defMAX_DISPLAY_SIZE     4 * defMAX_DATA_SIZE

typedef enum _tagENUM_ALIGN_TYPE
{
    enumAlignByte = 0,
    enumAlignWord,
    enumAlignDword,
    enumAlignMaxcount
} ENUM_ALIGN_TYPE;

typedef enum _tagENUM_FORMAT_TYPE
{
    enumFormatCarray = 0,
    enumFormatHex,
    enumFormatBigint,
    enumFormatMaxcount
} ENUM_FORMAT_TYPE;

typedef struct _tagOUTPUT_FORMAT_INFO
{
    int nAlignSize;
    int nCount;
    wchar_t wszFormat1[64];
    wchar_t wszFormat2[16];
    wchar_t wszFormat3[32];
} OUTPUT_FORMAT_INFO, *POUTPUT_FORMAT_INFO;

void AkeNewDumpWindow();

void AkeInitDialog(HWND hWndDlg);

void FormatSelectMemory(ENUM_ALIGN_TYPE alignType, ENUM_FORMAT_TYPE formatType);

void FormatCArray(void* pBuff, DWORD dwSize, ENUM_ALIGN_TYPE alignType);

void FormatHexString(void* pBuff, DWORD dwSize, BOOL bLittleEdian);

void SetOutput(HWND hWndDlg);

void CopyOutputToClipbaord(HWND hWndEdit);

static int Mnewdump(t_table *pt,wchar_t *name,ulong index,int mode);

void AkeNewSelectDump();

INT_PTR CALLBACK AkeDialogProcCopySelData(HWND hwndDlg,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam
                                          );

INT_PTR CALLBACK AkeDialogProcNewSelection(HWND hwndDlg,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam
                                          );

void AkeInitSelectDialog(HWND hWndDlg);

void AkeShowSelectionWindow(HWND hWndDlg);
