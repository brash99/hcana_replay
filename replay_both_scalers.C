  THcAnalyzer* analyzer = new THcAnalyzer;
void replay_both_scalers(Int_t RunNumber=52949, Int_t FirstToReplay=1, Int_t MaxEventToReplay=10000000) {

  //
  //  Steering script to test hodoscope decoding
  //
  
  //Int_t RunNumber=52949;
  
  TString data_dir = "/home/brash/Research/daq04";
  TString experiment = "daq04";
  TString RunFileNamePattern = data_dir + "/" + experiment + "_%d.log.0";
  TString run_file = data_dir + "/" + experiment + Form("_%d.log.0",RunNumber);

  if ( gSystem->AccessPathName(run_file) ) {
   Error("replay_both_scalers.C", "Input file does not exist: %s", run_file.Data() );
   return 1;
  }

  gHcParms->Define("gen_run_number", "Run Number", RunNumber);
  gHcParms->AddString("g_ctp_database_filename", "jan05.database");
  
  gHcParms->Load(gHcParms->GetString("g_ctp_database_filename"), RunNumber);

  // g_ctp_parm_filename and g_decode_map_filename should now be defined

  gHcParms->Load(gHcParms->GetString("g_ctp_kinematics_filename"), RunNumber);
  gHcParms->Load(gHcParms->GetString("g_ctp_parm_filename"));
  gHcParms->Load("hcana.param");

  // Constants not in ENGINE PARAM files that we want to be
  // configurable
  //gHcParms->Load(Form("PARAM/%05d/general.param",RunNumber));

  // Generate db_cratemap to correspond to map file contents
  char command[100];
  sprintf(command,"./make_cratemap.pl < %s > db_cratemap.dat",gHcParms->GetString("g_decode_map_filename"));
  system(command);

  // Load the Hall C style detector map
  gHcDetectorMap=new THcDetectorMap();
  gHcDetectorMap->Load(gHcParms->GetString("g_decode_map_filename"));
  
  // Set up the equipment to be analyzed.
  
  THaApparatus* HMS = new THcHallCSpectrometer("H","HMS");
  gHaApps->Add( HMS );
  
  HMS->AddDetector( new THcHodoscope("hod", "Hodoscope" ));
  HMS->AddDetector( new THcShower("cal", "Shower" ));
  THcCherenkov* cherenkov = new THcCherenkov("cher", "Gas Cerenkov" );
   HMS->AddDetector( cherenkov );
  HMS->AddDetector( new THcDC("dc", "Drift Chambers" ));
  THcAerogel* aerogel = new THcAerogel("aero", "Aerogel Cerenkov" );
  HMS->AddDetector( aerogel );
  //
  THcScalerEvtHandler *hscaler = new THcScalerEvtHandler("HS","HC scaler event type 0");
  hscaler->SetDebugFile((char *)"HScaler.txt");
  gHaEvtHandlers->Add (hscaler);
  //
    THaApparatus* SOS = new THcHallCSpectrometer("S","SOS");
   gHaApps->Add( SOS );
  // Add detectors
  SOS->AddDetector( new THcHodoscope("hod", "Hodoscope" ));
  SOS->AddDetector( new THcShower("cal", "Shower" ));
  SOS->AddDetector( new THcCherenkov("cher", "Gas Cerenkov" ));
  SOS->AddDetector( new THcDC("dc", "Drift Chambers" ));

  // Beamline and its detectors
  THaApparatus * BEAM = new THcRasteredBeam("RB","Rastered Beamline");
  gHaApps->Add( BEAM );

  // setup physics
  gHaPhysics->Add( new THaGoldenTrack( "H.gold", "HMS Golden Track", "H" ));
  gHaPhysics->Add( new THaGoldenTrack( "S.gold", "SOSS Golden Track", "S" ));
  // Set up the analyzer - we use the standard one,
  // but this could be an experiment-specific one as well.
  // The Analyzer controls the reading of the data, executes
  // tests/cuts, loops over Acpparatus's and PhysicsModules,
  // and executes the output routines.
  //  THcAnalyzer* analyzer = new THcAnalyzer;
   

  // A simple event class to be output to the resulting tree.
  // Creating your own descendant of THaEvent is one way of
  // defining and controlling the output.
  THaEvent* event = new THaEvent;
  
  // Define the run(s) that we want to analyze.
  // We just set up one, but this could be many.
  //char RunFileName[100];
  //sprintf(RunFileName,RunFileNamePattern,RunNumber);
  //THaRun* run = new THaRun(RunFileName);
  
  THaRun* run = new THaRun(run_file, "DAQ04 Data");

  // Eventually need to learn to skip over, or properly analyze
  // the pedestal events
  run->SetEventRange(FirstToReplay,MaxEventToReplay);//  Physics Event number, does not
                                // include scaler or control events

  // Define the analysis parameters
  analyzer->SetCountMode( 2 ); // 0 = counter is # of physics triggers
			       //1 = counter is # of all decode reads 
                               //2= counter is event number
  analyzer->SetEvent( event );
  analyzer->SetOutFile(Form("Rootfiles/replay_both_%05d.root",RunNumber));
  analyzer->SetOdefFile("output_replay_both.def");
  analyzer->SetCutFile("cuts_replay_both.def");        // optional
  
  // File to record cuts accounting information
  //  analyzer->SetSummaryFile("summary_example.log"); // optional
  
  analyzer->Process(run);     // start the actual analysis
  analyzer->PrintReport("report.template",Form("replay_both_%05d.report",RunNumber)); 
}
