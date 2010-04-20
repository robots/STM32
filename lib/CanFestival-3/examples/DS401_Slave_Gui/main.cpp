#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/slider.h>
#include <iostream>
#include <wx/brush.h>
#include <sstream>
#include <string.h>
#include <wx/listbox.h>
#include <wx/dynarray.h>
#include <wx/dcclient.h>
#include <wx/gauge.h>
#include <wx/spinctrl.h>

//#include "monicone.xpm"

#if defined(WIN32) && !defined(__CYGWIN__)
#include <windows.h>
extern "C"
{
#include "getopt.h"
}
#endif

#include "main.h"
#include "TestSlaveGui.h"
extern "C"
{
#include "canfestival.h"
#include "ObjDict.h"
}
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


wxTextCtrl *textLog;
int node_id_ext;
int to_start = 0;
int openInput = 0;
int openOutput = 0;

MyFrame *frame;
#define MAXLENGTH_BUSNAME 32
#define MAXLENGTH_BAUDRATE 8
char _busname[MAXLENGTH_BUSNAME] = "vcan0";
char _baudrate[MAXLENGTH_BAUDRATE] = "500K";
s_BOARD SlaveBoard = { _busname, _baudrate };

#define MAXLENGTH_LIBPATH 1024
char LibraryPath[MAXLENGTH_LIBPATH] = "libcanfestival_can_virtual.so";

#if defined(WIN32)
#define LIB_EXTENT wxT("*.dll")
#else
#define LIB_EXTENT wxT("*.so")
#endif

// Declare some IDs. These are arbitrary.
const int BOOKCTRL = 100;
const int FILE_QUIT = wxID_EXIT;
const int HELP_ABOUT = wxID_ABOUT;
const int ADD = 105;
const int REMOVE = 108;
const int QUIT = 109;
const int FREQBOX = 110;
const int START = 112;
const int TEXTBOX1 = 113;
const int STOP = 114;
const int LOAD = 115;
const int INBT1 = 118;
const int INBT2 = 119;
const int INBT3 = 120;
const int INBT4 = 121;
const int INBT5 = 122;
const int INBT6 = 123;
const int INBT7 = 124;
const int INBT8 = 125;
const int INS1 = 126;
const int INS2 = 127;
const int INS3 = 128;
const int INS4 = 129;
const int INS5 = 135;
const int INS6 = 136;
const int INS7 = 137;
const int INS8 = 138;
const int TIMER_ID = 130;
const int INST1 = 131;
const int INST2 = 132;
const int INST3 = 133;
const int INST4 = 134;
const int INST5 = 135;
const int INST6 = 136;
const int INST7 = 137;
const int INST8 = 138;

// Attach the event handlers. Put this after MyFrame declaration.
BEGIN_EVENT_TABLE (MyFrame, wxFrame)
EVT_BUTTON (STOP, MyFrame::OnStop)
EVT_BUTTON (LOAD, MyFrame::OnLoad)
EVT_BUTTON (START, MyFrame::OnStart)
EVT_TOGGLEBUTTON (INBT1, MyFrame::OnInbt1)
EVT_TOGGLEBUTTON (INBT2, MyFrame::OnInbt2)
EVT_TOGGLEBUTTON (INBT3, MyFrame::OnInbt3)
EVT_TOGGLEBUTTON (INBT4, MyFrame::OnInbt4)
EVT_TOGGLEBUTTON (INBT5, MyFrame::OnInbt5)
EVT_TOGGLEBUTTON (INBT6, MyFrame::OnInbt6)
EVT_TOGGLEBUTTON (INBT7, MyFrame::OnInbt7)
EVT_TOGGLEBUTTON (INBT8, MyFrame::OnInbt8)
EVT_COMMAND_SCROLL (INS1, MyFrame::OnIns1)
EVT_COMMAND_SCROLL (INS2, MyFrame::OnIns2)
EVT_COMMAND_SCROLL (INS3, MyFrame::OnIns3)
EVT_COMMAND_SCROLL (INS4, MyFrame::OnIns4)
EVT_COMMAND_SCROLL (INS5, MyFrame::OnIns5)
EVT_COMMAND_SCROLL (INS6, MyFrame::OnIns6)
EVT_COMMAND_SCROLL (INS7, MyFrame::OnIns7)
EVT_COMMAND_SCROLL (INS8, MyFrame::OnIns8)
EVT_SPINCTRL (INST1, MyFrame::OnInst1)
EVT_SPINCTRL (INST2, MyFrame::OnInst2)
EVT_SPINCTRL (INST3, MyFrame::OnInst3)
EVT_SPINCTRL (INST4, MyFrame::OnInst4)
EVT_SPINCTRL (INST5, MyFrame::OnInst5)
EVT_SPINCTRL (INST6, MyFrame::OnInst6)
EVT_SPINCTRL (INST7, MyFrame::OnInst7)
EVT_SPINCTRL (INST8, MyFrame::OnInst8)
EVT_LISTBOX (FREQBOX, MyFrame::OnFreqBoxClick)
EVT_MENU (FILE_QUIT, MyFrame::OnQuit)
EVT_BUTTON (QUIT, MyFrame::OnQuit) EVT_MENU (HELP_ABOUT, MyFrame::OnAbout)
EVT_TIMER (TIMER_ID, MyFrame::OnTimer)
END_EVENT_TABLE ()IMPLEMENT_APP_NO_MAIN (MyApp);
IMPLEMENT_WX_THEME_SUPPORT;

     void
     help ()
{
  printf ("**************************************************************\n");
  printf ("*  DS-401 Slave GUI                                          *\n");
  printf ("*                                                            *\n");
  printf ("*   Usage:                                                   *\n");
  printf ("*   ./DS401_Slave_Gui [OPTIONS]                              *\n");
  printf ("*                                                            *\n");
  printf ("*   OPTIONS:                                                 *\n");
  printf
    ("*     -l : Can library [\"libcanfestival_can_virtual.so\"]     *\n");
  printf ("*                                                            *\n");
  printf ("*    Slave:                                                  *\n");
  printf ("*     -i : Node id format [0x01 , 0x7F]                      *\n");
  printf ("*                                                            *\n");
  printf ("*    CAN bus:                                                *\n");
  printf
    ("*     -b : bus name [\"1\"]                                    *\n");
  printf ("*     -B : 1M,500K,250K,125K,100K,50K,20K,10K                *\n");
  printf ("*                                                            *\n");
  printf ("**************************************************************\n");
}


int
main (int argc, char **argv)
{

  int c;
  extern char *optarg;
  char *snodeid;
  while ((c = getopt (argc, argv, "-b:B:l:i:sIO")) != EOF)
    {
      switch (c)
	{
	case 'b':
	  if (optarg[0] == 0)
	    {
	      help ();
	      exit (1);
	    }
	  SlaveBoard.busname = optarg;
	  break;
	case 'B':
	  if (optarg[0] == 0)
	    {
	      help ();
	      exit (1);
	    }
	  SlaveBoard.baudrate = optarg;
	  break;
	case 'l':
	  if (optarg[0] == 0)
	    {
	      help ();
	      exit (1);
	    }
	  strncpy (LibraryPath, optarg, MAXLENGTH_LIBPATH);
	  break;
	case 'i':
	  if (optarg[0] == 0)
	    {
	      help ();
	      exit (1);
	    }
	  snodeid = optarg;
	  sscanf (snodeid, "%x", &node_id_ext);
	  break;
	case 's':
	  to_start = 1;
	  break;
	case 'I':
	  openInput = 1;
	  break;
	case 'O':
	  openOutput = 1;
	  break;
	default:
	  help ();
	  exit (1);
	}
    }
  wxEntry (argc, argv);
}


bool
MyApp::OnInit ()
{
  frame = new MyFrame (wxString::Format(wxT("CanOpen virtual DS-401 - Id : %d"),node_id_ext));
  frame->Show (true);
  if(openInput) frame->book->SetSelection(2);
  else if(openOutput) frame->book->SetSelection(3);
  return true;
}

MyFrame::MyFrame (const wxString & title):wxFrame (NULL, wxID_ANY, title, wxDefaultPosition, wxSize (800, 740),
	 wxDEFAULT_FRAME_STYLE, wxT ("frame")), m_timer (this,
							 TIMER_ID)
{
  wxMenu *
    fileMenu = new wxMenu;
  wxMenu *
    helpMenu = new wxMenu;
  wxPanel *
    panel;
  wxButton *
    quit;
  wxButton *
    load;
  wxBoxSizer *
    mysizer;
  wxBoxSizer *
    mysssizer;
  wxBoxSizer *
    myhsizer;
  wxBoxSizer *
    mybsizer;
  wxBoxSizer *
    myentrysizer;
  wxListBox *
    freqlist;
  wxStaticBox *
    namebox;
  wxStaticBox *
    freqbox;

// SetIcon(wxICON(monicone));
  SetBackgroundColour (wxSystemSettings::GetColour (wxSYS_COLOUR_BTNFACE));
  helpMenu->Append (HELP_ABOUT, _T ("&About...\tF1"),
		    _T ("Show about dialog"));
  fileMenu->Append (FILE_QUIT, _T ("E&xit\tAlt-X"), _T ("Quit this program"));
  wxMenuBar *
    menuBar = new wxMenuBar ();
  menuBar->Append (fileMenu, _T ("&File"));
  menuBar->Append (helpMenu, _T ("&Help"));
  SetMenuBar (menuBar);
  CreateStatusBar (2);
  SetStatusText (_T ("DS-401 GUI."), 0);
  SetStatusText (_T ("Welcome."), 1);

  book = new wxNotebook (this, BOOKCTRL);

  panel = new wxPanel (book);
  book->AddPage (panel, _T ("Configuration"), true);
  mysizer = new wxBoxSizer (wxVERTICAL);
  panel->SetSizer (mysizer);
  wxString
    choices[] = {
    _T ("10K"), _T ("20K"), _T ("50K"), _T ("100K"),
    _T ("125K"), _T ("250K"), _T ("500K"), _T ("1M")
  };

  myhsizer =
    new wxStaticBoxSizer (new
			  wxStaticBox (panel, wxID_ANY, _T ("Bus driver")),
			  wxHORIZONTAL);
  mysizer->Add (myhsizer, 0, wxEXPAND | wxALL, 5);
  load = new wxButton (panel, LOAD, _T ("Load can driver"));
  myhsizer->Add (load, 0, wxLEFT, 5);
  drivername =
    new wxTextCtrl (panel, wxID_ANY,
		    wxString ((const char *) LibraryPath, wxConvLocal));
  myhsizer->Add (drivername, 1, wxEXPAND | wxALL, 5);
  myentrysizer = new wxBoxSizer (wxHORIZONTAL);
  mysizer->Add (myentrysizer, 0, wxEXPAND | wxALL, 5);
  myhsizer =
    new wxStaticBoxSizer (new wxStaticBox (panel, wxID_ANY, _T ("Bus name")),
			  wxHORIZONTAL);
  myentrysizer->Add (myhsizer, 0, wxEXPAND | wxALL, 5);
  busname =
    new wxTextCtrl (panel, wxID_ANY,
		    wxString ((const char *) SlaveBoard.busname,
			      wxConvLocal));
  myhsizer->Add (busname, 0, wxLEFT, 5);
  myhsizer =
    new wxStaticBoxSizer (new
			  wxStaticBox (panel, wxID_ANY,
				       _T ("Node ID (Decimal)")), wxHORIZONTAL);
  myentrysizer->Add (myhsizer, 0, wxEXPAND | wxALL, 5);
  node_id =
    new wxSpinCtrl (panel, wxID_ANY, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, 1, 127, node_id_ext,
		    _T ("wxSpinCtrl"));
  myhsizer->Add (node_id, 0, wxRIGHT, 5);
  myhsizer =
    new wxStaticBoxSizer (new wxStaticBox (panel, wxID_ANY, _T ("Baudrate")),
			  wxHORIZONTAL);
  mysizer->Add (myhsizer, wxEXPAND | wxALL, wxEXPAND | wxALL, 5);
  freqlist =
    new wxListBox (panel, FREQBOX, wxDefaultPosition, wxDefaultSize, 8,
		   choices, wxLB_SINGLE | wxLB_HSCROLL);
  myhsizer->Add (freqlist, wxEXPAND | wxALL, wxEXPAND | wxALL, 5);
  quit = new wxButton (panel, QUIT, _T ("QUIT"));
  mysizer->Add (quit, 0, wxALL, 5);


  panel = new wxPanel (book);
  book->AddPage (panel, wxT ("Inputs"), true);
  mysizer = new wxBoxSizer (wxVERTICAL);
  panel->SetSizer (mysizer);
  myhsizer =
    new wxStaticBoxSizer (new
			  wxStaticBox (panel, wxID_ANY,
				       _T ("Read Inputs UNSIGNED8")),
			  wxHORIZONTAL);
  mysizer->Add (myhsizer, 0, wxEXPAND | wxALL, 30);
  inbt1 = new wxToggleButton (panel, INBT1, wxT ("1"));
  inbt2 = new wxToggleButton (panel, INBT2, wxT ("2"));
  inbt3 = new wxToggleButton (panel, INBT3, wxT ("3"));
  inbt4 = new wxToggleButton (panel, INBT4, wxT ("4"));
  inbt5 = new wxToggleButton (panel, INBT5, wxT ("5"));
  inbt6 = new wxToggleButton (panel, INBT6, wxT ("6"));
  inbt7 = new wxToggleButton (panel, INBT7, wxT ("7"));
  inbt8 = new wxToggleButton (panel, INBT8, wxT ("8"));
  myhsizer->Add (inbt1, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (inbt2, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (inbt3, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (inbt4, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (inbt5, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (inbt6, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (inbt7, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (inbt8, wxEXPAND | wxALL, 0, 5);
  myhsizer =
    new wxStaticBoxSizer (new
			  wxStaticBox (panel, wxID_ANY,
				       _T ("Analogue Inputs INTEGER16")),
			  wxVERTICAL);
  mysizer->Add (myhsizer, wxEXPAND | wxALL, wxEXPAND | wxALL, 30);
  mysssizer = new wxBoxSizer (wxHORIZONTAL);
  myhsizer->Add (mysssizer, 0, wxEXPAND | wxALL, 0);
  inst1 =
    new wxSpinCtrl (panel, INST1, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, -32768, 32767, 0,
		    _T ("0"));
  inst2 =
    new wxSpinCtrl (panel, INST2, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, -32768, 32767, 0,
		    _T ("0"));
  inst3 =
    new wxSpinCtrl (panel, INST3, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, -32768, 32767, 0,
		    _T ("0"));
  inst4 =
    new wxSpinCtrl (panel, INST4, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, -32768, 32767, 0,
		    _T ("0"));
  inst5 =
    new wxSpinCtrl (panel, INST5, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, -32768, 32767, 0,
		    _T ("0"));
  inst6 =
    new wxSpinCtrl (panel, INST6, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, -32768, 32767, 0,
		    _T ("0"));
  inst7 =
    new wxSpinCtrl (panel, INST7, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, -32768, 32767, 0,
		    _T ("0"));
  inst8 =
    new wxSpinCtrl (panel, INST8, wxEmptyString, wxDefaultPosition,
		    wxDefaultSize, wxSP_ARROW_KEYS, -32768, 32767, 0,
		    _T ("0"));
  mysssizer->Add (inst1, wxALL, 0, 5);
  mysssizer->Add (inst2, wxALL, 0, 5);
  mysssizer->Add (inst3, wxALL, 0, 5);
  mysssizer->Add (inst4, wxALL, 0, 5);
  mysssizer->Add (inst5, wxALL, 0, 5);
  mysssizer->Add (inst6, wxALL, 0, 5);
  mysssizer->Add (inst7, wxALL, 0, 5);
  mysssizer->Add (inst8, wxALL, 0, 5);
  mysssizer = new wxBoxSizer (wxHORIZONTAL);
  myhsizer->Add (mysssizer, wxEXPAND | wxALL, wxEXPAND | wxALL, 0);
  ins1 =
    new wxSlider (panel, INS1, 0, -32768, 32767, wxDefaultPosition,
		  wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE,
		  wxDefaultValidator, wxT ("slider"));
  ins2 =
    new wxSlider (panel, INS2, 0, -32768, 32767, wxDefaultPosition,
		  wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE,
		  wxDefaultValidator, wxT ("slider"));
  ins3 =
    new wxSlider (panel, INS3, 0, -32768, 32767, wxDefaultPosition,
		  wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE,
		  wxDefaultValidator, wxT ("slider"));
  ins4 =
    new wxSlider (panel, INS4, 0, -32768, 32767, wxDefaultPosition,
		  wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE,
		  wxDefaultValidator, wxT ("slider"));
  ins5 =
    new wxSlider (panel, INS5, 0, -32768, 32767, wxDefaultPosition,
		  wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE,
		  wxDefaultValidator, wxT ("slider"));
  ins6 =
    new wxSlider (panel, INS6, 0, -32768, 32767, wxDefaultPosition,
		  wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE,
		  wxDefaultValidator, wxT ("slider"));
  ins7 =
    new wxSlider (panel, INS7, 0, -32768, 32767, wxDefaultPosition,
		  wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE,
		  wxDefaultValidator, wxT ("slider"));
  ins8 =
    new wxSlider (panel, INS8, 0, -32768, 32767, wxDefaultPosition,
		  wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE,
		  wxDefaultValidator, wxT ("slider"));
  mysssizer->Add (ins1, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (ins2, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (ins3, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (ins4, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (ins5, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (ins6, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (ins7, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (ins8, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  quit = new wxButton (panel, QUIT, _T ("QUIT"));
  mysizer->Add (quit, 0, wxALL, 5);

  panel = new wxPanel (book);
  book->AddPage (panel, wxT ("Outputs"), true);
  mysizer = new wxBoxSizer (wxVERTICAL);
  panel->SetSizer (mysizer);
  myhsizer =
    new wxStaticBoxSizer (new
			  wxStaticBox (panel, wxID_ANY,
				       _T ("Write Outputs UNSIGNED8")),
			  wxHORIZONTAL);
  mysizer->Add (myhsizer, 0, wxEXPAND | wxALL, 30);
  outbt1 = new wxToggleButton (panel, wxID_ANY, wxT ("1"));
  outbt2 = new wxToggleButton (panel, wxID_ANY, wxT ("2"));
  outbt3 = new wxToggleButton (panel, wxID_ANY, wxT ("3"));
  outbt4 = new wxToggleButton (panel, wxID_ANY, wxT ("4"));
  outbt5 = new wxToggleButton (panel, wxID_ANY, wxT ("5"));
  outbt6 = new wxToggleButton (panel, wxID_ANY, wxT ("6"));
  outbt7 = new wxToggleButton (panel, wxID_ANY, wxT ("7"));
  outbt8 = new wxToggleButton (panel, wxID_ANY, wxT ("8"));
  myhsizer->Add (outbt1, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (outbt2, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (outbt3, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (outbt4, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (outbt5, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (outbt6, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (outbt7, wxEXPAND | wxALL, 0, 5);
  myhsizer->Add (outbt8, wxEXPAND | wxALL, 0, 5);
  myhsizer =
    new wxStaticBoxSizer (new
			  wxStaticBox (panel, wxID_ANY,
				       _T ("Analogue Output INTEGER16")),
			  wxVERTICAL);
  mysizer->Add (myhsizer, wxEXPAND | wxALL, wxEXPAND | wxALL, 30);
  mysssizer = new wxBoxSizer (wxHORIZONTAL);
  myhsizer->Add (mysssizer, 0, wxEXPAND | wxALL, 0);
  outst1 =
    new wxStaticText (panel, wxID_ANY, wxT ("label"), wxDefaultPosition,
		      wxDefaultSize, 0, wxT ("staticText"));
  outst2 =
    new wxStaticText (panel, wxID_ANY, wxT ("label"), wxDefaultPosition,
		      wxDefaultSize, 0, wxT ("staticText"));
  outst3 =
    new wxStaticText (panel, wxID_ANY, wxT ("label"), wxDefaultPosition,
		      wxDefaultSize, 0, wxT ("staticText"));
  outst4 =
    new wxStaticText (panel, wxID_ANY, wxT ("label"), wxDefaultPosition,
		      wxDefaultSize, 0, wxT ("staticText"));
  mysssizer->Add (outst1, wxEXPAND | wxALL, wxEXPAND | wxALL, 5);
  mysssizer->Add (outst2, wxEXPAND | wxALL, wxEXPAND | wxALL, 5);
  mysssizer->Add (outst3, wxEXPAND | wxALL, wxEXPAND | wxALL, 5);
  mysssizer->Add (outst4, wxEXPAND | wxALL, wxEXPAND | wxALL, 5);
  mysssizer = new wxBoxSizer (wxHORIZONTAL);
  myhsizer->Add (mysssizer, wxEXPAND | wxALL, wxEXPAND | wxALL, 0);
  outs1 =
    new wxGauge (panel, wxID_ANY, 65535, wxDefaultPosition, wxDefaultSize,
		 wxSL_VERTICAL | wxSL_INVERSE, wxDefaultValidator,
		 wxT ("gauge"));
  outs2 =
    new wxGauge (panel, wxID_ANY, 65535, wxDefaultPosition, wxDefaultSize,
		 wxSL_VERTICAL | wxSL_INVERSE, wxDefaultValidator,
		 wxT ("gauge"));
  outs3 =
    new wxGauge (panel, wxID_ANY, 65535, wxDefaultPosition, wxDefaultSize,
		 wxSL_VERTICAL | wxSL_INVERSE, wxDefaultValidator,
		 wxT ("gauge"));
  outs4 =
    new wxGauge (panel, wxID_ANY, 65535, wxDefaultPosition, wxDefaultSize,
		 wxSL_VERTICAL | wxSL_INVERSE, wxDefaultValidator,
		 wxT ("gauge"));
  mysssizer->Add (outs1, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (outs2, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (outs3, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  mysssizer->Add (outs4, wxEXPAND | wxALL, wxEXPAND | wxALL, 15);
  quit = new wxButton (panel, QUIT, _T ("QUIT"));
  mysizer->Add (quit, 0, wxALL, 5);

  wxString
    choice[] = {
    _T ("Analogue Out 1"), _T ("Analogue Out 2"),
    _T ("Analogue Out 3"), _T ("Analogue Out 4"),
    _T ("Bool Out 1"), _T ("Bool Out 2"),
    _T ("Bool Out 3"), _T ("Bool Out 4"),
    _T ("Bool Out 5"), _T ("Bool Out 6"),
    _T ("Bool Out 7"), _T ("Bool Out 8"),
    _T ("Analogue Input 1"), _T ("Analogue Input 2"),
    _T ("Analogue Input 3"), _T ("Analogue Input 4"),
    _T ("Analogue Input 5"), _T ("Analogue Input 6"),
    _T ("Analogue Input 7"), _T ("Analogue Input 8"),
    _T ("Bool Input 1"), _T ("Bool Input 2"),
    _T ("Bool Input 3"), _T ("Bool Input 4"),
    _T ("Bool Input 5"), _T ("Bool Input 6"),
    _T ("Bool Input 7"), _T ("Bool Input 8"),
  };

  panel = new wxPanel (book);
  book->InsertPage (0, panel, wxT ("Control"), true);
  mysizer = new wxBoxSizer (wxVERTICAL);
  panel->SetSizer (mysizer);
  myhsizer = new wxBoxSizer (wxHORIZONTAL);
  myhsizer =
    new wxStaticBoxSizer (new
			  wxStaticBox (panel, wxID_ANY,
				       _T ("Control your node")),
			  wxHORIZONTAL);
  mysizer->Add (myhsizer, 0, wxALL, 5);
  start = new wxButton (panel, START, _T ("START"));
  stop = new wxButton (panel, STOP, _T ("STOP"));
  stop->Enable (false);
  myhsizer->Add (start, 0, wxRIGHT, 5);
  myhsizer->Add (stop, 0, wxLEFT, 5);
  myhsizer =
    new wxStaticBoxSizer (new
			  wxStaticBox (panel, wxID_ANY, _T ("Operation log")),
			  wxHORIZONTAL);
  mysizer->Add (myhsizer, wxALL, wxEXPAND | wxALL, 5);
  textLog =
    new wxTextCtrl (panel, TEXTBOX1, _T (""), wxDefaultPosition,
		    wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator,
		    wxTextCtrlNameStr);
  myhsizer->Add (textLog, wxALL, wxEXPAND | wxALL, 5);
  quit = new wxButton (panel, QUIT, _T ("QUIT"));
  mysizer->Add (quit, 0, wxALL, 5);

  m_timer.Start (100);
  if (to_start)
    {
      Start ();
    }
}

void
MyFrame::OnQuit (wxCommandEvent & WXUNUSED (event))
{
  Close (true);
}

void
MyFrame::Start ()
{
  if (main_can (SlaveBoard, LibraryPath))
    {
      printf ("[KO]\n");
      fflush(stdout);
      *textLog <<
	wxT
	("----------------------------------------------------Initialization [KO]\n");
    }
  else
    {
      printf ("[OK]\n");
      fflush(stdout);
      *textLog <<
	wxT
	("----------------------------------------------------Initialization [OK]\n");
      stop->Enable (true);
      start->Enable (false);
    }
}

void
MyFrame::OnStart (wxCommandEvent & WXUNUSED (event))
{
  wxString s;
  
  node_id_ext = node_id->GetValue ();
  s = busname->GetValue ();
  strncpy (SlaveBoard.busname, s.mb_str (), MAXLENGTH_BUSNAME);
  *textLog << wxT ("-- Bus name: ------> ") << s << wxT ("\n");
  *textLog << wxT ("-- Node ID: -------> ") << node_id->
    GetValue () << wxT ("\n");
  SetTitle(wxString::Format(wxT("CanOpen virtual DS-401 - Id : %d"),node_id->
		    GetValue ()));
  Start ();
}


void
MyFrame::OnStop (wxCommandEvent & WXUNUSED (event))
{

  *textLog <<
    wxT
    ("----------------------------------------------------Slave [STOPPED]") <<
    wxT ("\n");
  stop_slave ();
  stop->Enable (false);
  start->Enable (true);
}

void
MyFrame::OnLoad (wxCommandEvent & WXUNUSED (event))
{
  wxFileDialog fd (this, wxT ("Choose a can driver library"), wxT (""),
		   wxT (""), LIB_EXTENT);

  if (fd.ShowModal () == wxID_OK)
    {
      drivername->Clear ();
      drivername->AppendText (fd.GetPath ());
      *textLog << wxT ("LibraryPath: ") << fd.GetPath () << wxT ("\n");
      strncpy (LibraryPath, fd.GetPath ().mb_str (), MAXLENGTH_LIBPATH);
      printf ("path=%s", LibraryPath);
    }
}

void
MyFrame::OnInbt1 (wxCommandEvent & WXUNUSED (event))
{
  Read_Inputs_8_Bit[0] = Read_Inputs_8_Bit[0] ^ 0x1;
}

void
MyFrame::OnInbt2 (wxCommandEvent & WXUNUSED (event))
{
  Read_Inputs_8_Bit[0] = Read_Inputs_8_Bit[0] ^ 0x2;
}

void
MyFrame::OnInbt3 (wxCommandEvent & WXUNUSED (event))
{
  Read_Inputs_8_Bit[0] = Read_Inputs_8_Bit[0] ^ 0x4;
}

void
MyFrame::OnInbt4 (wxCommandEvent & WXUNUSED (event))
{
  Read_Inputs_8_Bit[0] = Read_Inputs_8_Bit[0] ^ 0x8;
}

void
MyFrame::OnInbt5 (wxCommandEvent & WXUNUSED (event))
{
  Read_Inputs_8_Bit[0] = Read_Inputs_8_Bit[0] ^ 0x10;
}

void
MyFrame::OnInbt6 (wxCommandEvent & WXUNUSED (event))
{
  Read_Inputs_8_Bit[0] = Read_Inputs_8_Bit[0] ^ 0x20;
}

void
MyFrame::OnInbt7 (wxCommandEvent & WXUNUSED (event))
{
  Read_Inputs_8_Bit[0] = Read_Inputs_8_Bit[0] ^ 0x40;
}

void
MyFrame::OnInbt8 (wxCommandEvent & WXUNUSED (event))
{
  Read_Inputs_8_Bit[0] = Read_Inputs_8_Bit[0] ^ 0x80;
}



void
MyFrame::OnIns1 (wxScrollEvent & event)
{
  Read_Analogue_Input_16_Bit[0] = ins1->GetValue ();
  inst1->SetValue (ins1->GetValue ());
}

void
MyFrame::OnIns2 (wxScrollEvent & event)
{
  Read_Analogue_Input_16_Bit[1] = ins2->GetValue ();
  inst2->SetValue (ins2->GetValue ());
}

void
MyFrame::OnIns3 (wxScrollEvent & event)
{
  Read_Analogue_Input_16_Bit[2] = ins3->GetValue ();
  inst3->SetValue (ins3->GetValue ());
}

void
MyFrame::OnIns4 (wxScrollEvent & event)
{
  Read_Analogue_Input_16_Bit[3] = ins4->GetValue ();
  inst4->SetValue (ins4->GetValue ());
}

void
MyFrame::OnIns5 (wxScrollEvent & event)
{
  Read_Analogue_Input_16_Bit[4] = ins5->GetValue ();
  inst5->SetValue (ins5->GetValue ());
}

void
MyFrame::OnIns6 (wxScrollEvent & event)
{
  Read_Analogue_Input_16_Bit[5] = ins6->GetValue ();
  inst6->SetValue (ins6->GetValue ());
}

void
MyFrame::OnIns7 (wxScrollEvent & event)
{
  Read_Analogue_Input_16_Bit[6] = ins7->GetValue ();
  inst7->SetValue (ins7->GetValue ());
}

void
MyFrame::OnIns8 (wxScrollEvent & event)
{
  Read_Analogue_Input_16_Bit[7] = ins8->GetValue ();
  inst8->SetValue (ins8->GetValue ());
}


void
MyFrame::OnInst1 (wxSpinEvent & ev)
{
  Read_Analogue_Input_16_Bit[0] = inst1->GetValue ();
  ins1->SetValue (inst1->GetValue ());
}

void
MyFrame::OnInst2 (wxSpinEvent & ev)
{
  Read_Analogue_Input_16_Bit[1] = inst2->GetValue ();
  ins2->SetValue (inst2->GetValue ());
}

void
MyFrame::OnInst3 (wxSpinEvent & ev)
{
  Read_Analogue_Input_16_Bit[2] = inst3->GetValue ();
  ins3->SetValue (inst3->GetValue ());
}

void
MyFrame::OnInst4 (wxSpinEvent & ev)
{
  Read_Analogue_Input_16_Bit[3] = inst4->GetValue ();
  ins4->SetValue (inst4->GetValue ());
}

void
MyFrame::OnInst5 (wxSpinEvent & ev)
{
  Read_Analogue_Input_16_Bit[4] = inst5->GetValue ();
  ins5->SetValue (inst5->GetValue ());
}

void
MyFrame::OnInst6 (wxSpinEvent & ev)
{
  Read_Analogue_Input_16_Bit[5] = inst6->GetValue ();
  ins6->SetValue (inst6->GetValue ());
}

void
MyFrame::OnInst7 (wxSpinEvent & ev)
{
  Read_Analogue_Input_16_Bit[6] = inst7->GetValue ();
  ins7->SetValue (inst7->GetValue ());
}

void
MyFrame::OnInst8 (wxSpinEvent & ev)
{
  Read_Analogue_Input_16_Bit[7] = inst8->GetValue ();
  ins8->SetValue (inst8->GetValue ());
}


void
MyFrame::OnAbout (wxCommandEvent & WXUNUSED (event))
{
  wxString msg;
  msg.Printf (_T ("About.\n\n")
	      _T
	      ("Input-Output simulation by GRANDEMANGE Nicolas for CanFestival!!"));

  wxMessageBox (msg, _T ("About !!!"), wxOK | wxICON_INFORMATION, this);
}


void
MyFrame::OnFreqBoxClick (wxCommandEvent & event)
{
  wxString wxstr;

  wxstr = event.GetString ();
  strncpy (SlaveBoard.baudrate, wxstr.mb_str (), MAXLENGTH_BAUDRATE);
  *textLog << wxT ("Baudrate:") << event.GetString () << wxT ("\n");
}

static int
is_set (int i, wxListBox * inlist, wxArrayInt in)
{
  inlist->GetSelections (in);
  for (int j = 0; j < in.GetCount (); j++)
    {
      if (i == (in[j] + 1))
	{
	  return (i);
	}
    }
  return (0);
}

static int
get_bit (UNS8 input, int bit)
{
  return input & (((UNS8) 1) << bit - 1) ? 1 : 0;
}

void
actu_output (void)
{
  wxString tmp;

  frame->outbt1->SetValue (get_bit (Write_Outputs_8_Bit[0], 1));
  frame->outbt2->SetValue (get_bit (Write_Outputs_8_Bit[0], 2));
  frame->outbt3->SetValue (get_bit (Write_Outputs_8_Bit[0], 3));
  frame->outbt4->SetValue (get_bit (Write_Outputs_8_Bit[0], 4));
  frame->outbt5->SetValue (get_bit (Write_Outputs_8_Bit[0], 5));
  frame->outbt6->SetValue (get_bit (Write_Outputs_8_Bit[0], 6));
  frame->outbt7->SetValue (get_bit (Write_Outputs_8_Bit[0], 7));
  frame->outbt8->SetValue (get_bit (Write_Outputs_8_Bit[0], 8));

  frame->outs1->SetValue (Write_Analogue_Output_16_Bit[0] + 32768);
  frame->outs2->SetValue (Write_Analogue_Output_16_Bit[1] + 32768);
  frame->outs3->SetValue (Write_Analogue_Output_16_Bit[2] + 32768);
  frame->outs4->SetValue (Write_Analogue_Output_16_Bit[3] + 32768);
  tmp.Printf (wxT ("       %i"), Write_Analogue_Output_16_Bit[0]);
  frame->outst1->SetLabel (tmp);
  tmp.Printf (wxT ("       %i"), Write_Analogue_Output_16_Bit[1]);
  frame->outst2->SetLabel (tmp);
  tmp.Printf (wxT ("       %i"), Write_Analogue_Output_16_Bit[2]);
  frame->outst3->SetLabel (tmp);
  tmp.Printf (wxT ("       %i"), Write_Analogue_Output_16_Bit[3]);
  frame->outst4->SetLabel (tmp);
}

void
actu (void)
{
  actu_output ();
}

void
MyFrame::OnTimer (wxTimerEvent & event)
{
  actu ();			// do whatever you want to do every second here
}

void
MyFrame::OnPaint (wxPaintEvent & event)
{
  //Paint();
}
