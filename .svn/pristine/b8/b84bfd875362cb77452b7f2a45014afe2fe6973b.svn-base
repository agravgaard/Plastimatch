/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* Note: wxWidgets bug for RegisterHotKey ()
    http://lists.wxwidgets.org/pipermail/wx-users/2009-January/111237.html
*/

#include <wx/wx.h>
#include <wx/window.h>
#include <wx/filename.h>
#include <wx/config.h>
#include "mondoshot_main.h"
#include "sqlite3.h"
#include "plm_version.h"
#include "mondoshot_dicom.h"

struct sqlite_populate_cbstruct {
    MyFrame *m_frame;
    int m_list_index;
};

void sqlite_patients_query (MyFrame* frame);
void sqlite_patients_insert_record (wxString patient_id, wxString patient_name);
void config_initialize ();
void config_save (void);

/* -----------------------------------------------------------------------
   Global variables
   ----------------------------------------------------------------------- */
Config_settings config;

/* -----------------------------------------------------------------------
   Event tables
   ----------------------------------------------------------------------- */
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_MENU_QUIT, MyFrame::OnMenuQuit)
    EVT_MENU(ID_MENU_SETTINGS, MyFrame::OnMenuSettings)
    EVT_MENU(ID_MENU_ABOUT, MyFrame::OnMenuAbout)
    EVT_HOTKEY(0xB000, MyFrame::OnHotKey1)
    EVT_BUTTON(ID_BUTTON_SEND_PORTAL, MyFrame::OnButtonSend)
    EVT_BUTTON(ID_BUTTON_SEND_SCREENSHOT, MyFrame::OnButtonSend)
    EVT_BUTTON(ID_BUTTON_CANCEL, MyFrame::OnButtonCancel)
    EVT_CLOSE(MyFrame::OnWindowClose)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
    EVT_LIST_ITEM_SELECTED(ID_LISTCTRL_PATIENTS, MyListCtrl::OnSelected)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(Config_dialog, wxDialog)
    EVT_BUTTON(wxID_ANY, Config_dialog::OnButton)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MyApp, wxApp)
    EVT_QUERY_END_SESSION(MyApp::OnQueryEndSession)
END_EVENT_TABLE()


IMPLEMENT_APP(MyApp)

void
popup (char* fmt, ...)
{
    va_list argptr;
    va_start (argptr, fmt);

    wxMessageBox (
	wxString::FormatV (fmt, argptr), 
	wxT("Mondoshot"), 
	wxOK | wxICON_INFORMATION);

    va_end (argptr);
}

/* -----------------------------------------------------------------------
   MyApp
   ----------------------------------------------------------------------- */
bool
MyApp::OnInit ()
{
    /* Refuse to launch multiple instances */
    //const wxString name = wxString::Format("Mondoshot-%s", wxGetUserId().c_str());
    const wxString name = wxString::Format("Mondoshot");
    m_checker = new wxSingleInstanceChecker (name);
    if (m_checker->IsAnotherRunning()) {
        return false;
    }

    /* Initialize JPEG library */
    ::wxInitAllImageHandlers ();

    /* Initialize configuration settings */
    config_initialize ();

    /* Create and initialize main window */
    MyFrame* frame = new MyFrame( wxT("Mondoshot"), wxPoint(-1,-1), wxSize(600,500));
    frame->OnInit ();
    SetTopWindow (frame);

    return true;
}

void
MyApp::OnQueryEndSession (wxCloseEvent& event)
{
    wxWindow* frame = GetTopWindow();
    frame->Close (TRUE);
}

int
MyApp::OnExit ()
{
    delete m_checker;
    return 0;
}

/* -----------------------------------------------------------------------
   MyFrame
   ----------------------------------------------------------------------- */
MyFrame::MyFrame (const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
    wxMenuBar *menuBar = new wxMenuBar;
    wxMenu *menuFile = new wxMenu;

    menuFile->Append (ID_MENU_SETTINGS, wxT("&Settings..."));
    menuFile->AppendSeparator ();
    menuFile->Append (ID_MENU_ABOUT, wxT("&About..."));
    menuFile->AppendSeparator ();
    menuFile->Append (ID_MENU_QUIT, wxT("E&xit"));
    menuBar->Append (menuFile, wxT("&File"));
    this->SetMenuBar (menuBar);

    m_panel = new wxPanel (this, -1);

    wxButton *send_portal = new wxButton (m_panel, ID_BUTTON_SEND_PORTAL, wxT("Send Portal"));
    wxButton *send_screenshot = new wxButton (m_panel, ID_BUTTON_SEND_SCREENSHOT, wxT("Send Screenshot"));
    wxButton *cancel = new wxButton (m_panel, ID_BUTTON_CANCEL, wxT("Cancel"));

    wxBoxSizer *vbox = new wxBoxSizer (wxVERTICAL);
    wxFlexGridSizer *fgs = new wxFlexGridSizer (2, 2, 9, 25);
    wxBoxSizer *hbox1 = new wxBoxSizer (wxHORIZONTAL);
    wxBoxSizer *hbox2 = new wxBoxSizer (wxHORIZONTAL);
    wxBoxSizer *hbox3 = new wxBoxSizer (wxHORIZONTAL);

    wxStaticText *label_patient_id =  new wxStaticText (m_panel, wxID_ANY, wxT("Patient ID"));
    wxStaticText *label_patient_name =  new wxStaticText (m_panel, wxID_ANY, wxT("Patient Name"));
    this->m_textctrl_patient_id = new wxTextCtrl (m_panel, ID_TEXTCTRL_PATIENT_ID);
    this->m_textctrl_patient_name = new wxTextCtrl (m_panel, ID_TEXTCTRL_PATIENT_NAME);

    fgs->Add (label_patient_id);
    fgs->Add (m_textctrl_patient_id, 1, wxEXPAND);
    fgs->Add (label_patient_name);
    fgs->Add (m_textctrl_patient_name, 1, wxEXPAND);
    fgs->AddGrowableCol (1, 1);
    fgs->Layout ();

    /* Set up listbox */
    this->m_listctrl_patients = new MyListCtrl (
	this->m_panel, 
	ID_LISTCTRL_PATIENTS,
	wxDefaultPosition, 
	wxDefaultSize,
	wxLC_REPORT | wxLC_SINGLE_SEL | wxSUNKEN_BORDER | wxLC_EDIT_LABELS);
    wxListItem itemCol;
    itemCol.SetText (_T("Patient ID"));
    itemCol.SetAlign (wxLIST_FORMAT_LEFT);
    this->m_listctrl_patients->InsertColumn (0, itemCol);

    itemCol.SetText (_T("Patient Name"));
    itemCol.SetAlign (wxLIST_FORMAT_LEFT);
    this->m_listctrl_patients->InsertColumn (1, itemCol);

    itemCol.SetText (_T("Latest Image"));
    itemCol.SetAlign (wxLIST_FORMAT_LEFT);
    this->m_listctrl_patients->InsertColumn (2, itemCol);

    this->m_listctrl_patients->SetColumnWidth (0, 100);
    this->m_listctrl_patients->SetColumnWidth (1, 150);
    this->m_listctrl_patients->SetColumnWidth (2, 150);

    /* Populate listbox */
    this->listctrl_patients_populate ();

    hbox1->Add (this->m_listctrl_patients, 1, wxEXPAND);

    //hbox2->Add (new wxTextCtrl (m_panel, wxID_ANY), 1);
    //hbox2->Layout ();

    hbox3->Add (send_screenshot);
    hbox3->AddSpacer (20);
    hbox3->Add (send_portal);
    hbox3->AddSpacer (20);
    hbox3->Add (cancel);

    vbox->Add (fgs, 0, wxALL | wxEXPAND, 15);
    vbox->Add (hbox1, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    //vbox->Add (hbox2, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    vbox->Add(-1, 10);
    vbox->Add (hbox3, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    this->m_panel->SetSizer (vbox);
}

void MyFrame::OnMenuAbout (wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox (wxT("Mondoshot Version " PLASTIMATCH_VERSION_STRING "   "),
        wxT("MONDOSHOT"), wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnMenuSettings (wxCommandEvent& WXUNUSED(event))
{
    Config_dialog dlg (this);
    dlg.ShowModal ();
}

void MyFrame::OnMenuQuit (wxCommandEvent& WXUNUSED(event))
{
    this->Close (TRUE);
}

void MyFrame::OnButtonCancel (wxCommandEvent& WXUNUSED(event))
{
    /* Hide dialog box */
    this->Show (FALSE);
}

void
MyFrame::OnWindowClose (wxCloseEvent& event)
{
    //popup ("Event type = %d (%d)", event.GetEventType(), wxEVT_CLOSE_WINDOW);
    if (event.GetEventType() == wxEVT_CLOSE_WINDOW && event.CanVeto ()) {
	/* Hide dialog box */
	this->Show (FALSE);
	event.Veto ();
    } else {
	this->Destroy ();
    }
}

void
MyFrame::OnButtonSend (wxCommandEvent& event)
{
    wxString patient_name, patient_id;

    /* Save a copy */
#if defined (commentout)
    this->m_bitmap.SaveFile (
	::config.data_directory + wxString ("/tmp.jpg"),
	wxBITMAP_TYPE_JPEG);
    this->m_bitmap.SaveFile (
	::config.data_directory + wxString ("/tmp.png"),
	wxBITMAP_TYPE_PNG);
#endif

    /* Validate input fields */
    patient_name = this->m_textctrl_patient_name->GetValue ();
    if (patient_name.IsEmpty ()) {
	wxMessageBox (wxT("Please enter a patient name"),
	    wxT("MONDOSHOT"), wxOK | wxICON_INFORMATION, this);
	return;
    }
    patient_id = this->m_textctrl_patient_id->GetValue ();
    if (patient_id.IsEmpty ()) {
	wxMessageBox (wxT("Please enter a patient id"),
	    wxT("MONDOSHOT"), wxOK | wxICON_INFORMATION, this);
	return;
    }

    /* Hide dialog box */
    this->Show (FALSE);

    /* We need three filenames.  One for png storage (color), one for 
       dcm storage (grayscale), and one with a simple filename for 
       transmission using storescu. */
    wxString filename_base = wxString::Format ("%s [%s] [%s]",
        (const char*) wxDateTime::Now().Format("%Y-%m-%d-%H%M%S"),
        (const char*) patient_id,
        (const char*) patient_name
    );
    for (unsigned int i = 0; i < wxFileName::GetForbiddenChars().Len(); i++) {
	filename_base.Replace (wxFileName::GetForbiddenChars().Mid(i,1), "-", true);
    }
    filename_base = ::config.data_directory + wxString ("/") + filename_base;
    wxString png_filename = wxString::Format ("%s.png", filename_base);
    wxString dcm_filename = wxString::Format ("%s.dcm", filename_base);
    wxString storescu_filename = ::config.data_directory + wxString ("/mondoshot.dcm");

    /* Save the color image as png */
    this->m_bitmap.SaveFile (png_filename, wxBITMAP_TYPE_PNG);

    /* Get an image we can manipulate */
    wxImage image = this->m_bitmap.ConvertToImage ();

    /* Convert to grayscale for dicom.  Also, do whitening if requested. */
    if (::config.white_checkbox) {

        /* Get the ROI which should be whitened */
        // cmin, cmax, rmin, rmax
        int white_roi[4] = { 0, image.GetWidth()-1, 0, image.GetHeight()-1 };
        long value;
        if (::config.white_roi_cmin.ToLong(&value)) {
            white_roi[0] = value;
        }
        if (::config.white_roi_cmax.ToLong(&value)) {
            white_roi[1] = value;
        }
        if (::config.white_roi_rmin.ToLong(&value)) {
            white_roi[2] = value;
        }
        if (::config.white_roi_rmax.ToLong(&value)) {
            white_roi[3] = value;
        }
        if (white_roi[0] < 0) {
            white_roi[0] = 0;
        }
        if (white_roi[1] > image.GetWidth()-1) {
            white_roi[1] = image.GetWidth()-1;
        }
        if (white_roi[2] < 0) {
            white_roi[2] = 0;
        }
        if (white_roi[3] > image.GetHeight()-1) {
            white_roi[3] = image.GetHeight()-1;
        }

        /* Here we do a manual overwriting of the image data, setting 
           color pixels to white. */
        unsigned char* bytes = image.GetData ();
        for (int r = 0; r < image.GetHeight (); r++) {
            for (int c = 0; c < image.GetWidth (); c++) {
                int p = r * image.GetWidth() + c;
                float rd = (float) bytes[3 * p + 0];
                float gn = (float) bytes[3 * p + 1];
                float bl = (float) bytes[3 * p + 2];
                float intensity = (rd + gn + bl) / 3;
                float min_rgb = rd;
                if (gn < min_rgb) min_rgb = gn;
                if (bl < min_rgb) min_rgb = bl;
                float saturation = 0;
                if (intensity > 0) {
                    saturation = 1 - min_rgb / intensity;
                }

                if (c >= white_roi[0] && c <= white_roi[1]
                    && r >= white_roi[2] && r <= white_roi[3]
                    && saturation > 0)
                {
                    /* Do white replacement */
                    bytes[3 * p] = 255;
                }
                else {
                    bytes[3 * p] = intensity;
                }
            }
        }
    } else {
        image = image.ConvertToGreyscale ();
    }

    /* Figure out screen region of interest to save */
    // cmin, cmax, rmin, rmax
    int capture_roi[4] = { 0, image.GetWidth()-1, 0, image.GetHeight()-1 };
    if (::config.capture_checkbox) {
        long value;
        if (::config.capture_roi_cmin.ToLong(&value)) {
            capture_roi[0] = value;
        }
        if (::config.capture_roi_cmax.ToLong(&value)) {
            capture_roi[1] = value;
        }
        if (::config.capture_roi_rmin.ToLong(&value)) {
            capture_roi[2] = value;
        }
        if (::config.capture_roi_rmax.ToLong(&value)) {
            capture_roi[3] = value;
        }
        if (capture_roi[0] < 0) {
            capture_roi[0] = 0;
        }
        if (capture_roi[1] > image.GetWidth()-1) {
            capture_roi[1] = image.GetWidth()-1;
        }
        if (capture_roi[2] < 0) {
            capture_roi[2] = 0;
        }
        if (capture_roi[3] > image.GetHeight()-1) {
            capture_roi[3] = image.GetHeight()-1;
        }
    }

    /* Convert tri-plane RGB to single plane grayscale for transmission 
       and storage.  Simultaneously crop to capture ROI.  We modify the 
       RGB image in-place to accomplish this. */
    unsigned char* bytes = image.GetData ();
    for (int r = capture_roi[2]; r <= capture_roi[3]; r++) {
        for (int c = capture_roi[0]; c <= capture_roi[1]; c++) {
            int img_p = r * image.GetWidth() + c;
            int roi_p = (r - capture_roi[2]) 
                * (capture_roi[1] - capture_roi[0] + 1)
                + c - capture_roi[0];
            bytes[roi_p] = bytes[3 * img_p];
        }
    }

    /* Create dicom storage file */
    mondoshot_dicom_create_file (
        capture_roi[3] - capture_roi[2] + 1,
        capture_roi[1] - capture_roi[0] + 1,
        bytes, 
        0, 
        (event.GetId() == ID_BUTTON_SEND_PORTAL), 
        (const char*) patient_id,
        (const char*) patient_name,
        (const char*) dcm_filename);

    /* Unfortunately, the storescu program can't be used with these 
       kinds of complex filenames.  We create the second file with 
       the more mundane filename.  Normally I would rename (or link) 
       the file instead of creating two, but wxWidgets doesn't have an 
       API call for renaming. */
    mondoshot_dicom_create_file (
        capture_roi[3] - capture_roi[2] + 1,
        capture_roi[1] - capture_roi[0] + 1,
        bytes, 
        0, 
        (event.GetId() == ID_BUTTON_SEND_PORTAL), 
        (const char*) patient_id,
        (const char*) patient_name,
        (const char*) storescu_filename);

    /* Send the file, using the short filename */
    wxString cmd = wxString ("storescu -v ")
        + wxString ("-aet ") + ::config.local_aet + " "
        + wxString ("-aec ") + ::config.remote_aet + " "
        + ::config.remote_ip + " "
        + ::config.remote_port + " "
        + "\"" + storescu_filename + "\"";

    long rc = ::wxExecute (cmd, wxEXEC_SYNC);
    if (rc != 0) {
        popup ("Mondoshot failed to send image to relay");
    }

    /* Insert patient into the database */
    sqlite_patients_insert_record (patient_id, patient_name);

    /* Refresh listbox */
    this->listctrl_patients_populate ();
}

bool MyFrame::OnInit ()
{
    bool rc;
    rc = this->RegisterHotKey (0xB000, 0, wxCharCodeWXToMSW(WXK_F11));

    wxSize screenSize = wxGetDisplaySize();
    this->m_bitmap.Create (screenSize.x, screenSize.y);

    return true;
}

void MyFrame::OnHotKey1 (wxKeyEvent& WXUNUSED(event))
{
    /* Save screenshot */
    wxSize screenSize = wxGetDisplaySize();
    wxScreenDC dc;
    wxMemoryDC memDC;
    memDC.SelectObject (this->m_bitmap);
    memDC.Blit (0, 0, screenSize.x, screenSize.y, &dc, 0, 0);
    memDC.SelectObject (wxNullBitmap);

    this->Show (TRUE);
}

void
MyFrame::listctrl_patients_populate (void)
{
    // to speed up inserting we hide the control temporarily
    this->m_listctrl_patients->Hide ();
    this->m_listctrl_patients->DeleteAllItems ();
    sqlite_patients_query (this);
    if (this->m_listctrl_patients->GetItemCount() > 0) {
        this->m_listctrl_patients->SetItemState (0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }

    this->m_listctrl_patients->Show();
}

/* -----------------------------------------------------------------------
   MyListCtrl
   ----------------------------------------------------------------------- */
void
MyListCtrl::OnSelected (wxListEvent& event)
{
    bool rc;
    wxListItem info;
    wxString patient_id, patient_name;

    info.m_itemId = event.m_itemIndex;
    info.m_mask = wxLIST_MASK_TEXT;

    info.m_col = 0;
    rc = this->GetItem (info);
    if (!rc) {
        return;
    }
    patient_id = info.m_text;

    info.m_col = 1;
    rc = this->GetItem (info);
    if (!rc) {
        return;
    }
    patient_name = info.m_text;

    MyFrame *frame = (MyFrame*) this->GetParent()->GetParent();
    frame->m_textctrl_patient_id->SetValue (patient_id);
    frame->m_textctrl_patient_name->SetValue (patient_name);
}

/* -----------------------------------------------------------------------
   Config_dialog
   ----------------------------------------------------------------------- */
Config_dialog::Config_dialog (wxWindow *parent)
    : wxDialog(parent, wxID_ANY, wxString(_T("Mondoshot Configuration")))
{
    wxBoxSizer *vbox = new wxBoxSizer (wxVERTICAL);
    wxFlexGridSizer *edit_sizer = new wxFlexGridSizer (5, 2, 9, 25);
    wxBoxSizer *capture_checkbox_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer *capture_roi_sizer = new wxFlexGridSizer (2, 4, 9, 25);
    wxBoxSizer *white_checkbox_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer *white_roi_sizer = new wxFlexGridSizer (2, 4, 9, 25);
    wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);

    /* Edit fields at top */
    wxStaticText *label_remote_ip =  new wxStaticText (this, wxID_ANY, wxT("Dicom Remote IP"));
    wxStaticText *label_remote_port =  new wxStaticText (this, wxID_ANY, wxT("Dicom Remote Port"));
    wxStaticText *label_remote_aet =  new wxStaticText (this, wxID_ANY, wxT("Dicom Remote AET"));
    wxStaticText *label_local_aet =  new wxStaticText (this, wxID_ANY, wxT("Dicom Local AET"));
    wxStaticText *label_data_directory =  new wxStaticText (this, wxID_ANY, wxT("Local data directory"));
    this->m_textctrl_remote_ip = new wxTextCtrl (this, wxID_ANY, _T(""));
    this->m_textctrl_remote_port = new wxTextCtrl (this, wxID_ANY);
    this->m_textctrl_remote_aet = new wxTextCtrl (this, wxID_ANY);
    this->m_textctrl_local_aet = new wxTextCtrl (this, wxID_ANY);
    this->m_textctrl_data_directory = new wxTextCtrl (this, wxID_ANY, _T(""), wxDefaultPosition, wxSize(200, wxDefaultCoord));
    edit_sizer->Add (label_remote_ip);
    edit_sizer->Add (m_textctrl_remote_ip, 1, wxEXPAND);
    edit_sizer->Add (label_remote_port);
    edit_sizer->Add (m_textctrl_remote_port, 1, wxEXPAND);
    edit_sizer->Add (label_remote_aet);
    edit_sizer->Add (m_textctrl_remote_aet, 1, wxEXPAND);
    edit_sizer->Add (label_local_aet);
    edit_sizer->Add (m_textctrl_local_aet, 1, wxEXPAND);
    edit_sizer->Add (label_data_directory);
    edit_sizer->Add (m_textctrl_data_directory, 1, wxEXPAND);
    edit_sizer->AddGrowableCol (1, 1);
    edit_sizer->Layout ();

    /* Capture checkbox */
    m_capture_checkbox =  new wxCheckBox (this, wxID_ANY, wxT("Limit capture to region"));
    capture_checkbox_sizer->Add (m_capture_checkbox, 0, wxALIGN_LEFT | wxALL, 10);
    
    /* Capture ROI */
    wxStaticText *label_capture_roi_cmin =  new wxStaticText (this, wxID_ANY, wxT("Col min"));
    wxStaticText *label_capture_roi_cmax =  new wxStaticText (this, wxID_ANY, wxT("Col max"));
    wxStaticText *label_capture_roi_rmin =  new wxStaticText (this, wxID_ANY, wxT("Row min"));
    wxStaticText *label_capture_roi_rmax =  new wxStaticText (this, wxID_ANY, wxT("Row max"));
    this->m_textctrl_capture_roi_cmin = new wxTextCtrl (this, wxID_ANY, _T(""));
    this->m_textctrl_capture_roi_cmax = new wxTextCtrl (this, wxID_ANY, _T(""));
    this->m_textctrl_capture_roi_rmin = new wxTextCtrl (this, wxID_ANY, _T(""));
    this->m_textctrl_capture_roi_rmax = new wxTextCtrl (this, wxID_ANY, _T(""));
    capture_roi_sizer->Add (label_capture_roi_cmin);
    capture_roi_sizer->Add (m_textctrl_capture_roi_cmin, 1, wxEXPAND);
    capture_roi_sizer->Add (label_capture_roi_cmax);
    capture_roi_sizer->Add (m_textctrl_capture_roi_cmax, 1, wxEXPAND);
    capture_roi_sizer->Add (label_capture_roi_rmin);
    capture_roi_sizer->Add (m_textctrl_capture_roi_rmin, 1, wxEXPAND);
    capture_roi_sizer->Add (label_capture_roi_rmax);
    capture_roi_sizer->Add (m_textctrl_capture_roi_rmax, 1, wxEXPAND);
    
    /* Whiting checkbox */
    m_white_checkbox =  new wxCheckBox (this, wxID_ANY, wxT("Convert colors to white"));
    white_checkbox_sizer->Add (m_white_checkbox, 0, wxALIGN_LEFT | wxALL, 10);
    
    /* Whitening ROI */
    wxStaticText *label_white_roi_cmin =  new wxStaticText (this, wxID_ANY, wxT("Col min"));
    wxStaticText *label_white_roi_cmax =  new wxStaticText (this, wxID_ANY, wxT("Col max"));
    wxStaticText *label_white_roi_rmin =  new wxStaticText (this, wxID_ANY, wxT("Row min"));
    wxStaticText *label_white_roi_rmax =  new wxStaticText (this, wxID_ANY, wxT("Row max"));
    this->m_textctrl_white_roi_cmin = new wxTextCtrl (this, wxID_ANY, _T(""));
    this->m_textctrl_white_roi_cmax = new wxTextCtrl (this, wxID_ANY, _T(""));
    this->m_textctrl_white_roi_rmin = new wxTextCtrl (this, wxID_ANY, _T(""));
    this->m_textctrl_white_roi_rmax = new wxTextCtrl (this, wxID_ANY, _T(""));
    white_roi_sizer->Add (label_white_roi_cmin);
    white_roi_sizer->Add (m_textctrl_white_roi_cmin, 1, wxEXPAND);
    white_roi_sizer->Add (label_white_roi_cmax);
    white_roi_sizer->Add (m_textctrl_white_roi_cmax, 1, wxEXPAND);
    white_roi_sizer->Add (label_white_roi_rmin);
    white_roi_sizer->Add (m_textctrl_white_roi_rmin, 1, wxEXPAND);
    white_roi_sizer->Add (label_white_roi_rmax);
    white_roi_sizer->Add (m_textctrl_white_roi_rmax, 1, wxEXPAND);
    
    /* Buttons at bottom */
    m_button_save = new wxButton (this, wxID_ANY, _T("&Save"));
    m_button_cancel = new wxButton (this, wxID_CANCEL, _T("&Cancel"));
    button_sizer->Add (m_button_save, 0, wxALIGN_CENTER | wxALL, 5);
    button_sizer->Add (m_button_cancel, 0, wxALIGN_CENTER | wxALL, 5);

    /* Set values */
    m_textctrl_remote_ip->SetValue (::config.remote_ip);
    m_textctrl_remote_port->SetValue (::config.remote_port);
    m_textctrl_remote_aet->SetValue (::config.remote_aet);
    m_textctrl_local_aet->SetValue (::config.local_aet);
    m_textctrl_data_directory->SetValue (::config.data_directory);
    m_capture_checkbox->SetValue (::config.capture_checkbox);
    m_textctrl_capture_roi_cmin->SetValue (::config.capture_roi_cmin);
    m_textctrl_capture_roi_cmax->SetValue (::config.capture_roi_cmax);
    m_textctrl_capture_roi_rmin->SetValue (::config.capture_roi_rmin);
    m_textctrl_capture_roi_rmax->SetValue (::config.capture_roi_rmax);
    m_white_checkbox->SetValue (::config.white_checkbox);
    m_textctrl_white_roi_cmin->SetValue (::config.white_roi_cmin);
    m_textctrl_white_roi_cmax->SetValue (::config.white_roi_cmax);
    m_textctrl_white_roi_rmin->SetValue (::config.white_roi_rmin);
    m_textctrl_white_roi_rmax->SetValue (::config.white_roi_rmax);

    /* Sizer stuff */
    vbox->Add (edit_sizer, 0, wxALL | wxEXPAND, 15);
    vbox->Add (capture_checkbox_sizer, 0, wxALL | wxEXPAND, 10);
    vbox->Add (capture_roi_sizer, 0, wxALL | wxEXPAND, 10);
    vbox->Add (white_checkbox_sizer, 0, wxALL | wxEXPAND, 10);
    vbox->Add (white_roi_sizer, 0, wxALL | wxEXPAND, 10);
    vbox->Add (button_sizer, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);
    this->SetSizer (vbox);
    vbox->SetSizeHints (this);
    vbox->Fit (this);

    m_button_save->SetFocus();
    m_button_save->SetDefault();
}

void Config_dialog::OnButton (wxCommandEvent& event)
{
    if (event.GetEventObject() == m_button_save) {

        ::config.remote_ip = m_textctrl_remote_ip->GetValue ();
        ::config.remote_port = m_textctrl_remote_port->GetValue ();
        ::config.remote_aet = m_textctrl_remote_aet->GetValue ();
        ::config.local_aet = m_textctrl_local_aet->GetValue ();
        ::config.data_directory = m_textctrl_data_directory->GetValue ();
        ::config.capture_checkbox = m_capture_checkbox->GetValue ();
        ::config.capture_roi_cmin = m_textctrl_capture_roi_cmin->GetValue ();
        ::config.capture_roi_cmax = m_textctrl_capture_roi_cmax->GetValue ();
        ::config.capture_roi_rmin = m_textctrl_capture_roi_rmin->GetValue ();
        ::config.capture_roi_rmax = m_textctrl_capture_roi_rmax->GetValue ();
        ::config.white_checkbox = m_white_checkbox->GetValue ();
        ::config.white_roi_cmin = m_textctrl_white_roi_cmin->GetValue ();
        ::config.white_roi_cmax = m_textctrl_white_roi_cmax->GetValue ();
        ::config.white_roi_rmin = m_textctrl_white_roi_rmin->GetValue ();
        ::config.white_roi_rmax = m_textctrl_white_roi_rmax->GetValue ();
        config_save ();

        wxMessageBox(_T("Configuration saved!"));

        this->EndModal (0);

    } else {
        event.Skip();
    }
}

/* -----------------------------------------------------------------------
   configuration data
   ----------------------------------------------------------------------- */
void
config_save (void)
{
    /* Load from config file */
    wxConfig *wxconfig = new wxConfig("Mondoshot");
    wxconfig->Write ("remote_ip", ::config.remote_ip);
    wxconfig->Write ("remote_port", ::config.remote_port);
    wxconfig->Write ("remote_aet", ::config.remote_aet);
    wxconfig->Write ("local_aet", ::config.local_aet);
    wxconfig->Write ("data_directory", ::config.data_directory);
    wxconfig->Write ("capture_checkbox", ::config.capture_checkbox);
    wxconfig->Write ("capture_roi_cmin", ::config.capture_roi_cmin);
    wxconfig->Write ("capture_roi_cmax", ::config.capture_roi_cmax);
    wxconfig->Write ("capture_roi_rmin", ::config.capture_roi_rmin);
    wxconfig->Write ("capture_roi_rmax", ::config.capture_roi_rmax);
    wxconfig->Write ("white_checkbox", ::config.white_checkbox);
    wxconfig->Write ("white_roi_cmin", ::config.white_roi_cmin);
    wxconfig->Write ("white_roi_cmax", ::config.white_roi_cmax);
    wxconfig->Write ("white_roi_rmin", ::config.white_roi_rmin);
    wxconfig->Write ("white_roi_rmax", ::config.white_roi_rmax);

    delete wxconfig;
}

void
config_initialize (void)
{
    /* Load from config file */
    wxConfig *wxconfig = new wxConfig("Mondoshot");
    wxconfig->Read ("remote_ip", &::config.remote_ip, wxT("132.183.1.1"));
    wxconfig->Read ("remote_port", &::config.remote_port, wxT("104"));
    wxconfig->Read ("remote_aet", &::config.remote_aet, wxT("IMPAC_DCM_SCP"));
    wxconfig->Read ("local_aet", &::config.local_aet, wxT("MONDOSHOT"));
    wxconfig->Read ("data_directory", &::config.data_directory, wxT("C:/tmp"));
    wxconfig->Read ("capture_checkbox", &::config.capture_checkbox, false);
    wxconfig->Read ("capture_roi_cmin", &::config.capture_roi_cmin, wxT("0"));
    wxconfig->Read ("capture_roi_cmax", &::config.capture_roi_cmax, wxT("-1"));
    wxconfig->Read ("capture_roi_rmin", &::config.capture_roi_rmin, wxT("0"));
    wxconfig->Read ("capture_roi_rmax", &::config.capture_roi_rmax, wxT("-1"));
    wxconfig->Read ("white_checkbox", &::config.white_checkbox, false);
    wxconfig->Read ("white_roi_cmin", &::config.white_roi_cmin, wxT("0"));
    wxconfig->Read ("white_roi_cmax", &::config.white_roi_cmax, wxT("-1"));
    wxconfig->Read ("white_roi_rmin", &::config.white_roi_rmin, wxT("0"));
    wxconfig->Read ("white_roi_rmax", &::config.white_roi_rmax, wxT("-1"));

    /* Save settings */
    config_save ();

    delete wxconfig;
}

/* -----------------------------------------------------------------------
   sqlite stuff
   ----------------------------------------------------------------------- */
void
sqlite_patients_insert_record (wxString patient_id, wxString patient_name)
{
    int rc;
    sqlite3 *db;
    wxString wx_sql;
    wxString filename;
    const char *sql;
    char *sqlite3_err;

    /* Patient names may have apostrophes.  Escape these. */
    patient_name.Replace ("'", "''", true);

    filename = ::config.data_directory + wxString ("/mondoshot.sqlite");
    //rc = sqlite3_open ("C:/tmp/mondoshot.sqlite", &db);
    rc = sqlite3_open ((const char*) filename, &db);
    if (rc) {
        popup ("Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close (db);
        exit (1);
    }

    wx_sql = wxString::Format (
        "INSERT INTO patient_screenshots (patient_id, patient_name, screenshot_timestamp)"
        "values ('%s', '%s', julianday('now'));",
        patient_id, patient_name);
    sql = (const char*) wx_sql;
    rc = sqlite3_exec (db, sql, 0, 0, &sqlite3_err);
    if (rc != SQLITE_OK) {
        popup ("SQL error: %s\n", sqlite3_err);
        sqlite3_free (sqlite3_err);
    }

    sqlite3_close (db);
}

int
sqlite_patients_query_callback (void* data, int argc, char** argv, char** column_names)
{
    int i;
    struct sqlite_populate_cbstruct *cbstruct = (struct sqlite_populate_cbstruct *) data;
    MyFrame *frame = cbstruct->m_frame;
    MyListCtrl *patient_list = frame->m_listctrl_patients;
    int patient_name_idx = -1;
    int patient_id_idx = -1;
    int last_image_idx = -1;

    /* Check column_names */
    for (i = 0; i < argc; i++) {
        if (!strcmp (column_names[i], "patient_name")) {
            patient_name_idx = i;
        }
        if (!strcmp (column_names[i], "patient_id")) {
            patient_id_idx = i;
        }
        if (!strcmp (column_names[i], "datetime(MAX(screenshot_timestamp))")) {
            last_image_idx = i;
        }
    }
    if (patient_name_idx == -1 || patient_id_idx == -1) {
        return -1;
    }

    char* patient_name = argv[patient_name_idx];
    char* patient_id = argv[patient_id_idx];
    char* last_image = argv[last_image_idx];

    if (patient_name && patient_id && last_image) {
        wxString buf;
        int list_index = cbstruct->m_list_index ++;

        buf = patient_id;
        long tmp = patient_list->InsertItem (list_index, buf, 0);
        patient_list->SetItemData (tmp, list_index);

        buf = patient_name;
        patient_list->SetItem (tmp, 1, buf);

        buf = last_image;
        patient_list->SetItem (tmp, 2, buf);
    }

    return 0;
}

void
sqlite_patients_query (MyFrame* frame)
{
    int rc;
    sqlite3 *db;
    char *sql;
    char *sqlite3_err;
    wxString filename;
    struct sqlite_populate_cbstruct cbstruct;

    filename = ::config.data_directory + wxString ("/mondoshot.sqlite");
    //rc = sqlite3_open ("C:/tmp/mondoshot.sqlite", &db);
    rc = sqlite3_open ((const char*) filename, &db);
    if (rc) {
        popup ("Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close (db);
        exit (1);
    }

    sql = "CREATE TABLE IF NOT EXISTS patient_screenshots ( oi INTEGER PRIMARY KEY, patient_id TEXT, patient_name TEXT, screenshot_timestamp DATE );";
    rc = sqlite3_exec (db, sql, 0, 0, &sqlite3_err);
    if (rc != SQLITE_OK) {
        popup ("SQL error: %s\n", sqlite3_err);
        sqlite3_free (sqlite3_err);
    }

    cbstruct.m_frame = frame;
    cbstruct.m_list_index = 0;
    sql = 
        "SELECT patient_id,patient_name,datetime(MAX(screenshot_timestamp)) "
        "FROM patient_screenshots GROUP BY patient_id,patient_name "
        "ORDER BY MAX(screenshot_timestamp) DESC;";
    rc = sqlite3_exec (db, sql, sqlite_patients_query_callback, &cbstruct, &sqlite3_err);
    if (rc != SQLITE_OK) {
        popup ("SQL error: %s\n", sqlite3_err);
        sqlite3_free (sqlite3_err);
    }

    sqlite3_close (db);
}
