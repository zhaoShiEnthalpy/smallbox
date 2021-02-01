#include "sbScintillatorSD.hh"

sbScintillatorSD::sbScintillatorSD(const G4String& scintillatorSDName) :
    G4VSensitiveDetector(scintillatorSDName),
    fMuonHitsCollection(nullptr) {
    collectionName.push_back("muon_hits_collection");
}

sbScintillatorSD::~sbScintillatorSD() {}

void sbScintillatorSD::Initialize(G4HCofThisEvent* hitCollectionOfThisEvent) {
    fMuonHitsCollection = new sbScintillatorHitsCollection(SensitiveDetectorName, collectionName[0]);
    hitCollectionOfThisEvent->AddHitsCollection(GetCollectionID(0), fMuonHitsCollection);
}

G4bool sbScintillatorSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
    auto presentParticle = step->GetTrack()->GetParticleDefinition();
    if (presentParticle != G4MuonPlus::Definition() &&
        presentParticle != G4MuonMinus::Definition()) {
        return false;
    }
    // Present step point.
    auto preStepPoint = step->GetPreStepPoint();
    // A new hit.
    auto hit = new sbScintillatorHit(preStepPoint->GetPhysicalVolume()->GetName());
    hit->SetPostion(preStepPoint->GetPosition());
    hit->SetTime(preStepPoint->GetGlobalTime());
    fMuonHitsCollection->insert(hit);
    if (presentParticle == G4MuonPlus::Definition()) {
        std::cout << "mu+ hit scintillator " << hit->GetScintillatorID() << "!" << G4endl;
    } else {
        std::cout << "mu- hit scintillator " << hit->GetScintillatorID() << "!" << G4endl;
    }
    return true;
}

void sbScintillatorSD::EndOfEvent(G4HCofThisEvent*) {}

