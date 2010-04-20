#ifndef MAIN_H_
#define MAIN_H_

#include <wx/tglbtn.h>
#include <wx/slider.h>
#include <wx/brush.h>
#include <wx/dynarray.h>
#include <wx/listbox.h>
#include <wx/gauge.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>

class MyApp:public wxApp
{
public:
  virtual bool OnInit ();
};

class MyFrame:public wxFrame
{
public:

  wxNotebook * book;
  wxButton *stop;
  wxButton *start;
  wxTextCtrl *busname;
  wxSpinCtrl *inst1;
  wxSpinCtrl *inst2;
  wxSpinCtrl *inst3;
  wxSpinCtrl *inst4;
  wxSpinCtrl *inst5;
  wxSpinCtrl *inst6;
  wxSpinCtrl *inst7;
  wxSpinCtrl *inst8;
  wxTextCtrl *drivername;
  wxSpinCtrl *node_id;
  wxToggleButton *inbt1;
  wxToggleButton *inbt2;
  wxToggleButton *inbt3;
  wxToggleButton *inbt4;
  wxToggleButton *inbt5;
  wxToggleButton *inbt6;
  wxToggleButton *inbt7;
  wxToggleButton *inbt8;
  wxSlider *ins1;
  wxSlider *ins2;
  wxSlider *ins3;
  wxSlider *ins4;
  wxSlider *ins5;
  wxSlider *ins6;
  wxSlider *ins7;
  wxSlider *ins8;
  wxToggleButton *outbt1;
  wxToggleButton *outbt2;
  wxToggleButton *outbt3;
  wxToggleButton *outbt4;
  wxToggleButton *outbt5;
  wxToggleButton *outbt6;
  wxToggleButton *outbt7;
  wxToggleButton *outbt8;
  wxGauge *outs1;
  wxGauge *outs2;
  wxGauge *outs3;
  wxGauge *outs4;
  wxStaticText *outst1;
  wxStaticText *outst2;
  wxStaticText *outst3;
  wxStaticText *outst4;
  wxSlider *echelle;
  wxPanel *mygraphpan;
  wxPanel *mylegpan;
  wxListBox *inlist;
  wxArrayInt in;
  wxTimer m_timer;

    MyFrame (const wxString & title);

  void OnListBoxDoubleClick (wxCommandEvent & event);
  void OnInbt1 (wxCommandEvent & event);
  void OnInbt2 (wxCommandEvent & event);
  void OnInbt3 (wxCommandEvent & event);
  void OnInbt4 (wxCommandEvent & event);
  void OnInbt5 (wxCommandEvent & event);
  void OnInbt6 (wxCommandEvent & event);
  void OnInbt7 (wxCommandEvent & event);
  void OnInbt8 (wxCommandEvent & event);
  void OnQuit (wxCommandEvent & event);
  void OnAdd (wxCommandEvent & event);
  void OnRemove (wxCommandEvent & event);
  void OnAbout (wxCommandEvent & event);
  void OnStart (wxCommandEvent & WXUNUSED (event));
  void Start ();
  void OnStop (wxCommandEvent & WXUNUSED (event));
  void OnFreqBoxClick (wxCommandEvent & WXUNUSED (event));
  void OnLoad (wxCommandEvent & WXUNUSED (event));
  void OnPaint (wxPaintEvent & WXUNUSED (event));
  void OnIns1 (wxScrollEvent & event);
  void OnIns2 (wxScrollEvent & event);
  void OnIns3 (wxScrollEvent & event);
  void OnIns4 (wxScrollEvent & event);
  void OnIns5 (wxScrollEvent & event);
  void OnIns6 (wxScrollEvent & event);
  void OnIns7 (wxScrollEvent & event);
  void OnIns8 (wxScrollEvent & event);
  void OnInst1 (wxSpinEvent & ev);
  void OnInst2 (wxSpinEvent & ev);
  void OnInst3 (wxSpinEvent & ev);
  void OnInst4 (wxSpinEvent & ev);
  void OnInst5 (wxSpinEvent & ev);
  void OnInst6 (wxSpinEvent & ev);
  void OnInst7 (wxSpinEvent & ev);
  void OnInst8 (wxSpinEvent & ev);
  void Paint ();
  void OnTimer (wxTimerEvent & event);
private:
  DECLARE_EVENT_TABLE ()};

void actu (void);
void actu_output (void);


#endif /*MAIN_H_ */
