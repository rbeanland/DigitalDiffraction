// MyDlg2.cpp : implementation file
// Enable double support

#include "stdafx.h"
#include "MyDlg2.h"
#include "NumEdit.h"
#include "boost/lexical_cast.hpp"

// MyDlg2 dialog
IMPLEMENT_DYNAMIC(MyDlg2, CDialog)

MyDlg2::MyDlg2(CWnd* pParent /*=NULL*/)
	: CDialog(MyDlg2::IDD, pParent)
	, want_option_1(FALSE)
	, create_ask_cropped_bool_input(TRUE)
	, show_dlacbed_stack(TRUE)
	, enable_colour_change(TRUE)
	, print_f_and_fimgx(FALSE)
	, print_x_y_in_lacbed(FALSE)
	, display_cluster_gvectors(FALSE)
	, display_mag_theta(FALSE)
	, display_y_yy_vy(FALSE)
	, display_x_xx_vx(FALSE)
	, print_g1x_g1y(FALSE)
	, display_avg_cc(FALSE)
	, print_rr_tinc(FALSE)
	, process_select_all(FALSE)
	, display_ysh_input(FALSE)
	, print_xtpx_input(FALSE)
	, print_xshpx_input(FALSE)
	, display_calibration_input(FALSE)
	, kunprocessed_selected(TRUE)
	, kdarksubtracted_selected(FALSE)
	, kgainnormalized_selected(FALSE)
	, kmaxprocessing_selected(FALSE)
	, print_shift_tilt_input(FALSE)
	, print_time_input(FALSE)
{
	MyDlg2::InititalLoad();
}

MyDlg2::~MyDlg2()
{
}

void MyDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, want_option_1);
	DDX_Check(pDX, IDC_CHECK15, create_ask_cropped_bool_input);
	DDX_Check(pDX, IDC_CHECK16, show_dlacbed_stack);
	DDX_Check(pDX, IDC_CHECK18, enable_colour_change);
	DDX_Check(pDX, IDC_CHECK14, print_f_and_fimgx);
	DDX_Check(pDX, IDC_CHECK13, print_x_y_in_lacbed);
	DDX_Check(pDX, IDC_CHECK12, display_cluster_gvectors);
	DDX_Check(pDX, IDC_CHECK11, display_mag_theta);
	DDX_Check(pDX, IDC_CHECK10, display_y_yy_vy);
	DDX_Check(pDX, IDC_CHECK9, display_x_xx_vx);
	DDX_Check(pDX, IDC_CHECK8, print_g1x_g1y);
	DDX_Check(pDX, IDC_CHECK7, display_avg_cc);
	DDX_Check(pDX, IDC_CHECK6, print_rr_tinc);
	DDX_Check(pDX, IDC_CHECK17, process_select_all);
	DDX_Check(pDX, IDC_CHECK2, display_ysh_input);
	DDX_Check(pDX, IDC_CHECK4, print_xtpx_input);
	DDX_Check(pDX, IDC_CHECK5, print_xshpx_input);
	DDX_Check(pDX, IDC_CHECK3, display_calibration_input);
	DDX_Check(pDX, IDC_CHECK19, kunprocessed_selected);
	DDX_Check(pDX, IDC_CHECK20, kdarksubtracted_selected);
	DDX_Check(pDX, IDC_CHECK21, kgainnormalized_selected);
	DDX_Check(pDX, IDC_CHECK22, kmaxprocessing_selected);
	DDX_Check(pDX, IDC_CHECK23, print_shift_tilt_input);
	DDX_Check(pDX, IDC_CHECK24, print_time_input);
}


BEGIN_MESSAGE_MAP(MyDlg2, CDialog)
	ON_BN_CLICKED(IDC_BUTTON3, &MyDlg2::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &MyDlg2::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_CHECK17, &MyDlg2::OnBnClickedCheck17)
	ON_BN_CLICKED(IDC_CHECK19, &MyDlg2::OnBnClickedCheck19)
	ON_BN_CLICKED(IDC_CHECK20, &MyDlg2::OnBnClickedCheck20)
	ON_BN_CLICKED(IDC_CHECK21, &MyDlg2::OnBnClickedCheck21)
	ON_BN_CLICKED(IDC_CHECK22, &MyDlg2::OnBnClickedCheck22)
END_MESSAGE_MAP()


// MyDlg2 message handlers

// This functions is part of the WorkerClass.
// If you call Start() it will in turn run this function in a seperate Worker Thread, thus not locking up the UI.
void MyDlg2::DoWork()
{
}

void MyDlg2::OnBnClickedButton2()//Save settings button
{
	// TODO: Add your control notification handler code here
	UpdateData();
	DigitalMicrograph::TagGroup Persistent;
	Persistent = DigitalMicrograph::GetPersistentTagGroup();
	std::string Tag_path;
	Tag_path = "DigitalDiffraction:Settings:";
	// Calibration Settings
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Display XSh").c_str(), want_option_1);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Display YSh").c_str(), display_ysh_input);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Print xTpX etc").c_str(), print_xtpx_input);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Print xShpX etc").c_str(), print_xshpx_input);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Display Calibration Image").c_str(), display_calibration_input);
	//Process Settings
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Create and ask to view cropped").c_str(), create_ask_cropped_bool_input);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Show DLACBED stack").c_str(), show_dlacbed_stack);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Enable Colour Change").c_str(), enable_colour_change);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Print F and FimgX").c_str(), print_f_and_fimgx);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Print X,Y in DLACBED Stack").c_str(), print_x_y_in_lacbed);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Display Cluster and gVectors").c_str(), display_cluster_gvectors);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Display Mag Theta").c_str(), display_mag_theta);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Display Y Yy Vy").c_str(), display_y_yy_vy);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Display X Xx Vx").c_str(), display_x_xx_vx);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Print g1X g1Y").c_str(), print_g1x_g1y);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Display Avg Cross Correlation").c_str(), display_avg_cc);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Print Disc Radius Rr and tInc").c_str(), print_rr_tinc);
	DigitalMicrograph::Result("Settings Saved\n");
	//processing setting
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "kUnprocessed").c_str(), kunprocessed_selected);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "kDarkSubtracted").c_str(), kdarksubtracted_selected);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "kGainNormalized").c_str(), kgainnormalized_selected);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "kMaxProcessing").c_str(), kmaxprocessing_selected);
	//Global print controls
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Print Shift Tilt").c_str(), print_shift_tilt_input);
	DigitalMicrograph::TagGroupSetTagAsBoolean(Persistent, (Tag_path + "Print Time").c_str(), print_time_input);
}

void MyDlg2::InititalLoad()//Load settings when DigitalMicrograph is first opened
{
	DigitalMicrograph::TagGroup Persistent;
	Persistent = DigitalMicrograph::GetPersistentTagGroup();
	std::string Tag_path;
	Tag_path = "DigitalDiffraction:Settings:";
	bool temp_display_xsh_input, temp_display_ysh_input, temp_print_xtpx_input, temp_print_xshpx_input, temp_display_calibration_input;
	bool temp_create_ask_cropped_bool_input, temp_show_dlacbed_stack, temp_enable_colour_change, temp_print_f_and_fimgx, temp_print_x_y_in_lacbed, temp_display_cluster_gvectors, temp_display_mag_theta, temp_display_y_yy_vy, temp_display_x_xx_vx, temp_print_g1x_g1y, temp_display_avg_cc, temp_print_rr_tinc;
	bool temp_kunprocessed_selected, temp_kdarksubtracted_selected, temp_kgainnormalized_selected, temp_kmaxprocessing_selected;
	bool temp_print_shift_tilt_input, temp_print_time;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display XSh").c_str(), &temp_display_xsh_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display YSh").c_str(), &temp_display_ysh_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print xTpX etc").c_str(), &temp_print_xtpx_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print xShpX etc").c_str(), &temp_print_xshpx_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Calibration Image").c_str(), &temp_display_calibration_input);

		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Create and ask to view cropped").c_str(), &temp_create_ask_cropped_bool_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Show DLACBED stack").c_str(), &temp_show_dlacbed_stack);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Enable Colour Change").c_str(), &temp_enable_colour_change);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print F and FimgX").c_str(), &temp_print_f_and_fimgx);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print X,Y in DLACBED Stack").c_str(), &temp_print_x_y_in_lacbed);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Cluster and gVectors").c_str(), &temp_display_cluster_gvectors);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Mag Theta").c_str(), &temp_display_mag_theta);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Y Yy Vy").c_str(), &temp_display_y_yy_vy);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display X Xx Vx").c_str(), &temp_display_x_xx_vx);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print g1X g1Y").c_str(), &temp_print_g1x_g1y);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Avg Cross Correlation").c_str(), &temp_display_avg_cc);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print Disc Radius Rr and tInc").c_str(), &temp_print_rr_tinc);

		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kUnprocessed").c_str(), &temp_kunprocessed_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kDarkSubtracted").c_str(), &temp_kdarksubtracted_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kGainNormalized").c_str(), &temp_kgainnormalized_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kMaxProcessing").c_str(), &temp_kmaxprocessing_selected);

		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print Shift Tilt").c_str(), &temp_print_shift_tilt_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print Time").c_str(), &temp_print_time);

		want_option_1 = boost::lexical_cast<BOOL>(temp_display_xsh_input);
		display_ysh_input = boost::lexical_cast<BOOL>(temp_display_ysh_input);
		print_xtpx_input = boost::lexical_cast<BOOL>(temp_print_f_and_fimgx);
		print_xshpx_input = boost::lexical_cast<BOOL>(temp_print_xshpx_input);
		display_calibration_input = boost::lexical_cast<BOOL>(temp_display_calibration_input);

		create_ask_cropped_bool_input = boost::lexical_cast<BOOL>(temp_create_ask_cropped_bool_input);
		show_dlacbed_stack = boost::lexical_cast<BOOL>(temp_show_dlacbed_stack);
		enable_colour_change = boost::lexical_cast<BOOL>(temp_enable_colour_change);
		print_f_and_fimgx = boost::lexical_cast<BOOL>(temp_print_f_and_fimgx);
		print_x_y_in_lacbed = boost::lexical_cast<BOOL>(temp_print_x_y_in_lacbed);
		display_cluster_gvectors = boost::lexical_cast<BOOL>(temp_display_cluster_gvectors);
		display_mag_theta = boost::lexical_cast<BOOL>(temp_display_mag_theta);
		display_y_yy_vy = boost::lexical_cast<BOOL>(temp_display_y_yy_vy);
		display_x_xx_vx = boost::lexical_cast<BOOL>(temp_display_x_xx_vx);
		print_g1x_g1y = boost::lexical_cast<BOOL>(temp_print_g1x_g1y);
		display_avg_cc = boost::lexical_cast<BOOL>(temp_display_avg_cc);
		print_rr_tinc = boost::lexical_cast<BOOL>(temp_print_rr_tinc);

		kunprocessed_selected = boost::lexical_cast<BOOL>(temp_kunprocessed_selected);
		kdarksubtracted_selected = boost::lexical_cast<BOOL>(temp_kdarksubtracted_selected);
		kgainnormalized_selected = boost::lexical_cast<BOOL>(temp_kgainnormalized_selected);
		kmaxprocessing_selected = boost::lexical_cast<BOOL>(temp_kmaxprocessing_selected);

		print_shift_tilt_input= boost::lexical_cast<BOOL>(temp_print_shift_tilt_input);
		print_time_input = boost::lexical_cast<BOOL>(temp_print_time);

		DigitalMicrograph::Result("Settings Loaded\n");
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed to load settings, please save settings\n");
	}
}

void MyDlg2::OnBnClickedButton3()// Load Settings button
{
	// TODO: Add your control notification handler code here
	DigitalMicrograph::TagGroup Persistent;
	Persistent = DigitalMicrograph::GetPersistentTagGroup();
	std::string Tag_path;
	Tag_path = "DigitalDiffraction:Settings:";
	bool temp_display_xsh_input, temp_display_ysh_input, temp_print_xtpx_input, temp_print_xshpx_input, temp_display_calibration_input;
	bool temp_create_ask_cropped_bool_input, temp_show_dlacbed_stack, temp_enable_colour_change, temp_print_f_and_fimgx, temp_print_x_y_in_lacbed, temp_display_cluster_gvectors, temp_display_mag_theta, temp_display_y_yy_vy, temp_display_x_xx_vx, temp_print_g1x_g1y, temp_display_avg_cc, temp_print_rr_tinc;
	bool temp_kunprocessed_selected, temp_kdarksubtracted_selected, temp_kgainnormalized_selected, temp_kmaxprocessing_selected;
	bool temp_print_shift_tilt_input, temp_print_time;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display XSh").c_str(), &temp_display_xsh_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display YSh").c_str(), &temp_display_ysh_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print xTpX etc").c_str(), &temp_print_xtpx_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print xShpX etc").c_str(), &temp_print_xshpx_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Calibration Image").c_str(), &temp_display_calibration_input);

		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Create and ask to view cropped").c_str(), &temp_create_ask_cropped_bool_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Show DLACBED stack").c_str(), &temp_show_dlacbed_stack);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Enable Colour Change").c_str(), &temp_enable_colour_change);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print F and FimgX").c_str(), &temp_print_f_and_fimgx);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print X,Y in DLACBED Stack").c_str(), &temp_print_x_y_in_lacbed);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Cluster and gVectors").c_str(), &temp_display_cluster_gvectors);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Mag Theta").c_str(), &temp_display_mag_theta);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Y Yy Vy").c_str(), &temp_display_y_yy_vy);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display X Xx Vx").c_str(), &temp_display_x_xx_vx);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print g1X g1Y").c_str(), &temp_print_g1x_g1y);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Avg Cross Correlation").c_str(), &temp_display_avg_cc);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print Disc Radius Rr and tInc").c_str(), &temp_print_rr_tinc);

		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kUnprocessed").c_str(), &temp_kunprocessed_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kDarkSubtracted").c_str(), &temp_kdarksubtracted_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kGainNormalized").c_str(), &temp_kgainnormalized_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kMaxProcessing").c_str(), &temp_kmaxprocessing_selected);

		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print Shift Tilt").c_str(), &temp_print_shift_tilt_input);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print Time").c_str(), &temp_print_time);

				
		want_option_1 = boost::lexical_cast<BOOL>(temp_display_xsh_input);
		display_ysh_input = boost::lexical_cast<BOOL>(temp_display_ysh_input);
		print_xtpx_input = boost::lexical_cast<BOOL>(temp_print_f_and_fimgx);
		print_xshpx_input = boost::lexical_cast<BOOL>(temp_print_xshpx_input);
		display_calibration_input = boost::lexical_cast<BOOL>(temp_display_calibration_input);

		create_ask_cropped_bool_input = boost::lexical_cast<BOOL>(temp_create_ask_cropped_bool_input);
		show_dlacbed_stack = boost::lexical_cast<BOOL>(temp_show_dlacbed_stack);
		enable_colour_change = boost::lexical_cast<BOOL>(temp_enable_colour_change);
		print_f_and_fimgx = boost::lexical_cast<BOOL>(temp_print_f_and_fimgx);
		print_x_y_in_lacbed = boost::lexical_cast<BOOL>(temp_print_x_y_in_lacbed);
		display_cluster_gvectors = boost::lexical_cast<BOOL>(temp_display_cluster_gvectors);
		display_mag_theta = boost::lexical_cast<BOOL>(temp_display_mag_theta);
		display_y_yy_vy = boost::lexical_cast<BOOL>(temp_display_y_yy_vy);
		display_x_xx_vx = boost::lexical_cast<BOOL>(temp_display_x_xx_vx);
		print_g1x_g1y = boost::lexical_cast<BOOL>(temp_print_g1x_g1y);
		display_avg_cc = boost::lexical_cast<BOOL>(temp_display_avg_cc);
		print_rr_tinc = boost::lexical_cast<BOOL>(temp_print_rr_tinc);

		kunprocessed_selected = boost::lexical_cast<BOOL>(temp_kunprocessed_selected);
		kdarksubtracted_selected = boost::lexical_cast<BOOL>(temp_kdarksubtracted_selected);
		kgainnormalized_selected = boost::lexical_cast<BOOL>(temp_kgainnormalized_selected);
		kmaxprocessing_selected = boost::lexical_cast<BOOL>(temp_kmaxprocessing_selected);
			
		print_shift_tilt_input = boost::lexical_cast<BOOL>(temp_print_shift_tilt_input);
		print_time_input = boost::lexical_cast<BOOL>(temp_print_time);

		DigitalMicrograph::Result("Settings Loaded\n");
	}
	catch (...)
	{

	}
		UpdateData(0);
}

void MyDlg2::OnBnClickedCheck17()// Select all check box
{
	UpdateData();
	if (process_select_all == true)
	{
		create_ask_cropped_bool_input = true;
		show_dlacbed_stack = true;
		enable_colour_change = true;
		print_f_and_fimgx = true;
		print_x_y_in_lacbed = true;
		display_cluster_gvectors = true;
		display_mag_theta = true;
		display_y_yy_vy = true;
		display_x_xx_vx = true;
		print_g1x_g1y = true;
		display_avg_cc = true;
		print_rr_tinc = true;
		
		UpdateData(0);
	}
	else if (process_select_all == false)
	{
		create_ask_cropped_bool_input = false;
		show_dlacbed_stack = false;
		enable_colour_change = false;
		print_f_and_fimgx = false;
		print_x_y_in_lacbed = false;
		display_cluster_gvectors = false;
		display_mag_theta = false;
		display_y_yy_vy = false;
		display_x_xx_vx = false;
		print_g1x_g1y = false;
		display_avg_cc = false;
		print_rr_tinc = false;

		UpdateData(0);
	}
	// TODO: Add your control notification handler code here
}

//Making sure only one processing option can be selected at once
void MyDlg2::OnBnClickedCheck19()//kUnprocessed check box
{
	UpdateData();
	if (kunprocessed_selected == true)
	{
		kdarksubtracted_selected = false;
		kgainnormalized_selected = false;
		kmaxprocessing_selected = false;
		UpdateData(0);
	}
	// TODO: Add your control notification handler code here
}

void MyDlg2::OnBnClickedCheck20()//kDarkSubtracted check box
{
	UpdateData();
	if (kdarksubtracted_selected == true)
	{
		kunprocessed_selected= false;
		kgainnormalized_selected = false;
		kmaxprocessing_selected = false;
		UpdateData(0);
	}
	// TODO: Add your control notification handler code here
}

void MyDlg2::OnBnClickedCheck21()//kGainNormalized check box
{
	UpdateData();
	if (kgainnormalized_selected == true)
	{
		kunprocessed_selected = false;
		kdarksubtracted_selected = false;
		kmaxprocessing_selected = false;
		UpdateData(0);
	}

	// TODO: Add your control notification handler code here
}

void MyDlg2::OnBnClickedCheck22()//kMaxProcessing chgeck box
{
	UpdateData();
	if (kmaxprocessing_selected == true)
	{
		kunprocessed_selected = false;
		kdarksubtracted_selected = false;
		kgainnormalized_selected = false;
		UpdateData(0);
	}
	// TODO: Add your control notification handler code here
}
