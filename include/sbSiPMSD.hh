#ifndef SB_SIPM_SD_H
#define SB_SIPM_SD_H 1

#include "G4VSensitiveDetector.hh"
#include "G4SDManager.hh"
#include "G4OpticalPhoton.hh"
#include "g4analysis.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

#include "sbGlobal.hh"
#include "sbSiPMHit.hh"

class sbSiPMSD : public G4VSensitiveDetector {
private:
    std::pair<sbSiPMHitsCollection*, sbSiPMHitsCollection*> fSiPMPhotonHC;

    G4ToolsAnalysisManager* fAnalysisManager;

public:
    sbSiPMSD(const G4String& SiPMSDName);
    virtual ~sbSiPMSD();

    virtual void Initialize(G4HCofThisEvent* eventHitCollection);
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory*);
    virtual void EndOfEvent(G4HCofThisEvent*);

    static std::mutex fMutex;
    static G4int fHitEventCount;

private:
    inline G4double SiPMSinglePhotoelectricResponse(const G4double& elapsedTimeAfterHit) const;
    void FillNtuple() const;

    inline static bool compareHit(sbSiPMHit* lhs, sbSiPMHit* rhs) { return lhs->GetTime() < rhs->GetTime(); }
};

inline G4double sbSiPMSD::SiPMSinglePhotoelectricResponse(const G4double& elapsedTimeAfterHit) const {
    return exp(-elapsedTimeAfterHit);
}

#endif

