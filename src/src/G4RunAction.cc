#include "G4Timer.hh"

#include "G4RunAction.hh"
#include "G4Constantes.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "g4root.hh"

#include <fstream>

// #include "G4NeutronHPManager.hh"

using namespace std;

//================================================================================

G4RunAction::G4RunAction() {
    
    timer = new G4Timer;
    nOfReflections_Total = 0;
    nOfDetections_Total = 0;
    TOF_Detections_Total = 0;
}

//================================================================================

G4RunAction::~G4RunAction() {
    
    delete timer;
  
}

//================================================================================

void G4RunAction::BeginOfRunAction(const G4Run* aRun) {
    
    // G4NeutronHPManager::GetInstance()->SetVerboseLevel(0);


    G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;

    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    
    G4cout << "Using " << analysisManager->GetType() << " analysis manager." << G4endl;
    timer->Start();

    analysisManager->OpenFile("OutPut");
    analysisManager->SetVerboseLevel(0);


    // Indica o id do primeiro histograma criado -- default = 0
    analysisManager->SetFirstHistoId(1);
    // Cria histogramas    -- (nome, titulo, nbins, xmin, xmax)
    analysisManager->CreateH1("0","Time of arrival",5000,0.,10);


    // Indica o id da primeira ntuple criada -- default = 0
    analysisManager->SetFirstNtupleId(1);
    //Declara ntuples
    analysisManager->CreateNtuple("ntuple", "data");
    analysisManager->CreateNtupleIColumn("evt");
    analysisManager->CreateNtupleDColumn("x");
    analysisManager->CreateNtupleDColumn("y");
    analysisManager->CreateNtupleDColumn("z");
    analysisManager->CreateNtupleDColumn("t");
    analysisManager->CreateNtupleDColumn("energy");
    analysisManager->CreateNtupleDColumn("theta");
    analysisManager->CreateNtupleDColumn("dx");
    analysisManager->CreateNtupleDColumn("dy");
    analysisManager->CreateNtupleDColumn("dz");
    analysisManager->FinishNtuple();
}

//================================================================================

void G4RunAction::EndOfRunAction(const G4Run* aRun) {
    G4cout << "End of run." << G4endl;
    timer->Stop();

    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();

    delete G4AnalysisManager::Instance();
        
}

//================================================================================
