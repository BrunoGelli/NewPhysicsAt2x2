#include "MyPhysicsList.hh"

#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalPhysics.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4eMultipleScattering.hh"
#include "G4hIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4CoulombScattering.hh"


MyPhysicsList::MyPhysicsList() : FTFP_BERT() {
    ReplacePhysics(new G4EmStandardPhysics_option4());
    RegisterPhysics(new G4OpticalPhysics());
}

void MyPhysicsList::DefineMillichargedParticle() {
    if (!G4ParticleTable::GetParticleTable()->FindParticle("millicharged")) {
        new G4ParticleDefinition(
            "millicharged",        // name
            50.0 * CLHEP::MeV,           // mass
            0.0 * CLHEP::MeV,             // width
            0.01*CLHEP::eplus,           // charge
            1,                     // 2*spin
            0, 0,                  // parity, C-conjugation
            0, 0, 0,               // isospin
            "lepton",              // type
            1,                     // lepton number
            0,                     // baryon number
            9999,                  // PDG code
            true,                  // stable
            -1.0,                  // lifetime
            nullptr,               // decay table
            false,                 // shortlived
            "millicharged"         // subtype
        );
    }
}

void MyPhysicsList::ConstructParticle() {
    // Let base FTFP_BERT register all standard particles
    FTFP_BERT::ConstructParticle();
    // Then add mCP
    DefineMillichargedParticle();
}

void MyPhysicsList::ConstructProcess() {
    // Let FTFP_BERT register all standard processes
    FTFP_BERT::ConstructProcess();

    // Add processes for millicharged
    auto* particleIterator = GetParticleIterator();
    particleIterator->reset();

    while ((*particleIterator)()) {
        G4ParticleDefinition* particle = particleIterator->value();
        G4ProcessManager* pmanager = particle->GetProcessManager();

        if (particle->GetParticleName() == "millicharged") {
            pmanager->AddProcess(new G4eMultipleScattering(), -1, 1, 1);
            pmanager->AddProcess(new G4hIonisation(),         -1, 2, 2);
            pmanager->AddProcess(new G4eBremsstrahlung(),     -1, -1, 3);
            pmanager->AddDiscreteProcess(new G4CoulombScattering());
        }
    }
}
