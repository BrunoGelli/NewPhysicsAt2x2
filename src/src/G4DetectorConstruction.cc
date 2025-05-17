    #include "G4DetectorConstruction.hh"
    #include "G4Constantes.hh"

    #include "G4Material.hh"
    #include "G4Element.hh"
    #include "G4NistManager.hh"

    #include "G4Box.hh"
    #include "G4Orb.hh"
    #include "G4Tubs.hh"
    #include "G4Sphere.hh"
    #include "G4Trd.hh"

    #include "G4LogicalVolume.hh"
    #include "G4ThreeVector.hh"
    #include "G4PVPlacement.hh"
    #include "G4AutoDelete.hh"
    #include "G4LogicalBorderSurface.hh"
    #include "G4LogicalSkinSurface.hh"
    #include "G4OpticalSurface.hh"

    #include "G4SubtractionSolid.hh"
    #include "G4VSolid.hh"
    #include "G4UnionSolid.hh"
    #include "G4VPVParameterisation.hh"
    #include "G4PVParameterised.hh"


    #include "G4VisAttributes.hh"
    #include "G4Colour.hh"

    #include "G4PhysicalConstants.hh"
    #include "G4SystemOfUnits.hh"
    #include "globals.hh"

    #include "G4Navigator.hh"
    #include "G4VPhysicalVolume.hh"
    #include "G4TransportationManager.hh"

    #include <fstream>
    using namespace std;

    #include "G4NeutronHPManager.hh"
    #include <G4HadronicProcessStore.hh>
    
    //================================================================================

    G4DetectorConstruction::G4DetectorConstruction (G4double RIndex)
    : G4VUserDetectorConstruction(), fCheckOverlaps(true), Refr_Index(RIndex) {

        G4NeutronHPManager::GetInstance()->SetVerboseLevel(0);
        G4HadronicProcessStore::Instance()->SetVerbose(0);
        // World
        world_x = 100 * cm * 0.5;
        world_y = 100 * cm * 0.5;
        world_z = 200 * cm * 0.5;

        VLAr_x =  50 * cm * 0.5;
        VLAr_y =  50 * cm * 0.5;
        VLAr_z = 100 * cm * 0.5;
        
        Pixel_x =  50 * cm * 0.5;
        Pixel_y =  10 * cm * 0.5;
        Pixel_z =  10 * cm * 0.5;

    }

    //================================================================================

    G4DetectorConstruction::~G4DetectorConstruction (){;}


    //================================================================================

    class Full3DParameterisation : public G4VPVParameterisation {
    public:
        Full3DParameterisation(G4int nY, G4int nZ, G4double pitchY, G4double pitchZ)
            : fNY(nY), fNZ(nZ), fPitchY(pitchY), fPitchZ(pitchZ) {}

        void ComputeTransformation(G4int copyNo, G4VPhysicalVolume* physVol) const override {
            G4int iz = copyNo / fNY;
            G4int iy = copyNo % fNY;

            G4double y = (-fNY/2.0 + iy + 0.5) * fPitchY;
            // G4double z = (-fNZ/2.0 + iz + 0.5) * fPitchZ;
            G4double z = (-fNZ/2.0 + iz + 0.5) * fPitchZ;

            physVol->SetTranslation(G4ThreeVector(0., y, z));
        }

        void ComputeDimensions(G4Box& box, G4int, const G4VPhysicalVolume*) const override {
            // box is already defined — nothing to do here if you're using a fixed size
        }

    private:
        G4int fNY;
        G4int fNZ;
        G4double fPitchY;
        G4double fPitchZ;
    };

    //================================================================================

    G4VPhysicalVolume* G4DetectorConstruction::Construct () {

        // Define materials 
        DefineMaterials();
        
        // Define volumes
        return DefineVolumes();
        
    }

    //================================================================================

    void G4DetectorConstruction::DefineMaterials() {
        
        G4NistManager* nistManager = G4NistManager::Instance();

        nistManager->FindOrBuildMaterial("G4_lAr");
        nistManager->FindOrBuildMaterial("G4_Galactic");
        nistManager->FindOrBuildMaterial("G4_URANIUM_MONOCARBIDE");

        G4cout << *(G4Material::GetMaterialTable()) << G4endl;
        
    }

    //================================================================================

    G4VPhysicalVolume* G4DetectorConstruction::DefineVolumes() {
        // Get materials
       
        G4Material* Vacuo   = G4Material::GetMaterial("G4_Galactic");       
        G4Material* LAr     = G4Material::GetMaterial("G4_lAr");
        G4Material* HighSP     = G4Material::GetMaterial("G4_URANIUM_MONOCARBIDE");

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Construction %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    //====================== World ===========-===========

        G4Box*              WorldS          = new G4Box (MUNDO_NOME, world_x, world_y, world_z);

        G4LogicalVolume*    WorldLV         = new G4LogicalVolume (WorldS, Vacuo, MUNDO_NOME);

        G4VPhysicalVolume*  WorldPV         = new G4PVPlacement (0, G4ThreeVector (), WorldLV, MUNDO_NOME, 0, true, 0, fCheckOverlaps);

    //======================= Parametric volumes =======================

    // Mother volume for stack
    auto motherSolid = new G4Box("Mother", VLAr_x, VLAr_y, VLAr_z);
    auto motherLogic = new G4LogicalVolume(motherSolid, Vacuo, "Mother");
    new G4PVPlacement(0, {}, motherLogic, "Mother", WorldLV, false, 0);


    // Slice volume (same dimensions for all copies)
    auto sliceSolid = new G4Box("Slice", Pixel_x, Pixel_y, Pixel_z);
    auto sliceLogic = new G4LogicalVolume(sliceSolid, LAr, "Slice");


    G4int nY = VLAr_y/Pixel_y;
    G4int nZ = VLAr_z/Pixel_z;
    G4int nTotal = nY * nZ;

    new G4PVParameterised("Prisms",
                          sliceLogic,
                          motherLogic,
                          kUndefined,  // Not tied to a single axis
                          nTotal,
                          new Full3DParameterisation(nY, nZ, Pixel_y*2, Pixel_z*2));


    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% PROPERTIES %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    //====================== LAr ======================
      

    //     G4int    NUMENTRIES              = 1;

    //     G4double Scint_PP   [NUMENTRIES] = {9.8*eV};
    //     G4double Scint_FAST [NUMENTRIES] = {1};
    //     G4double Scint_SLOW [NUMENTRIES] = {1};

    //     G4double rindex     [NUMENTRIES] = {1.58};
    //     G4double rindexV    [NUMENTRIES] = {1.00};
    //     G4double absorption [NUMENTRIES] = {1.00*m};


    //     G4MaterialPropertiesTable* Scint_MPT = new G4MaterialPropertiesTable();

    //     Scint_MPT->AddConstProperty("SCINTILLATIONYIELD",   200/MeV); // <-----------------------------------------
    //     Scint_MPT->AddConstProperty("RESOLUTIONSCALE",      0.0001);
    //     Scint_MPT->AddConstProperty("FASTTIMECONSTANT",        3.*ns);
    //     Scint_MPT->AddConstProperty("SLOWTIMECONSTANT",     1200.*ns);
    //     Scint_MPT->AddConstProperty("YIELDRATIO",           0.25);

    //     Scint_MPT->AddProperty("FASTCOMPONENT", Scint_PP,   Scint_FAST,     NUMENTRIES);
    //     Scint_MPT->AddProperty("SLOWCOMPONENT", Scint_PP,   Scint_SLOW,     NUMENTRIES);
    //     Scint_MPT->AddProperty("RINDEX",        Scint_PP,   rindexV,        NUMENTRIES);
    //     Scint_MPT->AddProperty("ABSLENGTH",     Scint_PP,   absorption,     NUMENTRIES);

    //     LAr->SetMaterialPropertiesTable(Scint_MPT);



    // //==================== Stainless (Inox) =====================

    //     G4MaterialPropertiesTable* Inox_MPT = new G4MaterialPropertiesTable();

    //     G4double reflect[NUMENTRIES] = {0.3};
    //     G4double absorption_al[NUMENTRIES] = {0.0000000001*cm};

    //     Inox_MPT->AddProperty("REFLECTIVITY",   Scint_PP,   reflect,        NUMENTRIES);
    //     Inox_MPT->AddProperty("ABSLENGTH",      Scint_PP,   absorption_al,  NUMENTRIES);

    //     Inox->SetMaterialPropertiesTable(Inox_MPT);

    //     G4OpticalSurface *Inox_OptSurf = new G4OpticalSurface("Inox_OptSurf");
    //     Inox_OptSurf->SetModel(glisur);
    //     Inox_OptSurf->SetType(dielectric_metal);
    //     Inox_OptSurf->SetFinish(ground);
    //     Inox_OptSurf->SetPolish(0.9);

    //     Inox_OptSurf->SetMaterialPropertiesTable(Inox_MPT);

    //     new G4LogicalBorderSurface("Inox_BorderSurface1", WorldPV, Tube_place,  Inox_OptSurf);
    //     new G4LogicalBorderSurface("Inox_BorderSurface1", WorldPV, Cap_place,   Inox_OptSurf);
    

    // //==================== quartz =====================

    //     G4MaterialPropertiesTable* Quartz_MPT = new G4MaterialPropertiesTable();

    //     G4double RINDEXQ    [NUMENTRIES] = {1.1};
    //     G4double absorption_Q[NUMENTRIES] = {0.1*cm};

    //     Quartz_MPT->AddProperty("RINDEX",         Scint_PP,   RINDEXQ,        NUMENTRIES);
    //     Quartz_MPT->AddProperty("ABSLENGTH",      Scint_PP,   absorption_Q,   NUMENTRIES);

    //     Quartz->SetMaterialPropertiesTable(Quartz_MPT);

    // //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Visualization Attributes %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


    //     G4VisAttributes* cinza = new G4VisAttributes (G4Colour (0.5, 0.5, 0.5, 1));
    //     G4VisAttributes* azul = new G4VisAttributes (G4Colour (0.5, 0.5, 1, 0.8));
    //     cinza->SetForceSolid (true);
    //     azul->SetForceSolid (true);


    //     Cham_logico     ->SetVisAttributes(azul);
    //     PMT_logico      ->SetVisAttributes(cinza);
    //     Sampleholder_logico   ->SetVisAttributes(cinza);

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% End %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        return WorldPV;
        
    }