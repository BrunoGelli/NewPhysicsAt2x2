void EventDisplay() {
    // Open file and get trees
    TFile *f = new TFile("OutPut.root");
    if (!f || f->IsZombie()) {
        std::cerr << "Failed to open OutPut.root" << std::endl;
        return;
    }

    // Get data tree and config tree
    TTree *t = (TTree*)f->Get("ntuple");
    TTree *config = (TTree*)f->Get("config");
    if (!t || !config) {
        std::cerr << "Missing ntuple or config tree!" << std::endl;
        return;
    }

    // Read detector configuration
    int sizeY, sizeZ;
    double pixelSizeY, pixelSizeZ;
    config->SetBranchAddress("sizeY", &sizeY);
    config->SetBranchAddress("sizeZ", &sizeZ);
    config->SetBranchAddress("pixelSizeY", &pixelSizeY);
    config->SetBranchAddress("pixelSizeZ", &pixelSizeZ);
    config->GetEntry(0); // read the only entry

    // Compute histogram bounds and binning
    double yMin = -sizeY / 2.0;
    double yMax =  sizeY / 2.0;
    double zMin = -sizeZ / 2.0;
    double zMax =  sizeZ / 2.0;
    int yBins = sizeY / pixelSizeY;
    int zBins = sizeZ / pixelSizeZ;

    // Create histogram
    TH2D *h = new TH2D("h", "Energy Deposition YZ;Z [cm];Y [cm];Edep [MeV]",
                       zBins, zMin, zMax,
                       yBins, yMin, yMax);

    // Fill histogram with energy deposition
    t->Draw("dy:dz>>h", "energy", "COLZ");

    gPad->SetLogz(); // Optional
}