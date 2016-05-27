#include <windows.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <commdlg.h>

#include "resource.h"
#include "main.h"
#include "in2.h"

/********* A LITTLE BIT OF WINAMP PLUGIN USAGE **********/

// define procedures, that'll be found in a .DLL
typedef In_Module* (*INHDRPROC)(void);
typedef Out_Module* (*OUTHDRPROC)(void);

// dsp-functions
int dsp_donothing(short int *, int cnt, int, int, int){return cnt;}
int dsp_isactive(){return 0;}

// other functions, needed to get it to work
void SAVSAInit(int maxlatency_in_ms, int srate){}
void SAVSADeInit(){}
void SAAddPCMData(void *PCMData, int nch, int bps, int timestamp){}
int  SAGetMode(){return 0;}
void SAAdd(void *data, int timestamp, int csa){}
void VSAAddPCMData(void *PCMData, int nch, int bps, int timestamp){}
int  VSAGetMode(int *specNch, int *waveNch){*specNch = *waveNch = 0;return 0;}
void VSAAdd(void *data, int timestamp){}
void VSASetInfo(int nch, int srate){}
void SetInfo(int bitrate, int srate, int stereo, int synched){}


/********** END OF BIT OF WINAMP PLUGIN USAGE ***********/


// these three makes dialog-handling much easier
// but they are Win32 specific, but so is my program
// so it really doesn't matter
#define CTLID   LOWORD(wParam)       // Control ID for WM_COMMAND
#define CTLMSG  HIWORD(wParam)       // Notification Message of Control
#define HCTL    (HWND)lParam         // window handle of control


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

	hInstance = hInst;

    //init the CMP3Info class
    mp3Info = new CMP3Info();

    int success = DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAIN_WINDOW),NULL,(DLGPROC)MainDlgProc);

    delete mp3Info;

    return success;

}

LRESULT CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
            { 
                SendDlgItemMessage(hDlg, IDC_TITLE, EM_SETLIMITTEXT,(WPARAM)30, (LPARAM)0);
                SendDlgItemMessage(hDlg, IDC_ARTIST, EM_SETLIMITTEXT,(WPARAM)30, (LPARAM)0);
                SendDlgItemMessage(hDlg, IDC_ALBUM, EM_SETLIMITTEXT,(WPARAM)30, (LPARAM)0);
                SendDlgItemMessage(hDlg, IDC_GENRE, EM_SETLIMITTEXT,(WPARAM)20, (LPARAM)0);
                SendDlgItemMessage(hDlg, IDC_COMMENT, EM_SETLIMITTEXT,(WPARAM)30, (LPARAM)0);
                SendDlgItemMessage(hDlg, IDC_YEAR, EM_SETLIMITTEXT,(WPARAM)4, (LPARAM)0);
                SendDlgItemMessage(hDlg, IDC_TRACK, EM_SETLIMITTEXT,(WPARAM)3, (LPARAM)0);
            }
			break;
        case WM_COMMAND:

            switch (CTLID) {

                case IDEXIT:

                    { 
                        // check if user really wants to quit, if so, terminate
                        int answer=MessageBox(hDlg,"Do you really want to quit?",szAppName,MB_ICONQUESTION|MB_YESNO);
                        if (answer==IDYES) EndDialog(hDlg,FALSE);

                    }
                    break;

                case IDOPEN:

                    {
                        
                        // there will probably be no code here, but a execution of a large function
                        openFile(hDlg);

                    }
                    break;

                case IDABOUT:

                    {

                        // alert a message box telling a little about the program
                        char mybuf[1024];
                        sprintf(mybuf,"welcome to the about dialog for MP3Info Example v %d.%d\n\nthis program is entirely written by Gustav Munkby,\nand unforuntaly I can't guarrantue that it works :(\nI think that's all from me for now...\n\nBILLDAL %d", 1,1,990818);
                        MessageBox(hDlg,mybuf,"MP3Info Example - About", MB_OK|MB_ICONINFORMATION|MB_YESNOCANCEL);
                    }
                    break;

                case IDSAVETAG:

                    {

                        saveTag(hDlg);

                    }
                    break;

                case IDPLAYFILE:

                    {

                        playFile();

                    }
                    break;

            }
            break;

        case WM_PAINT:
			return FALSE;
        case WM_KEYDOWN:
            break;
        case WM_LBUTTONDOWN:
            {
//                mp3info *info = new mp3info("C:\\_mp3\\nirvan~1.mp3"); //Nirvana - nevermind - 01. Smells Like Teen Spirit.mp3");
//                info->alertInfo(hwnd);
            }
            break;
        default:
            return FALSE; // let windows handle message
    }
    return TRUE;
}

int openFile(HWND hDlg) {

    // string which will be used for the fileName
    // the = {""} part just makes sure that the filename
    // textbox in the dialog is empty, otherwise there might
    // be something in it, I kept getting "ÁÁÁ" which I didn't want
    char szFileName[256] = {""};

    // OPENFILENAME structure used for getting a file name from
    // OpenFile Common Dialog...
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));

    // fill OPENFILENAME structure 
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = "MPEG Audio Files (*.mp1, *.mp2, *.mp3)\0*.mp1;*.mp2;*.mp3\0All Files (*.*)\0*.*\0\0";
    ofn.lpstrFile = szFileName;
    ofn.lpstrTitle = "select MPEG Audio File to get information from...";
    ofn.nMaxFile = 256;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "mp3";


    if(GetOpenFileName(&ofn)){

        int loadstate = mp3Info->loadInfo(szFileName); 
        
        if (loadstate!=0) return loadstate;

        // the lengths of the following char arrays are
        // all exponents of 2 and the way I figured them
        // out, was by trial and error, so no guarrantue
        // here... :)

        // this is the same values as used inside other
        // functions in the class...
        char tracknumber[4] = {"n/a"};

        char year[8] = {"n/a"};

        char bitrate[16] = {"n/a"};
        char filesize[16] = {"n/a"};
        char frequency[16] = {"n/a"};
        char length[16] = {"n/a"};
        char numberofframes[16] = {"n/a"};

        char album[32] = {"n/a"};
        char artist[32] = {"n/a"};
        char comment[32] = {"n/a"};
        char genre[32] = {"n/a"};
        char mode[32] = {"n/a"};
        char mpegversion[32] = {"n/a"};
        char title[32] = {"n/a"};

        // get version like "MPEG v1.0 Layer III"
        mp3Info->getVersion(mpegversion);

        // get the bitrate like "128 kbps"
        sprintf(bitrate, "%d kbps", mp3Info->getBitrate() );

        // get frequncy like "44100 Hz"
        sprintf(frequency, "%d Hz", mp3Info->getFrequency() );

        // get length like "mm:ss"
        mp3Info->getFormattedLength(length);

        // get file size like "1253 kB"
        sprintf(filesize, "%d kB", (mp3Info->getFileSize()/1024) );

        // get playing mode like "Stereo"
        mp3Info->getMode(mode);

        // get song title like "Californication"
        if ( mp3Info->isTagged() ) mp3Info->getTitle(title);

        // get artist name like "Red Hot Chili Peppers"
        if ( mp3Info->isTagged() ) mp3Info->getArtist(artist);

        // get album name like "One Hot Minute"
        if ( mp3Info->isTagged() ) mp3Info->getAlbum(album);
        
        // get name of genre like "Grunge"
        if ( mp3Info->isTagged() ) mp3Info->getGenre(genre);

        // get comment like "encoded by me..."
        if ( mp3Info->isTagged() ) mp3Info->getComment(comment);

        // get Number of frames like "123432"
        sprintf( numberofframes, "%d", mp3Info->getNumberOfFrames() );

        // get Year like "1999"
        if ( mp3Info->isTagged() ) sprintf( year, "%d", mp3Info->getYear() );

        // get track number (obly for id3v1.1) like "1"
        if ( !(mp3Info->getTrackNumber()==-1) ) sprintf( tracknumber, "%d", mp3Info->getTrackNumber() );


        // now we should print all these values into the dialog-boxes
        // none of this information will be explained as I think it's
        // a: quite self-explanatory
        // b: to darn much to write

        // hope you can manage, if someone should have great trouble
        // with understanding, please contact me and I'll comment the
        // code in this section...

        SendDlgItemMessage(hDlg,IDC_OPENED_FILE , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)szFileName);
        SendDlgItemMessage(hDlg,IDC_MPEG_VERSION, WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)mpegversion);
        SendDlgItemMessage(hDlg,IDC_BITRATE     , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)bitrate);
        SendDlgItemMessage(hDlg,IDC_FREQUENCY   , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)frequency);
        SendDlgItemMessage(hDlg,IDC_LENGTH      , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)length);
        SendDlgItemMessage(hDlg,IDC_FILESIZE    , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)filesize);
        SendDlgItemMessage(hDlg,IDC_MODE        , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)mode);
        SendDlgItemMessage(hDlg,IDC_TITLE       , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)title);
        SendDlgItemMessage(hDlg,IDC_ARTIST      , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)artist);
        SendDlgItemMessage(hDlg,IDC_ALBUM       , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)album);
        SendDlgItemMessage(hDlg,IDC_GENRE       , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)genre);
        SendDlgItemMessage(hDlg,IDC_COMMENT     , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)comment);
        SendDlgItemMessage(hDlg,IDC_FRAMES      , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)numberofframes);
        SendDlgItemMessage(hDlg,IDC_YEAR        , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)year);
        SendDlgItemMessage(hDlg,IDC_TRACK       , WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)tracknumber);

        CheckDlgButton(hDlg, IDC_VBR, (mp3Info->isVBitRate()?BST_CHECKED:BST_UNCHECKED) );

        EnableWindow(GetDlgItem(hDlg,IDSAVETAG), TRUE);
        EnableWindow(GetDlgItem(hDlg,IDPLAYFILE), TRUE);


    }

    return 0;

}

void saveTag(HWND hDlg) {

    if (mp3Info->getBitrate()!=0) {

        // initate variable for later use...
        char tracknumber[4] = {""};
        char year[8] = {""};
        char album[32] = {""};
        char artist[32] = {""};
        char comment[32] = {""};
        char genre[32] = {""};
        char title[32] = {""};

        // get updated? strings from the dialog box
        SendDlgItemMessage(hDlg,IDC_TRACK   , WM_GETTEXT, (WPARAM)4,  (LPARAM)(LPCTSTR)tracknumber);
        SendDlgItemMessage(hDlg,IDC_YEAR    , WM_GETTEXT, (WPARAM)8,  (LPARAM)(LPCTSTR)year);
        SendDlgItemMessage(hDlg,IDC_ALBUM   , WM_GETTEXT, (WPARAM)32, (LPARAM)(LPCTSTR)album);
        SendDlgItemMessage(hDlg,IDC_ARTIST  , WM_GETTEXT, (WPARAM)32, (LPARAM)(LPCTSTR)artist);
        SendDlgItemMessage(hDlg,IDC_COMMENT , WM_GETTEXT, (WPARAM)32, (LPARAM)(LPCTSTR)comment);
        SendDlgItemMessage(hDlg,IDC_GENRE   , WM_GETTEXT, (WPARAM)32, (LPARAM)(LPCTSTR)genre);
        SendDlgItemMessage(hDlg,IDC_TITLE   , WM_GETTEXT, (WPARAM)32, (LPARAM)(LPCTSTR)title);

        // save the new strings to tag structure & .mp3-file
        mp3Info->saveTag(title,artist,album,year,comment,tracknumber,genre);
            
     }

}




void playFile() {

    // load input-library & outputlibrary
    HINSTANCE hout = LoadLibrary("out_wave.dll");
    HINSTANCE hin = LoadLibrary("in_mp3.dll");

    // search for function to get a pointer to the interesting information
    INHDRPROC ihp = (INHDRPROC)GetProcAddress(hin,"winampGetInModule2");
    OUTHDRPROC ohp = (OUTHDRPROC)GetProcAddress(hout,"winampGetOutModule");

    // use the functions to get the pointers
    In_Module* in = ihp();
    Out_Module* out = ohp();
    
    // if it's an incorrect version
    if (in->version != IN_VER || out->version != OUT_VER) 
    {
        FreeLibrary(hout);
        FreeLibrary(hin);
        return;
    }

    // fill the out header
    out->hMainWindow = NULL;
    out->hDllInstance = hout;

    // fill the in header
    in->hMainWindow = NULL;
    in->hDllInstance = hin;


    if (in->UsesOutputPlug)
    {
        in->outMod = out;
    }

    // set functions to specified ones
    in->SAVSAInit = SAVSAInit;
    in->SAVSADeInit = SAVSADeInit;
    in->SAAdd = SAAdd;
    in->SAGetMode = SAGetMode;
    in->SAAddPCMData = SAAddPCMData;
    in->VSAAddPCMData = VSAAddPCMData;
    in->VSAGetMode = VSAGetMode;
    in->VSAAdd = VSAAdd;
    in->VSASetInfo = VSASetInfo;
    
    // dsp functions
    in->dsp_dosamples = dsp_donothing;
    in->dsp_isactive = dsp_isactive;

    in->SetInfo = SetInfo;

    // init the plugins
    out->Init();
    in->Init();

    char playFile[256] = {""};
    mp3Info->getFileName(playFile);

    // if playing starts correctly
    if (!in->Play(playFile))
    {
        // set max volume and center panning
        in->SetVolume(255);
        in->SetPan(0);


        //int x = 1,len = in->GetLength();
        //for (;x=1 && in->GetOutputTime()<len;) Sleep(100);
        
        // the music will play until a button in the messageBox is pressed
        MessageBox(0,"press the button to terminate play", "playing music...", MB_OK|MB_SYSTEMMODAL);

        // when playing stops, terminate
        in->Stop();
    }

    // un-init plugins
    in->Quit();
    out->Quit();

    // deload the .DLLs
    FreeLibrary(hin);
    FreeLibrary(hout);

};  

