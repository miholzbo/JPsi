#ifndef MYPACKAGE_MYALG_H
#define MYPACKAGE_MYALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "JpsiUpsilonTools/JpsiFinder.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "GoodRunsLists/GoodRunsListSelectionTool.h"

#include "TTree.h"
#include "TLorentzVector.h"
#include "TVector3.h"

#include <vector>

namespace Trk {
    class V0Tools;
}


class MyAlg : public ::AthAlgorithm {
public:
    MyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    virtual ~MyAlg();

    virtual StatusCode initialize();

    virtual StatusCode execute();

    virtual StatusCode finalize();

private:
    std::string m_jpsiContainerName; //!< Name of output container to store results
    double m_muonMass;


    TTree* tree;

    ToolHandle<Analysis::JpsiFinder>            m_jpsiFinder;
    ToolHandle<Trk::V0Tools>                    m_v0Tools;
    ToolHandle<Analysis::PrimaryVertexRefitter> m_pvRefitter;
    ToolHandle<IGoodRunsListSelectionTool> m_grlTool;


    void addBranches(void);
    void initializeBranches(void);
    void clearBranches(void);

    bool hasPV();

    TVector3       trackMomentum(const xAOD::Vertex * vxCandidate, uint trkIndex) const;
    TLorentzVector track4Momentum(const xAOD::Vertex * vxCandidate, int trkIndex, double mass) const;
    TVector3       origTrackMomentum(const xAOD::Vertex * vxCandidate, int trkIndex) const;
    TLorentzVector origTrack4Momentum(const xAOD::Vertex * vxCandidate, int trkIndex, double mass) const;
    double         trackCharge(const xAOD::Vertex * vxCandidate, int i) const;

//    double         invariantMassError(const xAOD::Vertex* vxCandidate, std::vector<double> masses) const;
//    double         massErrorVKalVrt(const xAOD::Vertex * vxCandidate, std::vector<double> masses) const;

    // global counters
    int eventCntr;
    int uncleanCntr;
    int jpsiCntr;

    // event variables
    int jpsiCand;
    std::vector<double> * m_jpsiMass;
    std::vector<double> * m_jpsiMassError;
    std::vector<double> * m_jpsiMassRec;
    std::vector<double> * m_jpsiMassPullRec;
    std::vector<double> * m_jpsiMassPullMC;
    std::vector<double> * m_jpsiChi2;

    std::vector<double> * m_trkRefitPx1;
    std::vector<double> * m_trkRefitPy1;
    std::vector<double> * m_trkRefitPz1;
    std::vector<double> * m_trkRefitPt1;
    std::vector<double> * m_trkRefitPx2;
    std::vector<double> * m_trkRefitPy2;
    std::vector<double> * m_trkRefitPz2;
    std::vector<double> * m_trkRefitPt2;

    std::vector<double> * m_vx;
    std::vector<double> * m_vy;
    std::vector<double> * m_vz;

    std::vector<double> * m_trkOrigCharge1;
    std::vector<double> * m_trkOrigPx1;
    std::vector<double> * m_trkOrigPy1;
    std::vector<double> * m_trkOrigPz1;
    std::vector<double> * m_trkOrigPt1;
    std::vector<double> * m_trkOrigCharge2;
    std::vector<double> * m_trkOrigPx2;
    std::vector<double> * m_trkOrigPy2;
    std::vector<double> * m_trkOrigPz2;
    std::vector<double> * m_trkOrigPt2;
};

#endif //> !MYPACKAGE_MYALG_H
