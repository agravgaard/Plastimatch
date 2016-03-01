/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __mondoshot_main_h__
#define __mondoshot_main_h__

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/snglinst.h>

enum
{
    ID_MENU_QUIT = 1,
    ID_MENU_SETTINGS,
    ID_MENU_ABOUT,
    ID_BUTTON_SEND_PORTAL,
    ID_BUTTON_SEND_SCREENSHOT,
    ID_BUTTON_CANCEL,
    ID_LISTCTRL_PATIENTS,
    ID_TEXTCTRL_PATIENT_NAME,
    ID_TEXTCTRL_PATIENT_ID
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit ();
    virtual void OnQueryEndSession (wxCloseEvent& event);
    virtual int OnExit ();
public:
    wxSingleInstanceChecker *m_checker;
private:
    DECLARE_EVENT_TABLE()
};

class MyListCtrl : public wxListCtrl
{
public:
    MyListCtrl(wxWindow *parent,
               const wxWindowID id,
               const wxPoint& pos,
               const wxSize& size,
               long style)
        : wxListCtrl(parent, id, pos, size, style),
          m_attr(*wxBLUE, *wxLIGHT_GREY, wxNullFont)
        {
#ifdef __POCKETPC__
            EnableContextMenu();
#endif
        }

    // add one item to the listctrl in report mode
    void InsertItemInReportView(int i);

    void OnColClick(wxListEvent& event);
    void OnColRightClick(wxListEvent& event);
    void OnColBeginDrag(wxListEvent& event);
    void OnColDragging(wxListEvent& event);
    void OnColEndDrag(wxListEvent& event);
    void OnBeginDrag(wxListEvent& event);
    void OnBeginRDrag(wxListEvent& event);
    void OnBeginLabelEdit(wxListEvent& event);
    void OnEndLabelEdit(wxListEvent& event);
    void OnDeleteItem(wxListEvent& event);
    void OnDeleteAllItems(wxListEvent& event);
#if WXWIN_COMPATIBILITY_2_4
    void OnGetInfo(wxListEvent& event);
    void OnSetInfo(wxListEvent& event);
#endif
    void OnSelected(wxListEvent& event);
    void OnDeselected(wxListEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnActivated(wxListEvent& event);
    void OnFocused(wxListEvent& event);
    void OnCacheHint(wxListEvent& event);

    void OnChar(wxKeyEvent& event);

    void OnRightClick(wxMouseEvent& event);

private:
    void ShowContextMenu(const wxPoint& pos);
    wxLog *m_logOld;
    void SetColumnImage(int col, int image);

    void LogEvent(const wxListEvent& event, const wxChar *eventName);
    void LogColEvent(const wxListEvent& event, const wxChar *eventName);

#if defined (commentout)
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemColumnImage(long item, long column) const;
    virtual wxListItemAttr *OnGetItemAttr(long item) const;
#endif

    wxListItemAttr m_attr;

    DECLARE_NO_COPY_CLASS(MyListCtrl)
    DECLARE_EVENT_TABLE()
};

class Config_settings
{
public:
    wxString local_aet;
    wxString remote_aet;
    wxString remote_ip;
    wxString remote_port;
    wxString data_directory;
    bool capture_checkbox;
    wxString capture_roi_cmin;
    wxString capture_roi_cmax;
    wxString capture_roi_rmin;
    wxString capture_roi_rmax;
    bool white_checkbox;
    wxString white_roi_cmin;
    wxString white_roi_cmax;
    wxString white_roi_rmin;
    wxString white_roi_rmax;
};

class MyFrame : public wxFrame
{
public:

    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    virtual bool OnInit ();
    void OnMenuQuit (wxCommandEvent& event);
    void OnMenuSettings (wxCommandEvent& event);
    void OnMenuAbout (wxCommandEvent& event);
    void OnButtonSend (wxCommandEvent& event);
    void OnButtonCancel (wxCommandEvent& event);
    void OnHotKey1 (wxKeyEvent& event);
    void OnHotKey2 (wxKeyEvent& event);
    void OnWindowClose (wxCloseEvent& event);
    void listctrl_patients_populate (void);

    wxBitmap m_bitmap;
    wxTextCtrl *m_textctrl_patient_name;
    wxTextCtrl *m_textctrl_patient_id;
    MyListCtrl *m_listctrl_patients;
    wxPanel *m_panel;

    DECLARE_EVENT_TABLE()
};

class Config_dialog : public wxDialog
{
public:
    Config_dialog (wxWindow *parent);

    void OnButton (wxCommandEvent& event);

    wxTextCtrl *m_textctrl_data_directory;
    wxTextCtrl *m_textctrl_remote_ip;
    wxTextCtrl *m_textctrl_remote_port;
    wxTextCtrl *m_textctrl_remote_aet;
    wxTextCtrl *m_textctrl_local_aet;
    wxCheckBox *m_capture_checkbox;
    wxTextCtrl *m_textctrl_capture_roi_cmin;
    wxTextCtrl *m_textctrl_capture_roi_cmax;
    wxTextCtrl *m_textctrl_capture_roi_rmin;
    wxTextCtrl *m_textctrl_capture_roi_rmax;
    wxCheckBox *m_white_checkbox;
    wxTextCtrl *m_textctrl_white_roi_cmin;
    wxTextCtrl *m_textctrl_white_roi_cmax;
    wxTextCtrl *m_textctrl_white_roi_rmin;
    wxTextCtrl *m_textctrl_white_roi_rmax;

private:
    wxButton *m_button_save;
    wxButton *m_button_cancel;

    DECLARE_EVENT_TABLE()
};


#endif /* __mondoshot_main_h__ */
