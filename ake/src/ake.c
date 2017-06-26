#define _CRT_SECURE_NO_DEPRECATE

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <winnt.h>                     // Only if you call ODBG2_Pluginmainloop
#include "resource.h"
#include "ake.h"

#include "plugin.h"

#define PLUGINNAME     L"AKE"    // Unique plugin name
#define VERSION        L"2017.6.22.1"      // Plugin version
#define defDEFAULT_DUMP_SIZE        0x100

HINSTANCE        hdllinst;             // Instance of plugin DLL

t_dump g_OllyCpuDump;
void* g_pMemoryData;
wchar_t* g_pwDisplayBuffer;

wchar_t* g_pwFormat[enumAlignMaxcount] = {
    L"unsigned char pData[%d] = \r\n{",
    L"unsigned short pwData[%d] = \r\n{",
    L"unsigned long pdwData[%d] = \r\n{"
};

OUTPUT_FORMAT_INFO g_FormatInfo[enumAlignMaxcount] = {
    {
        sizeof(unsigned char),
        16,
        L"unsigned char pData[%d] = \r\n{",
        L"0x%02X, ",
        L"0x%02X \r\n};\r\n",
    },
    {
        sizeof(unsigned short),
        8,
        L"unsigned short pwData[%d] = \r\n{",
        L"0x%04X, ",
        L"0x%04X \r\n};\r\n",
    },
    {
        sizeof(unsigned long),
        4,
        L"unsigned long pdwData[%d] = \r\n{",
        L"0x%08X, ",
        L"0x%08X \r\n};\r\n",
    }
};

#pragma comment(lib, "ollydbg.lib")

void AkeNewDumpWindow(DWORD dwStart, DWORD dwEnd)
{
    static int g_nWinCount = 1;
    wchar_t wszTitle[TEXTLEN] = { 0 };
    t_control* pHollyCtrl = NULL;

    Swprintf(wszTitle, L"%s Dump Window %d", PLUGINNAME, g_nWinCount++);

    Createdumpwindow(wszTitle,
                     g_OllyCpuDump.base, 
                     g_OllyCpuDump.size ? g_OllyCpuDump.size : defDEFAULT_DUMP_SIZE,
                     NULL, 
                     g_OllyCpuDump.dumptype ? g_OllyCpuDump.dumptype : DUMP_HEXA16,
                     //g_OllyCpuDump.sel0,
                     //g_OllyCpuDump.sel1,
                     dwStart,
                     dwEnd,
                     NULL);        
}

INT_PTR CALLBACK AkeDialogProcNewSelection(HWND hwndDlg,
                                           UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam
                                           )
{
    INT_PTR nRet = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG: 
        AkeInitSelectDialog(hwndDlg);
        nRet = TRUE;        
        break;
    case WM_COMMAND:
        switch (HIWORD(wParam))
        {        
        case BN_CLICKED:
            if ( IDC_BUTTON_SHOW == LOWORD(wParam) )
            {                
                AkeShowSelectionWindow(hwndDlg);
                SendMessage(hwndDlg, WM_CLOSE, 0, 0);
            }
            break;
        }
        nRet = TRUE;
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        nRet = TRUE;
        break;
    }
    return FALSE;
}

void AkeInitSelectDialog(HWND hWndDlg)
{
    wchar_t wszTitle[TEXTLEN] = { 0 };
    Swprintf(wszTitle, L"0x%08x", g_OllyCpuDump.sel0);
    SetDlgItemText(hWndDlg, IDC_EDIT_START, wszTitle);

    ZeroMemory(wszTitle, sizeof(wszTitle));
    Swprintf(wszTitle, L"0x%x", g_OllyCpuDump.sel1 - g_OllyCpuDump.sel0);
    SetDlgItemText(hWndDlg, IDC_EDIT_SELSIZE, wszTitle);
}

void AkeShowSelectionWindow(HWND hWndDlg)
{
    DWORD dwStartAddress = 0;
    DWORD dwSize = 0;
    wchar_t wszBuff[TEXTLEN] = { 0 };

    GetDlgItemText(hWndDlg, IDC_EDIT_START, wszBuff, TEXTLEN);
    swscanf_s(wszBuff, L"0x%08x", &dwStartAddress);

    ZeroMemory(wszBuff, sizeof(wszBuff));
    GetDlgItemText(hWndDlg, IDC_EDIT_SELSIZE, wszBuff, TEXTLEN);
    swscanf_s(wszBuff, L"0x%x", &dwSize);

    AkeNewDumpWindow(dwStartAddress, dwStartAddress + dwSize);
}

void AkeNewSelectDump()
{
    HMODULE hMod = NULL;

    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)AkeDialogProcNewSelection, &hMod);

    DialogBoxParam(hMod, MAKEINTRESOURCE(IDD_DIALOG2), g_OllyCpuDump.table.hparent, AkeDialogProcNewSelection, (LPARAM)NULL);
}

void FormatSelectMemory(ENUM_ALIGN_TYPE alignType, ENUM_FORMAT_TYPE formatType)
{
    ulong ulSize = g_OllyCpuDump.sel1 - g_OllyCpuDump.sel0;
    ulong ulReaded = 0;

    if ( !g_pMemoryData )
    {
        goto Exit0;
    }

    if (ulSize > defMAX_DATA_SIZE)
    {
        goto Exit0;
    }

    ZeroMemory(g_pMemoryData, defMAX_DATA_SIZE);

    ulReaded = Readmemory(g_pMemoryData, g_OllyCpuDump.sel0, ulSize, MM_SILENT | MM_PARTIAL);
    if ( !ulReaded )
    {
        goto Exit0;
    }

    switch (formatType)
    {
    case enumFormatCarray:
        FormatCArray(g_pMemoryData, ulReaded, alignType);
    	break;
    case enumFormatHex:
        FormatHexString(g_pMemoryData, ulReaded, FALSE);
        break;
    case enumFormatBigint:
        FormatHexString(g_pMemoryData, ulReaded, TRUE);
        break;
    }

Exit0:
    return;
}

void FormatCArray(void* pBuff, DWORD dwSize, ENUM_ALIGN_TYPE alignType)
{
    int i = 0;
    DWORD j = 0;

    if ( !pBuff || !dwSize )
    {
        goto Exit0;
    }

    if ( !g_pwDisplayBuffer )
    {
        goto Exit0;
    }

    ZeroMemory(g_pwDisplayBuffer, defMAX_DISPLAY_SIZE);

    if ( dwSize % g_FormatInfo[alignType].nAlignSize != 0 )
    {
        dwSize /= g_FormatInfo[alignType].nAlignSize;
        dwSize++;
    }
    else
    {
        dwSize /= g_FormatInfo[alignType].nAlignSize;
    }

    i = Swprintf(g_pwDisplayBuffer, g_FormatInfo[alignType].wszFormat1, dwSize);

    switch (alignType)
    {
    case enumAlignByte:
        for ( j = 0; j < dwSize - 1; j++ )
        {
            if ( j % g_FormatInfo[alignType].nCount == 0 )
            {
                g_pwDisplayBuffer[i++] = L'\r';
                g_pwDisplayBuffer[i++] = L'\n'; 
            }        
            i += Swprintf(g_pwDisplayBuffer + i, g_FormatInfo[alignType].wszFormat2, *((unsigned char*)pBuff + j));
        }

        if ( j % g_FormatInfo[alignType].nCount == 0 )
        {
            g_pwDisplayBuffer[i++] = L'\r';
            g_pwDisplayBuffer[i++] = L'\n'; 
        } 
        Swprintf(g_pwDisplayBuffer + i, g_FormatInfo[alignType].wszFormat3, *((unsigned char*)pBuff + j));
    	break;
    case enumAlignWord:
        for ( j = 0; j < dwSize - 1; j++ )
        {
            if ( j % g_FormatInfo[alignType].nCount == 0 )
            {
                g_pwDisplayBuffer[i++] = L'\r';
                g_pwDisplayBuffer[i++] = L'\n'; 
            }        
            i += Swprintf(g_pwDisplayBuffer + i, g_FormatInfo[alignType].wszFormat2, *((unsigned short*)pBuff + j));
        }

        if ( j % g_FormatInfo[alignType].nCount == 0 )
        {
            g_pwDisplayBuffer[i++] = L'\r';
            g_pwDisplayBuffer[i++] = L'\n'; 
        } 
        Swprintf(g_pwDisplayBuffer + i, g_FormatInfo[alignType].wszFormat3, *((unsigned short*)pBuff + j));
        break;
    case enumAlignDword:
        for ( j = 0; j < dwSize - 1; j++ )
        {
            if ( j % g_FormatInfo[alignType].nCount == 0 )
            {
                g_pwDisplayBuffer[i++] = L'\r';
                g_pwDisplayBuffer[i++] = L'\n'; 
            }        
            i += Swprintf(g_pwDisplayBuffer + i, g_FormatInfo[alignType].wszFormat2, *((unsigned long*)pBuff + j));
        }

        if ( j % g_FormatInfo[alignType].nCount == 0 )
        {
            g_pwDisplayBuffer[i++] = L'\r';
            g_pwDisplayBuffer[i++] = L'\n'; 
        } 
        Swprintf(g_pwDisplayBuffer + i, g_FormatInfo[alignType].wszFormat3, *((unsigned long*)pBuff + j));
        break;
    default:
        break;
    }

Exit0:
    return;
}

void FormatHexString(void* pBuff, DWORD dwSize, BOOL bLittleEdian)
{
    DWORD i = 0;
    int j = 0;

    if ( !pBuff || !dwSize )
    {
        goto Exit0;
    }

    if (!g_pwDisplayBuffer)
    {
        goto Exit0;
    }

    ZeroMemory(g_pwDisplayBuffer, defMAX_DISPLAY_SIZE);

    if ( bLittleEdian )
    {
        for ( i = 0; i < dwSize; i++ )
        {
            if ( i % 16 == 0 )
            {
                g_pwDisplayBuffer[j++] = L'\r';
                g_pwDisplayBuffer[j++] = L'\n'; 
            }
            j += Swprintf(g_pwDisplayBuffer + j, L"%02X", *((unsigned char*)pBuff + dwSize - 1 - i));
        }
    }
    else
    {
        for ( i = 0; i < dwSize; i++ )
        {
            if ( i % 16 == 0 )
            {
                g_pwDisplayBuffer[j++] = L'\r';
                g_pwDisplayBuffer[j++] = L'\n'; 
            }
            j += Swprintf(g_pwDisplayBuffer + j, L"%02X", *((unsigned char*)pBuff + i));
        }
    }

Exit0:
    return;
}

void SetOutput(HWND hWndDlg)
{
    ENUM_ALIGN_TYPE alignType = SendDlgItemMessage(hWndDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0);
    ENUM_ALIGN_TYPE formatType = SendDlgItemMessage(hWndDlg, IDC_COMBO2, CB_GETCURSEL, 0, 0);
    FormatSelectMemory(alignType, formatType);
    if ( g_pwDisplayBuffer )
    {
        SetDlgItemText(hWndDlg, IDC_EDIT_OUTPUT, g_pwDisplayBuffer);
    } 
}

void CopyOutputToClipbaord(HWND hWndDlg)
{    
    SendDlgItemMessage(hWndDlg, IDC_EDIT_OUTPUT, EM_SETSEL, 0, -1);
    SendDlgItemMessage(hWndDlg, IDC_EDIT_OUTPUT, WM_COPY, 0, 0);
}

void AkeInitDialog(HWND hWndDlg)
{
    wchar_t wszTitle[TEXTLEN] = { 0 };
    Swprintf(wszTitle, L"0x%08x", g_OllyCpuDump.sel0);
    SetDlgItemText(hWndDlg, IDC_EDIT_ADDR, wszTitle);

    ZeroMemory(wszTitle, sizeof(wszTitle));
    Swprintf(wszTitle, L"0x%x", g_OllyCpuDump.sel1 - g_OllyCpuDump.sel0);
    SetDlgItemText(hWndDlg, IDC_EDIT_SIZE, wszTitle);

    SendDlgItemMessage(hWndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)L"Byte");
    SendDlgItemMessage(hWndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)L"Word");
    SendDlgItemMessage(hWndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)L"DWord");
    SendDlgItemMessage(hWndDlg, IDC_COMBO1, CB_SETCURSEL, (WPARAM)0, 0);

    SendDlgItemMessage(hWndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)L"C Array");
    SendDlgItemMessage(hWndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)L"Big Number");
    SendDlgItemMessage(hWndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)L"Hex String");
    SendDlgItemMessage(hWndDlg, IDC_COMBO2, CB_SETCURSEL, (WPARAM)0, 0);

    SetOutput(hWndDlg);  
}

INT_PTR CALLBACK AkeDialogProcCopySelData(HWND hwndDlg,
                            UINT uMsg,
                            WPARAM wParam,
                            LPARAM lParam
                            )
{
    INT_PTR nRet = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG: 
        AkeInitDialog(hwndDlg);
        nRet = TRUE;        
        break;
    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case CBN_SELCHANGE:
            SetOutput(hwndDlg);           
        	break;
        case BN_CLICKED:
            if ( IDC_BUTTON_SHOW == LOWORD(wParam) )
            {
                CopyOutputToClipbaord(hwndDlg);
            }
            break;
        }
        nRet = TRUE;
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        nRet = TRUE;
    	break;
    }
    return FALSE;
}

void AkeCopySelectData()
{
    HMODULE hMod = NULL;

    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)AkeDialogProcCopySelData, &hMod);

    DialogBoxParam(hMod, MAKEINTRESOURCE(IDD_DIALOG1), g_OllyCpuDump.table.hparent, AkeDialogProcCopySelData, (LPARAM)NULL);     
}

// Entry point of the plugin DLL. Many system calls require DLL instance
// which is passed to DllEntryPoint() as one of parameters. Remember it. Do
// not make any initializations here, preferrable way is to place them into
// ODBG_Plugininit() and cleanup in ODBG_Plugindestroy().
BOOL WINAPI DllEntryPoint(HINSTANCE hi,DWORD reason,LPVOID reserved) {
    if (reason==DLL_PROCESS_ATTACH)
        hdllinst=hi;                       // Mark plugin instance
    return 1;                            // Report success
};

// ODBG2_Pluginquery() is a "must" for valid OllyDbg plugin. It must check
// whether given OllyDbg version is correctly supported, and return 0 if not.
// Then it should fill plugin name and plugin version (as UNICODE strings) and
// return version of expected plugin interface. If OllyDbg decides that this
// plugin is not compatible, it will be unloaded. Plugin name identifies it
// in the Plugins menu. This name is max. 31 alphanumerical UNICODE characters
// or spaces + terminating L'\0' long. To keep life easy for users, name must
// be descriptive and correlate with the name of DLL. Parameter features is
// reserved for the future. I plan that features[0] will contain the number
// of additional entries in features[]. Attention, this function should not
// call any API functions: they may be incompatible with the version of plugin!
extc int __cdecl ODBG2_Pluginquery(int ollydbgversion,ulong *features,
                                   wchar_t pluginname[SHORTNAME],wchar_t pluginversion[SHORTNAME]) {
                                       // Check whether OllyDbg has compatible version. This plugin uses only the
                                       // most basic functions, so this check is done pro forma, just to remind of
                                       // this option.
                                       if (ollydbgversion<201)
                                           return 0;
                                       // Report name and version to OllyDbg.
                                       wcscpy(pluginname,PLUGINNAME);       // Name of plugin
                                       wcscpy(pluginversion,VERSION);       // Version of plugin
                                       return PLUGIN_VERSION;               // Expected API version
};

// Optional entry, called immediately after ODBG2_Pluginquery(). Plugin should
// make one-time initializations and allocate resources. On error, it must
// clean up and return -1. On success, it must return 0.
extc int __cdecl ODBG2_Plugininit(void) {

    g_pMemoryData = malloc(defMAX_DATA_SIZE);
    g_pwDisplayBuffer = malloc(defMAX_DISPLAY_SIZE);

    // Report success.
    return 0;
};

// Function is called when user opens new or restarts current application.
// Plugin should reset internal variables and data structures to the initial
// state.
extc void __cdecl ODBG2_Pluginreset(void) {
    
};

// OllyDbg calls this optional function when user wants to terminate OllyDbg.
// All MDI windows created by plugins still exist. Function must return 0 if
// it is safe to terminate. Any non-zero return will stop closing sequence. Do
// not misuse this possibility! Always inform user about the reasons why
// termination is not good and ask for his decision! Attention, don't make any
// unrecoverable actions for the case that some other plugin will decide that
// OllyDbg should continue running.
extc int __cdecl ODBG2_Pluginclose(void) {    
    return 0;
};

// OllyDbg calls this optional function once on exit. At this moment, all MDI
// windows created by plugin are already destroyed (and received WM_DESTROY
// messages). Function must free all internally allocated resources, like
// window classes, files, memory etc.
extc void __cdecl ODBG2_Plugindestroy(void) {
    if ( g_pMemoryData )
    {        
        free(g_pMemoryData);
        g_pMemoryData = NULL;
    }
    if ( g_pwDisplayBuffer )
    {        
        free(g_pwDisplayBuffer);
        g_pwDisplayBuffer = NULL;
    }
};

// Menu function of main menu, displays About dialog.
static int Mnewdump(t_table *pt,wchar_t *name,ulong index,int mode) {
    if (mode==MENU_VERIFY)
        return MENU_NORMAL;                // Always available
    else if (mode==MENU_EXECUTE) {
        AkeNewDumpWindow(g_OllyCpuDump.sel0, g_OllyCpuDump.sel1);
        return MENU_NOREDRAW;
    };
    return MENU_ABSENT;
};

static int Mnewselect(t_table *pt,wchar_t *name,ulong index,int mode) {
    if (mode==MENU_VERIFY)
        return MENU_NORMAL;                // Always available
    else if (mode==MENU_EXECUTE) {
        AkeNewSelectDump();
        return MENU_NOREDRAW;
    };
    return MENU_ABSENT;
};

static int Mcopyseldata(t_table *pt,wchar_t *name,ulong index,int mode) {
    if (mode==MENU_VERIFY)
        return MENU_NORMAL;                // Always available
    else if (mode==MENU_EXECUTE) {
        AkeCopySelectData();
        return MENU_NOREDRAW;
    };
    return MENU_ABSENT;
};

// Menu function of main menu, displays About dialog.
static int Mabout(t_table *pt,wchar_t *name,ulong index,int mode) {
    int n;
    wchar_t s[TEXTLEN];
    if (mode==MENU_VERIFY)
        return MENU_NORMAL;                // Always available
    else if (mode==MENU_EXECUTE) {
        // Debuggee should continue execution while message box is displayed.
        Resumeallthreads();
        // In this case, Swprintf() would be as good as a sequence of StrcopyW(),
        // but secure copy makes buffer overflow impossible.
        n=StrcopyW(s,TEXTLEN,L"AKE plugin v");
        n+=StrcopyW(s+n,TEXTLEN-n,VERSION);
        // COPYRIGHT POLICY: This bookmark plugin is an open-source freeware. It's
        // just a sample. The copyright below is also just a sample and applies to
        // the unmodified sample code only. Replace or remove copyright message if
        // you make ANY changes to this code!
        n+=StrcopyW(s+n,TEXTLEN-n,L"\nCopyright (C) 2017 cnbragon");        

        MessageBox(hwollymain,s,
            L"AKE Plugin",MB_OK|MB_ICONINFORMATION);
        // Suspendallthreads() and Resumeallthreads() must be paired, even if they
        // are called in inverse order!
        Suspendallthreads();
        return MENU_NOREDRAW;
    };
    return MENU_ABSENT;
};


// Plugin menu that will appear in the main OllyDbg menu. Note that this menu
// must be static and must be kept for the whole duration of the debugging
// session.
static t_menu mainmenu[] = { 
    { L"New Dump Window", L"Create a new dump window", K_NONE, Mnewdump, NULL, 0 },
    { L"New selection ", L"Create a new dump window with selected size", K_NONE, Mnewselect, NULL, 0 },
    { L"Copy selected data...", L"Copy selected data to various format", K_NONE, Mcopyseldata, NULL, 0 },
    { L"|Help", L"AKE Plugin help", K_NONE, Mabout, NULL, 0 },
    { L"|About", L"About AKE Plugin", K_NONE, Mabout, NULL, 0 },   
    { NULL, NULL, K_NONE, NULL, NULL, 0 }
};

// Plugin menu that will appear in the Disassembler pane of CPU window.
static t_menu disasmmenu[] = {    
    { L"AKE", L"About ake plugin", K_NONE, Mabout, NULL, 0 },    
    // End of menu.
    { NULL, NULL, K_NONE, NULL, NULL, 0 }
};

static t_menu dumpmenu[] = {    
    { L"New dump window", L"Create a new dump window", K_NONE, Mnewdump, NULL, 0 }, 
    { L"New selection ", L"Create a new dump window with selected size", K_NONE, Mnewselect, NULL, 0 },
    { L"Copy selected data...", L"Copy selected data to various format", K_NONE, Mcopyseldata, NULL, 0 }, 
    // End of menu.
    { NULL, NULL, K_NONE, NULL, NULL, 0 }
};

// Adds items either to main OllyDbg menu (type=PWM_MAIN) or to popup menu in
// one of the standard OllyDbg windows, like PWM_DISASM or PWM_MEMORY. When
// type matches, plugin should return address of menu. When there is no menu of
// given type, it must return NULL. If menu includes single item, it will
// appear directly in menu, otherwise OllyDbg will create a submenu with the
// name of plugin. Therefore, if there is only one item, make its name as
// descriptive as possible.
extc t_menu * __cdecl ODBG2_Pluginmenu(wchar_t *type) {
    if (wcscmp(type,PWM_MAIN)==0)
        // Main menu.
        return mainmenu;
    //else if (wcscmp(type,PWM_DISASM)==0)
        // Disassembler pane of CPU window.
        //return disasmmenu;
    else if (wcscmp(type, PWM_DUMP) == 0)
    {
        return dumpmenu;
    }
    return NULL;                         // No menu
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// DUMP WINDOW HOOK ///////////////////////////////

// Dump windows display contents of memory or file as bytes, characters,
// integers, floats or disassembled commands. Plugins have the option to modify
// the contents of the dump windows. If ODBG2_Plugindump() is present and some
// dump window is being redrawn, this function is called first with column=
// DF_FILLCACHE, addr set to the address of the first visible element in the
// dump window and n to the estimated total size of the data displayed in the
// window (n may be significantly higher than real data size for disassembly).
// If plugin returns 0, there are no elements that will be modified by plugin
// and it will receive no other calls. If necessary, plugin may cache some data
// necessary later. OllyDbg guarantees that there are no calls to
// ODBG2_Plugindump() from other dump windows till the final call with
// DF_FREECACHE.
// When OllyDbg draws table, there is one call for each table cell (line/column
// pair). Parameters s (UNICODE), mask (DRAW_xxx) and select (extended DRAW_xxx
// set) contain description of the generated contents of length n. Plugin may
// modify it and return corrected length, or just return the original length.
// When table is completed, ODBG2_Plugindump() receives final call with
// column=DF_FREECACHE. This is the time to free resources allocated on
// DF_FILLCACHE. Returned value is ignored.
// Use this feature only if absolutely necessary, because it may strongly
// impair the responsiveness of the OllyDbg. Always make it switchable with
// default set to OFF!
extc int __cdecl ODBG2_Plugindump(t_dump *pd,
                                  wchar_t *s,
                                  uchar *mask,
                                  int n,
                                  int *select,
                                  ulong addr,
                                  int column) 
{
    int nRet = 0;
  
    if ( DF_FILLCACHE == column )
    {
        // Check whether it's Disassembler pane of the CPU window.
        if ( !pd || (pd->menutype & DMT_CPUMASK) != DMT_CPUDUMP )
        {
            nRet = 0;
        }
        else
        {
            //nRet = 1;
            memcpy(&g_OllyCpuDump, pd, sizeof(t_dump));
        }       
    }
    else if (DF_FREECACHE == column) 
    {
        // We have allocated no resources, so we have nothing to do here.
    }
    else
    {
        nRet = n;
    }

    return nRet;
};
