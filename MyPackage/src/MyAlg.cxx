// MyPackage includes
#include "MyAlg.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "xAODMuon/MuonContainer.h"

#include <stdio.h>


MyAlg::MyAlg(const std::string &name, ISvcLocator *pSvcLocator) : AthAlgorithm(name, pSvcLocator),
                                                                  m_jpsiContainerName("JpsiCandidates"),
                                                                  m_jpsiFinder("JpsiFinder/Whatever"),
                                                                  m_v0Tools("Trk::V0Tools"),
                                                                  m_pvRefitter("Analysis::PrimaryVertexRefitter"),
                                                                  m_grlTool("GoodRunsListSelectionTool/Whatever"),
                                                                  m_tdt("Trig::TrigDecisionTool/TrigDecisionTool"),
                                                                  m_muSel("CP::MuonSelectionTool/MuonSelectionTool")

{
    declareProperty("JpsiFinder", m_jpsiFinder, "The private JpsiFinder tool");
    declareProperty("V0Tools"   , m_v0Tools);
    declareProperty("PVRefitter", m_pvRefitter);

    declareProperty("JpsiCandidatesOutputName",m_jpsiContainerName="JpsiCandidates");
    declareProperty("GRLTool", m_grlTool, "The GRL Tool");
}


MyAlg::~MyAlg() {}


StatusCode MyAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << "...");
    CHECK( m_jpsiFinder.retrieve());
    CHECK( m_v0Tools.retrieve());
    CHECK( m_pvRefitter.retrieve());
    CHECK( m_grlTool.retrieve());
    CHECK( m_tdt.retrieve());
    CHECK( m_muSel.retrieve());

    ServiceHandle<ITHistSvc> histSvc("THistSvc",name());
    CHECK( histSvc.retrieve() );
    tree = new TTree("myTree","myTree");
    CHECK( histSvc->regTree("/JPsiOutput/myTree", tree) );

    eventCntr = 0;
    uncleanCntr = 0;

    initializeBranches();
    addBranches();

    return StatusCode::SUCCESS;
}

StatusCode MyAlg::finalize() {
    ATH_MSG_INFO("Finalizing " << name() << "...");
    ATH_MSG_INFO("===================");
    ATH_MSG_INFO("SUMMARY OF ANALYSIS");
    ATH_MSG_INFO("===================");
    ATH_MSG_INFO(" ");
    ATH_MSG_INFO("Total number of events analysed: " << eventCntr);
    ATH_MSG_INFO("Total number of events that did not pass Event Cleaning: " << uncleanCntr);
    ATH_MSG_INFO("Total number of jpsi candidates: " << jpsiCntr);

    return StatusCode::SUCCESS;
}

StatusCode MyAlg::execute() {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    // Crosscheck
//    const xAOD::MuonContainer* muons = 0;
//    CHECK( evtStore()->retrieve(muons,"Muons") );
//    ATH_MSG_INFO("Number of Muons" << muons->size());
//    for (const auto& muon : *muons){
//        if (!muon->inDetTrackParticleLink().isValid()) continue;
//        ATH_MSG_INFO("Muon pT:" << muon->pt() << "   " << "Associated Trk pT: " << muon->inDetTrackParticleLink().cachedElement()->pt());
//    }

    // Increment counter
    ++eventCntr;

    // Event Cleaning 
    const xAOD::EventInfo *evt = 0;
    if (!evtStore()->retrieve(evt, "EventInfo").isSuccess()) {
        ATH_MSG_ERROR("could not get EventInfo!");
        return StatusCode::FAILURE;
    }
    if (!(evt->eventType(xAOD::EventInfo::IS_SIMULATION))) {
        if (!m_grlTool->passRunLB(*evt)) {
            ATH_MSG_INFO("Event did not pass GRL!");
            ++uncleanCntr;
            return StatusCode::SUCCESS;
        }
        if ((evt->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error) ||
            (evt->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error) ||
            (evt->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error) ||
            (evt->eventFlags(xAOD::EventInfo::Core) & 0x40000)) {
            ++uncleanCntr;
            return StatusCode::SUCCESS;
        }
        if (!hasPV()){
            ++uncleanCntr;
            return StatusCode::SUCCESS;
        }
    }

    clearBranches();



    // Jpsi container and its auxilliary store
    xAOD::VertexContainer*    jpsiContainer = NULL;
    xAOD::VertexAuxContainer* jpsiAuxContainer = NULL;

    // call Jpsi finder
    ATH_MSG_DEBUG("Calling JpsiFinder ...");
    if( !m_jpsiFinder->performSearch(jpsiContainer, jpsiAuxContainer).isSuccess() ) {
        ATH_MSG_FATAL("Jpsi finder (" << m_jpsiFinder << ") failed.");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Successfully called JpsiFinder!");

    // Extracting information from the Jpsi candidates
    jpsiCntr += jpsiContainer->size(); // Count the Jpsis
    jpsiCand = jpsiContainer->size();
    ATH_MSG_INFO("Number of Jpsi Candidates:" << jpsiContainer->size());

    for ( xAOD::VertexContainer::const_iterator vxcItr = jpsiContainer->begin() ; vxcItr != jpsiContainer->end() ; vxcItr++ ) {
        const xAOD::Vertex* jpsiCandidate = (*vxcItr);
        //ATH_MSG_INFO("Cov. matrix " << jpsiCandidate->covariance().size());

        // refitted track parameters
        TLorentzVector refTrk1 = track4Momentum(jpsiCandidate, 0, m_muonMass);
        m_trkRefitPx1->push_back(refTrk1.Px());
        m_trkRefitPy1->push_back(refTrk1.Py());
        m_trkRefitPz1->push_back(refTrk1.Pz());
        m_trkRefitPt1->push_back(refTrk1.Pt());

        TLorentzVector refTrk2 = track4Momentum(jpsiCandidate, 1, m_muonMass);
        m_trkRefitPx2->push_back(refTrk2.Px());
        m_trkRefitPy2->push_back(refTrk2.Py());
        m_trkRefitPz2->push_back(refTrk2.Pz());
        m_trkRefitPt2->push_back(refTrk2.Pt());

        //vertex position
        m_vx->push_back(jpsiCandidate->x());
        m_vy->push_back(jpsiCandidate->y());
        m_vz->push_back(jpsiCandidate->z());

        TLorentzVector origTrk1 = origTrack4Momentum(jpsiCandidate, 0, m_muonMass);
        m_trkOrigCharge1->push_back(trackCharge(jpsiCandidate, 0));
        m_trkOrigPx1->push_back(origTrk1.Px());
        m_trkOrigPy1->push_back(origTrk1.Py());
        m_trkOrigPz1->push_back(origTrk1.Pz());
        m_trkOrigPt1->push_back(origTrk1.Pt());

        TLorentzVector origTrk2 = origTrack4Momentum(jpsiCandidate, 1, m_muonMass);
        m_trkOrigCharge2->push_back(trackCharge(jpsiCandidate, 1));
        m_trkOrigPx2->push_back(origTrk2.Px());
        m_trkOrigPy2->push_back(origTrk2.Py());
        m_trkOrigPz2->push_back(origTrk2.Pz());
        m_trkOrigPt2->push_back(origTrk2.Pt());

        // Calculate the invariant masses and their errors
        double orig_mass = (origTrk1+origTrk2).M();                      // mass from original tracks
        double mass      = (refTrk1+refTrk2).M();                        // mass from refitted tracks
//        double error     = invariantMassError(jpsiCandidate, std::vector<double>(2, m_muonMass));  // invariant mass error

        m_jpsiMass->push_back(mass);
//        m_jpsiMassError->push_back(error);
        m_jpsiChi2->push_back(jpsiCandidate->chiSquared());
        m_jpsiMassRec->push_back(orig_mass);
//        m_jpsiMassPullRec->push_back((mass-orig_mass)/error);
//        m_jpsiMassPullMC->push_back((mass-3096.88)/error);

    }


    // retrieve primary vertices
//    const xAOD::VertexContainer*    pvContainer = NULL;
//    CHECK( evtStore()->retrieve(pvContainer, "PrimaryVertices") );


    // create container for refitted primary vertices
//    xAOD::VertexContainer*    refPvContainer = new xAOD::VertexContainer;
//    xAOD::VertexAuxContainer* refPvAuxContainer = new xAOD::VertexAuxContainer;
//    refPvContainer->setStore(refPvAuxContainer);

    // loop over all Primary Vertices
//    int i = 0;
//    for (auto pv : *pvContainer){
//        if (pv->vertexType() == xAOD::VxType::PriVtx || pv->vertexType() == xAOD::VxType::PileUp) {
//            i++;
//            ATH_MSG_INFO("Original PV: (" << pv->x() << "," << pv->y() << "," << pv->z() << ")");
//            for (auto jpsi : *jpsiContainer) {
//                const xAOD::Vertex *refPV = m_pvRefitter->refitVertex(pv, jpsi, false);
//                if (refPV) {
//                    ATH_MSG_INFO("refitted PV: (" << refPV->x() << "," << refPV->y() << "," << refPV->z() << ")");
//                }
//            }
//        }
//    }
//    ATH_MSG_INFO("Number of PVs: " << pvContainer->size() << " Number of good PVs: " << i);

    // save in the StoreGate
    ATH_MSG_DEBUG("Recording to StoreGate: " << m_jpsiContainerName << " size:" <<jpsiContainer->size());

    if (!evtStore()->contains<xAOD::VertexContainer>(m_jpsiContainerName))
        CHECK(evtStore()->record(jpsiContainer, m_jpsiContainerName));

    if (!evtStore()->contains<xAOD::VertexAuxContainer>(m_jpsiContainerName+"Aux."))
        CHECK(evtStore()->record(jpsiAuxContainer, m_jpsiContainerName+"Aux."));

    tree->Fill();


    return StatusCode::SUCCESS;
}


bool MyAlg::hasPV() {

    const xAOD::VertexContainer *vertices(0);
    if (evtStore()->retrieve(vertices, "PrimaryVertices").isSuccess()) {
        for (const auto &vx : *vertices) {
            if (vx->vertexType() == xAOD::VxType::PriVtx) {
                ATH_MSG_DEBUG("PrimaryVertex found with z=" << vx->z());
                return vx;
            }
        }
    } else {
        ATH_MSG_WARNING("Failed to retrieve VertexContainer \"PrimaryVertices\", returning NULL");
    }
    return NULL;
}

void MyAlg::addBranches() {
    tree->Branch("jpsiCand", &jpsiCand);

    tree->Branch("jpsiMass", &m_jpsiMass);
    tree->Branch("jpsiMassError", &m_jpsiMassError);
    tree->Branch("jpsiMassRec", &m_jpsiMassRec);
    tree->Branch("jpsiMassPullRec", &m_jpsiMassPullRec);
    tree->Branch("jpsiMassPullMC", &m_jpsiMassPullMC);
    tree->Branch("jpsiChi2", &m_jpsiChi2);
    tree->Branch("jpsiMatched", &m_jpsiMatched);

    tree->Branch("trkRefitPx1",  &m_trkRefitPx1);
    tree->Branch("trkRefitPy1", &m_trkRefitPy1);
    tree->Branch("trkRefitPz1", &m_trkRefitPz1);
    tree->Branch("trkRefitPt1", &m_trkRefitPt1);
    tree->Branch("trkRefitPx2",  &m_trkRefitPx2);
    tree->Branch("trkRefitPy2", &m_trkRefitPy2);
    tree->Branch("trkRefitPz2", &m_trkRefitPz2);
    tree->Branch("trkRefitPt2", &m_trkRefitPt2);

    tree->Branch("vx",m_vx);
    tree->Branch("vy",m_vy);
    tree->Branch("vz",m_vz);

    tree->Branch("trkOrigCharge1",  &m_trkOrigCharge1);
    tree->Branch("trkOrigPx1",  &m_trkOrigPx1);
    tree->Branch("trkOrigPy1", &m_trkOrigPy1);
    tree->Branch("trkOrigPz1", &m_trkOrigPz1);
    tree->Branch("trkOrigPt1", &m_trkOrigPt1);
    tree->Branch("trkOrigCharge2",  &m_trkOrigCharge2);
    tree->Branch("trkOrigPx2",  &m_trkOrigPx2);
    tree->Branch("trkOrigPy2", &m_trkOrigPy2);
    tree->Branch("trkOrigPz2", &m_trkOrigPz2);
    tree->Branch("trkOrigPt2", &m_trkOrigPt2);
}

void MyAlg::initializeBranches() {
    jpsiCntr = 0;

    m_jpsiMass = new std::vector<double>;
    m_jpsiMassError = new std::vector<double>;
    m_jpsiMassRec = new std::vector<double>;
    m_jpsiMassPullRec = new std::vector<double>;
    m_jpsiMassPullMC = new std::vector<double>;
    m_jpsiChi2 = new std::vector<double>;
    m_jpsiMatched = new std::vector<int>;

    m_trkRefitPx1 = new std::vector<double>;
    m_trkRefitPy1 = new std::vector<double>;
    m_trkRefitPz1 = new std::vector<double>;
    m_trkRefitPt1 = new std::vector<double>;
    m_trkRefitPx2 = new std::vector<double>;
    m_trkRefitPy2 = new std::vector<double>;
    m_trkRefitPz2 = new std::vector<double>;
    m_trkRefitPt2 = new std::vector<double>;

    m_vx = new std::vector<double>;
    m_vy = new std::vector<double>;
    m_vz = new std::vector<double>;

    m_trkOrigCharge1 = new std::vector<double>;
    m_trkOrigPx1 = new std::vector<double>;
    m_trkOrigPy1 = new std::vector<double>;
    m_trkOrigPz1 = new std::vector<double>;
    m_trkOrigPt1 = new std::vector<double>;
    m_trkOrigCharge2 = new std::vector<double>;
    m_trkOrigPx2 = new std::vector<double>;
    m_trkOrigPy2 = new std::vector<double>;
    m_trkOrigPz2 = new std::vector<double>;
    m_trkOrigPt2 = new std::vector<double>;


}

void MyAlg::clearBranches() {
    jpsiCand = 0;

    m_jpsiMass->clear();
    m_jpsiMassError->clear();
    m_jpsiMassRec->clear();
    m_jpsiMassPullRec->clear();
    m_jpsiMassPullMC->clear();
    m_jpsiChi2->clear();
    m_jpsiMatched->clear();

    m_trkRefitPx1->clear();
    m_trkRefitPy1->clear();
    m_trkRefitPz1->clear();
    m_trkRefitPt1->clear();
    m_trkRefitPx2->clear();
    m_trkRefitPy2->clear();
    m_trkRefitPz2->clear();
    m_trkRefitPt2->clear();


    m_vx->clear();
    m_vy->clear();
    m_vz->clear();

    m_trkOrigCharge1->clear();
    m_trkOrigPx1->clear();
    m_trkOrigPy1->clear();
    m_trkOrigPz1->clear();
    m_trkOrigPt1->clear();
    m_trkOrigCharge2->clear();
    m_trkOrigPx2->clear();
    m_trkOrigPy2->clear();
    m_trkOrigPz2->clear();
    m_trkOrigPt2->clear();

}


int MyAlg::isMuonMatched(TLorentzVector& refTrk) {
    const xAOD::MuonContainer* muons = 0;
    CHECK( evtStore()->retrieve(muons,"CalibratedMuons") );
    for (const auto& muon : *muons){
        if (m_muSel->getQuality(*muon) <= xAOD::Muon::Medium && ParticleDR(*muon, refTrk) < 0.005)
            return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------------
// trackMomentum: returns refitted track momentum
// ---------------------------------------------------------------------------------

TVector3 MyAlg::trackMomentum(const xAOD::Vertex * vxCandidate, uint trkIndex) const
{
    float px = 0., py = 0., pz = 0.;
//  assert(vxCandidate!=0);
//  assert(i<vxCandidate->vxTrackAtVertex().size());

//  if (0 != vxCandidate) {
//    const Trk::TrackParameters* aPerigee = vxCandidate->vxTrackAtVertex()[trkIndex].perigeeAtVertex();
//    px = aPerigee->momentum()[Trk::px];
//    py = aPerigee->momentum()[Trk::py];
//    pz = aPerigee->momentum()[Trk::pz];
//  }

    static SG::AuxElement::Accessor< std::vector<float> > refTrackPxAcc("RefTrackPx");
    static SG::AuxElement::Accessor< std::vector<float> > refTrackPyAcc("RefTrackPy");
    static SG::AuxElement::Accessor< std::vector<float> > refTrackPzAcc("RefTrackPz");
    const std::vector<float>& refTrackPx = refTrackPxAcc(*vxCandidate);
    const std::vector<float>& refTrackPy = refTrackPyAcc(*vxCandidate);
    const std::vector<float>& refTrackPz = refTrackPzAcc(*vxCandidate);

    if(trkIndex < refTrackPx.size() && refTrackPx.size() == refTrackPy.size() && refTrackPz.size()) {
        px = refTrackPx[trkIndex];
        py = refTrackPy[trkIndex];
        pz = refTrackPz[trkIndex];
    }

    TVector3 mom(px,py,pz);
    return mom;
}

// ---------------------------------------------------------------------------------
// track4Momentum: returns refitted track 4-momentum given mass hypothesis
// ---------------------------------------------------------------------------------

TLorentzVector MyAlg::track4Momentum(const xAOD::Vertex * vxCandidate, int trkIndex, double mass) const
{
    TVector3 mom = trackMomentum(vxCandidate, trkIndex);
    TLorentzVector lorentz;
    lorentz.SetVectM(mom, mass);
    return lorentz;
}


// ---------------------------------------------------------------------------------
// origTrackMomentum: returns original track momentum
// ---------------------------------------------------------------------------------

TVector3 MyAlg::origTrackMomentum(const xAOD::Vertex * vxCandidate, int trkIndex) const
{
    TVector3 mom;
    const xAOD::TrackParticle* origTrack = vxCandidate->trackParticle(trkIndex);
    if(origTrack==NULL) {
        return mom;
    }

    mom.SetPtEtaPhi(origTrack->pt(), origTrack->eta(), origTrack->phi());
    return mom;
}

// ---------------------------------------------------------------------------------
// origTrack4Momentum: returns original track 4-momentum given mass hypothesis
// ---------------------------------------------------------------------------------

TLorentzVector MyAlg::origTrack4Momentum(const xAOD::Vertex * vxCandidate, int trkIndex, double mass) const
{
    TVector3 mom = origTrackMomentum(vxCandidate, trkIndex);
    TLorentzVector lorentz;
    lorentz.SetVectM(mom, mass);
    return lorentz;
}

// ---------------------------------------------------------------------------------
// trackCharge: returns charge of the i-th track
// ---------------------------------------------------------------------------------

double MyAlg::trackCharge(const xAOD::Vertex * vxCandidate, int i) const
{
    if(vxCandidate!=NULL && vxCandidate->trackParticle(i)!=NULL) {
        return vxCandidate->trackParticle(i)->charge();
    }

    return -999999;
}