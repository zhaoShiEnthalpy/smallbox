#include "sbPrimaryGeneratorAction.hh"

sbPrimaryGeneratorAction::sbPrimaryGeneratorAction() :
    G4VUserPrimaryGeneratorAction(),
    fpParticleGun(new G4ParticleGun(1)),
    fCosmicMuonProperties(
        CreateMapFromCSV<G4double>("./datafiles/cosmicMuonProperties.csv")
    ),
    fCosmicMuonZenithDistributionCDFinv(
        &fCosmicMuonProperties["thetaDistribution"][0],
        &fCosmicMuonProperties["theta"][0],
        fCosmicMuonProperties["theta"].size()
    ),
    fCosmicMuonEnergySpectrumCDFinv(
        &fCosmicMuonProperties["energySpectrum"][0],
        &fCosmicMuonProperties["energy"][0],
        fCosmicMuonProperties["energy"].size()
    ) {}

sbPrimaryGeneratorAction::~sbPrimaryGeneratorAction() {
    delete fpParticleGun;
}

void sbPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    if (G4UniformRand() > 0.545) {
        fpParticleGun->SetParticleDefinition(G4MuonMinus::Definition());
    } else {
        fpParticleGun->SetParticleDefinition(G4MuonPlus::Definition());
    }
    
    auto posAndDir = MuonPositionAndDirection();
    fpParticleGun->SetParticlePosition(posAndDir.first);
    fpParticleGun->SetParticleMomentumDirection(posAndDir.second);
    
    fpParticleGun->SetParticleEnergy(CosmicMuonEnergySpectrum());

    fpParticleGun->GeneratePrimaryVertex(anEvent);
}

