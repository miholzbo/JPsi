// MyPackage includes
#include "MyAlg.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "xAODEventInfo/EventInfo.h"
#include <stdio.h>

MyAlg::MyAlg(const std::string &name, ISvcLocator *pSvcLocator) : AthAlgorithm(name, pSvcLocator),
                                                                  m_jpsiContainerName("JpsiCandidates"),
                                                                  m_jpsiFinder("JpsiFinder/Whatever"),
                                                                  m_v0Tools("Trk::V0Tools"),
                                                                  m_pvRefitter("Analysis::PrimaryVertexRefitter"),
                                                                  m_grlTool("GoodRunsListSelectionTool/Whatever")

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
    CHECK(m_jpsiFinder.retrieve());
    CHECK( m_v0Tools.retrieve());
    CHECK( m_pvRefitter.retrieve());
    CHECK( m_grlTool.retrieve());

    eventCntr = 0;
    jpsiCntr = 0;

    return StatusCode::SUCCESS;
}

StatusCode MyAlg::finalize() {
    ATH_MSG_INFO("Finalizing " << name() << "...");
    ATH_MSG_INFO("===================");
    ATH_MSG_INFO("SUMMARY OF ANALYSIS");
    ATH_MSG_INFO("===================");
    ATH_MSG_INFO(" ");
    ATH_MSG_INFO("Total number of events analysed: " << eventCntr);
    ATH_MSG_INFO("Total number of jpsi candidates: " << jpsiCntr);

    return StatusCode::SUCCESS;
}

StatusCode MyAlg::execute() {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    // Increment counter
    ++eventCntr;

    // Event Cleaning 
    const xAOD::EventInfo* evt = 0;
    if (!evtStore()->retrieve(evt, "EventInfo").isSuccess() || 0==evt) {
	ATH_MSG_ERROR("could not get EventInfo!");
	return StatusCode::FAILURE;
    }
    if (!(evt->eventType(xAOD::EventInfo::IS_SIMULATION))){
    	if (!m_grlTool->passRunLB(*evt)){
		ATH_MSG_INFO("Event did not pass GRL!");
	//	return StatusCode::SUCCESS;
    	}
	if ((evt->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error ) ||
            (evt->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error ) ||
            (evt->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error ) ||
            (evt->eventFlags(xAOD::EventInfo::Core) & 0x40000 )) {
	//	return StatusCode::SUCCESS;
        }	
    }

    // Jpsi container and its auxilliary store
    xAOD::VertexContainer*    jpsiContainer = NULL;
    xAOD::VertexAuxContainer* jpsiAuxContainer = NULL;

    // call Jpsi finder
    if( !m_jpsiFinder->performSearch(jpsiContainer, jpsiAuxContainer).isSuccess() ) {
        ATH_MSG_FATAL("Jpsi finder (" << m_jpsiFinder << ") failed.");
        return StatusCode::FAILURE;
    }

    // Extracting information from the Jpsi candidates
    jpsiCntr += jpsiContainer->size(); // Count the Jpsis
    ATH_MSG_INFO("Number of Jpsi Candidates:" << jpsiContainer->size());

    // retrieve primary vertices
    const xAOD::VertexContainer*    pvContainer = NULL;
    CHECK( evtStore()->retrieve(pvContainer, "PrimaryVertices") );

    // create container for refitted primary vertices
    xAOD::VertexContainer*    refPvContainer = new xAOD::VertexContainer;
    xAOD::VertexAuxContainer* refPvAuxContainer = new xAOD::VertexAuxContainer;
    refPvContainer->setStore(refPvAuxContainer);

    // loop over all Primary Vertices
    int i = 0;
    for (auto pv : *pvContainer){
        if (pv->vertexType() == xAOD::VxType::PriVtx || pv->vertexType() == xAOD::VxType::PileUp) {
            i++;
            ATH_MSG_INFO("Original PV: (" << pv->x() << "," << pv->y() << "," << pv->z() << ")");
            for (auto jpsi : *jpsiContainer) {
                const xAOD::Vertex *refPV = m_pvRefitter->refitVertex(pv, jpsi, false);
                if (refPV) {
                    ATH_MSG_INFO("refitted PV: (" << refPV->x() << "," << refPV->y() << "," << refPV->z() << ")");
                }
            }
        }
    }
    ATH_MSG_INFO("Number of PVs: " << pvContainer->size() << " Number of good PVs: " << i);

    // save in the StoreGate
    ATH_MSG_DEBUG("Recording to StoreGate: " << m_jpsiContainerName << " size:" <<jpsiContainer->size());

    if (!evtStore()->contains<xAOD::VertexContainer>(m_jpsiContainerName))
        CHECK(evtStore()->record(jpsiContainer, m_jpsiContainerName));

    if (!evtStore()->contains<xAOD::VertexAuxContainer>(m_jpsiContainerName+"Aux."))
        CHECK(evtStore()->record(jpsiAuxContainer, m_jpsiContainerName+"Aux."));




    return StatusCode::SUCCESS;
}


