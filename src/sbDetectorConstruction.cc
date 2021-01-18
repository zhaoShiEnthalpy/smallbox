#include "sbDetectorConstruction.hh"

sbDetectorConstruction* sbDetectorConstruction::instance_ = nullptr;

sbDetectorConstruction::sbDetectorConstruction() :
    nist(G4NistManager::Instance()),
    // Set world's size, name, and materials
    //
    world_radius_(1.1 * m),
    world_name_("world"),
    world_material_name_("G4_AIR"),
    // Set scintillators' size, name, and materials
    //
    scintillator_half_size_{ 5.0 * cm, 5.0 * cm, 0.5 * cm },
    scintillator_centre_distance_(1.0 * cm),
    scintillator_1_name_("scintillator_1"),
    scintillator_2_name_("scintillator_2"),
    sensitive_detector_name_("scintillator"),
    scintillator_material_name_("plastic_scintillator"),
    // Set aluminum foils' size, name, and materials
    //
    al_foil_thickness_(100 * um),
    al_foil_hole_half_width_(5 * mm),
    al_foil_scintillator_gap_(50 * um),
    al_foil_1_name_("al_foil_1"),
    al_foil_2_name_("al_foil_2"),
    al_foil_material_name_("G4_Al") {}

G4VPhysicalVolume* sbDetectorConstruction::Construct() {
    // Set if check overlaps
    //
    constexpr G4bool enable_check_overlaps = true;

#pragma region construct_world
    // world material
    //
    G4Material* world_material = nist->FindOrBuildMaterial(world_material_name_);

#if SB_ENABLE_OPTICAL_PHYSICS
    SetWorldOpticalProperties(world_material);
#endif

    // world construction
    //
    G4Sphere* solid_big_sphere = new G4Sphere(
        "big_sphere",
        0.0,
        world_radius_,
        0.0,
        360 * deg,
        0.0,
        360 * deg
    );
    G4Box* solid_big_box = new G4Box(
        "big_box",
        world_radius_,
        world_radius_,
        world_radius_
    );
    const G4ThreeVector big_sphere_transition(0.0, 0.0,
        -world_radius_ - (scintillator_centre_distance_ / 2.0 + 2.0 * scintillator_half_size_[2]) - 5.0 * cm);
    G4SubtractionSolid* solid_world = new G4SubtractionSolid(
        world_name_,
        solid_big_sphere,
        solid_big_box,
        nullptr,
        big_sphere_transition
    );
    G4LogicalVolume* logical_world = new G4LogicalVolume(
        solid_world,
        world_material,
        world_name_
    );

    G4VPhysicalVolume* physical_world = new G4PVPlacement(
        0,
        G4ThreeVector(),
        logical_world,
        world_name_,
        0,
        false,
        0,
        enable_check_overlaps
    );
#pragma endregion

#pragma region construct_scintillators
    // scintillator material
    //
    G4Material* scintillator_material = new G4Material(scintillator_material_name_, 1.032 * g / cm3, 2, kStateSolid);
    G4Element* element_H = new G4Element("Hydrogen", "H", 1.0, 1.008 * g / mole);
    G4Element* element_C = new G4Element("Carbon", "C", 6.0, 12.01 * g / mole);
    scintillator_material->AddElement(element_C, 9);
    scintillator_material->AddElement(element_H, 10);

#if SB_ENABLE_OPTICAL_PHYSICS
    SetScintillatorOpticalProperties(scintillator_material);
#endif

    // solid & logical scintillator construction
    //
    G4Box* solid_scintillator = new G4Box(
        "scintillator",
        scintillator_half_size_[0],
        scintillator_half_size_[1],
        scintillator_half_size_[2]
    );
    G4LogicalVolume* logical_scintillator = new G4LogicalVolume(
        solid_scintillator,
        scintillator_material,
        "scintillator"
    );
    SetScintillatorsAsSensitiveDetector("scintillator", sensitive_detector_name_);

    // scintillators' position
    //
    const G4double scintillator_1_z_position = scintillator_centre_distance_ / 2.0 + scintillator_half_size_[2];
    const G4ThreeVector scintillator_1_position(0.0 * cm, 0.0 * cm, scintillator_1_z_position);
    const G4ThreeVector scintillator_2_position(0.0 * cm, 0.0 * cm, -scintillator_1_z_position);

    // physical scintillator 1 construction
    //
    G4VPhysicalVolume* physical_scintillator_1 = new G4PVPlacement(
        0,
        scintillator_1_position,
        logical_scintillator,
        scintillator_1_name_,
        logical_world,
        false,
        0,
        enable_check_overlaps
    );

    // physical scintillator 2 construction
    //
    G4VPhysicalVolume* physical_scintillator_2 = new G4PVPlacement(
        0,
        scintillator_2_position,
        logical_scintillator,
        scintillator_2_name_,
        logical_world,
        false,
        0,
        enable_check_overlaps
    );
#pragma endregion

#pragma region construct_al_foils
    // aluminum foil material
    //
    G4Material* al_foil_material = nist->FindOrBuildMaterial(al_foil_material_name_);

#if SB_ENABLE_OPTICAL_PHYSICS
    // aluminum foil optical surface
    //
    G4OpticalSurface* al_foil_optical_surface = new G4OpticalSurface(
        "al_foil_optical_surface",
        unified,
        polished,
        dielectric_metal
    );
    SetAlFoilOpticalProperties(al_foil_optical_surface);
#endif

    // solid & logical aluminum foil construction
    //
    G4Box* solid_al_foil_and_scintillator = new G4Box(
        "al_foil_and_scintillator",
        scintillator_half_size_[0] + al_foil_scintillator_gap_ + al_foil_thickness_,
        scintillator_half_size_[1] + al_foil_scintillator_gap_ + al_foil_thickness_,
        scintillator_half_size_[2] + al_foil_scintillator_gap_ + al_foil_thickness_
    );
    G4Box* solid_volume_inside_al_foil = new G4Box(
        "al_foil_subtrahend",
        scintillator_half_size_[0] + al_foil_scintillator_gap_,
        scintillator_half_size_[1] + al_foil_scintillator_gap_,
        scintillator_half_size_[2] + al_foil_scintillator_gap_
    );
    G4SubtractionSolid* solid_al_foil_without_hole = new G4SubtractionSolid(
        "al_foil_without_hole",
        solid_al_foil_and_scintillator,
        solid_volume_inside_al_foil
    );
    G4Box* solid_hole = new G4Box(
        "al_foil_hole",
        al_foil_hole_half_width_,
        al_foil_hole_half_width_,
        0.5 * al_foil_thickness_
    );

    // aluminum foil 1 construction
    //
    G4SubtractionSolid* solid_al_foil_1 = new G4SubtractionSolid(
        al_foil_1_name_,
        solid_al_foil_without_hole,
        solid_hole,
        nullptr,
        G4ThreeVector(0, 0, scintillator_half_size_[2] + al_foil_scintillator_gap_ + 0.5 * al_foil_thickness_)
    );
    G4LogicalVolume* logical_al_foil_1 = new G4LogicalVolume(
        solid_al_foil_1,
        al_foil_material,
        al_foil_1_name_
    );
    G4VPhysicalVolume* physical_al_foil_1 = new G4PVPlacement(
        0,
        scintillator_1_position,
        logical_al_foil_1,
        al_foil_1_name_,
        logical_world,
        false,
        0,
        enable_check_overlaps
    );

    // aluminum foil 2 construction
    //
    G4SubtractionSolid* solid_al_foil_2 = new G4SubtractionSolid(
        al_foil_2_name_,
        solid_al_foil_without_hole,
        solid_hole,
        nullptr,
        G4ThreeVector(0, 0, -scintillator_half_size_[2] - al_foil_scintillator_gap_ - 0.5 * al_foil_thickness_)
    );
    G4LogicalVolume* logical_al_foil_2 = new G4LogicalVolume(
        solid_al_foil_2,
        al_foil_material,
        al_foil_2_name_
    );
    G4VPhysicalVolume* physical_al_foil_2 = new G4PVPlacement(
        0,
        scintillator_2_position,
        logical_al_foil_2,
        al_foil_2_name_,
        logical_world,
        false,
        0,
        enable_check_overlaps
    );

#if SB_ENABLE_OPTICAL_PHYSICS
    // aluminum foil 1/2 surface construction
    //
    G4LogicalSkinSurface* logical_al_foil_1_surface = new G4LogicalSkinSurface(
        al_foil_1_name_ + "_surface",
        logical_al_foil_1,
        al_foil_optical_surface
    );
    G4LogicalSkinSurface* logical_al_foil_2_surface = new G4LogicalSkinSurface(
        al_foil_2_name_ + "_surface",
        logical_al_foil_2,
        al_foil_optical_surface
    );
#endif
#pragma endregion

    return physical_world;
}

void sbDetectorConstruction::SetScintillatorsAsSensitiveDetector(const G4String& scintillator_name,
    const G4String& sensitive_detector_name) {
    sbTrackerSD* aTrackerSD = new sbTrackerSD(sensitive_detector_name, "TrackerHitsCollection");
    G4SDManager::GetSDMpointer()->AddNewDetector(aTrackerSD);
    SetSensitiveDetector(scintillator_name, aTrackerSD, true);
}

#if SB_ENABLE_OPTICAL_PHYSICS

void sbDetectorConstruction::SetWorldOpticalProperties(G4Material*& world_material) const {
    G4MaterialPropertiesTable* world_properties_table = new G4MaterialPropertiesTable();

    // Refraction index
    G4double refraction_photon_energy[2] = { 1.0 * eV, 20.0 * eV };
    G4double refraction_index[2] = { 1.0, 1.0 };
    world_properties_table->AddProperty("RINDEX", refraction_photon_energy, refraction_index, 2);

    // Set!
    world_material->SetMaterialPropertiesTable(world_properties_table);
}

void sbDetectorConstruction::SetScintillatorOpticalProperties(G4Material*& scintillator_material) const {
    G4MaterialPropertiesTable* scintillator_properties_table = new G4MaterialPropertiesTable();

    // Absorption length
    XYlist absorption_length("scintillator_absorbtion_length.csv", 502);
    scintillator_properties_table->AddProperty(
        "ABSLENGTH",
        absorption_length.px(),
        absorption_length.py(),
        absorption_length.Size()
    );
    // Refraction index
    XYlist refraction_index("scintillator_refraction_index.csv", 18);
    scintillator_properties_table->AddProperty(
        "RINDEX",
        refraction_index.px(),
        refraction_index.py(),
        refraction_index.Size()
    );
    // Reemission probablity
    XYlist reemission_probablity("scintillator_reemission_probablity.csv", 28);
    scintillator_properties_table->AddProperty(
        "REEMISSIONPROB",
        reemission_probablity.px(),
        reemission_probablity.py(),
        reemission_probablity.Size()
    );
    // Rayleigh scattering
    XYlist rayleigh_scattering_length("scintillator_rayleigh_scattering_length.csv", 11);
    scintillator_properties_table->AddProperty(
        "RAYLEIGH",
        rayleigh_scattering_length.px(),
        rayleigh_scattering_length.py(),
        rayleigh_scattering_length.Size()
    );
    // Fast light component
    XYlist fast_light_component("scintillator_fast_light_component.csv", 275);
    scintillator_properties_table->AddProperty(
        "FASTCOMPONENT",
        fast_light_component.px(),
        fast_light_component.py(),
        fast_light_component.Size()
    );
    // Slow light component
    scintillator_properties_table->AddProperty(
        "SLOWCOMPONENT",
        fast_light_component.px(),
        fast_light_component.py(),
        fast_light_component.Size()
    );
    // Scintillation yield
    scintillator_properties_table->AddConstProperty("SCINTILLATIONYIELD", 11522 / MeV);
    // Resolution scale
    scintillator_properties_table->AddConstProperty("RESOLUTIONSCALE", 1.0);
    // Fast time constant
    scintillator_properties_table->AddConstProperty("FASTTIMECONSTANT", 2.1 * ns);
    // Slow time constant
    scintillator_properties_table->AddConstProperty("SLOWTIMECONSTANT", 14.2 * ns);
    // Yield ratio
    scintillator_properties_table->AddConstProperty("YIELDRATIO", 0.799);
    // Alpha fast time constant
    scintillator_properties_table->AddConstProperty("AlphaFASTTIMECONSTANT", 1.0 * ns);
    // Alpha Slow time constant
    scintillator_properties_table->AddConstProperty("AlphaSLOWTIMECONSTANT", 35.0 * ns);
    // Alpha yield ratio
    scintillator_properties_table->AddConstProperty("AlphaYIELDRATIO", 0.65);
    // Neutron fast time constant
    scintillator_properties_table->AddConstProperty("NeutronFASTTIMECONSTANT", 1.0 * ns);
    // Neutron Slow time constant
    scintillator_properties_table->AddConstProperty("NeutronSLOWTIMECONSTANT", 34.0 * ns);
    // Neutron yield ratio
    scintillator_properties_table->AddConstProperty("NeutronYIELDRATIO", 0.65);
    // Reemission fast time constant
    scintillator_properties_table->AddConstProperty("ReemissionFASTTIMECONSTANT", 1.50 * ns);
    // Reemission Slow time constant
    scintillator_properties_table->AddConstProperty("ReemissionSLOWTIMECONSTANT", 1.50 * ns);
    // Reemission yield ratio
    scintillator_properties_table->AddConstProperty("ReemissionYIELDRATIO", 1.000);

    // Set!
    scintillator_material->SetMaterialPropertiesTable(scintillator_properties_table);

    // Birks constant
    scintillator_material->GetIonisation()->SetBirksConstant(0.15 * mm / MeV);
}

void sbDetectorConstruction::SetAlFoilOpticalProperties(G4OpticalSurface*& al_foil_optical_surface) const {
    G4MaterialPropertiesTable* al_foil_properties_table = new G4MaterialPropertiesTable();

    // Reflectivity
    G4double reflection_photon_energy[2] = { 1.0 * eV, 20.0 * eV };
#if SB_ENABLE_AL_FOIL_REFLECTION
    G4double reflectivity[2] = { 0.9, 0.9 };
#else
    G4double reflectivity[2] = { 0.0, 0.0 };
#endif
    al_foil_properties_table->AddProperty("REFLECTIVITY", reflection_photon_energy, reflectivity, 2);

    // Set!
    al_foil_optical_surface->SetMaterialPropertiesTable(al_foil_properties_table);
}

#endif

