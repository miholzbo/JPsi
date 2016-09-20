#ifndef MYPACKAGE_MYALG_H
#define MYPACKAGE_MYALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h" //included under assumption you'll want to use some tools! Remove if you don't!
#include "JpsiUpsilonTools/JpsiFinder.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "GoodRunsLists/GoodRunsListSelectionTool.h"

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
    int eventCntr;
    int jpsiCntr;
    std::string m_jpsiContainerName; //!< Name of output container to store results

    ToolHandle<Analysis::JpsiFinder>            m_jpsiFinder;
    ToolHandle<Trk::V0Tools>                    m_v0Tools;
    ToolHandle<Analysis::PrimaryVertexRefitter> m_pvRefitter;
    ToolHandle<IGoodRunsListSelectionTool> m_grlTool;

};

#endif //> !MYPACKAGE_MYALG_H
