from AthenaCommon.AthenaCommonFlags import athenaCommonFlags

# athenaCommonFlags.FilesInput = [ "/project/etp5/miholzbo/mc15/mc15_13TeV.424100.Pythia8B_A14_CTEQ6L1_Jpsimu4mu4.merge.AOD.e3735_s2608_s2183_r7772_r7676/AOD.08439866._000181.pool.root.1"]
athenaCommonFlags.FilesInput = [ "/afs/cern.ch/user/m/miholzbo/data16/data16_13TeV.00303304.physics_Main.merge.AOD.f716_m1620._lb1250._0004.1"]

from RecExConfig.RecFlags import rec

rec.doTrigger.set_Value_and_Lock(False) # leave false; nothing to do with trigger analysis



# Output log setting; this is for the framework in general
# You may over-ride this in your job options for your algorithm
rec.OutputLevel.set_Value_and_Lock(INFO);

# Write settings; keep all of these to false.
# Control the writing of your own n-tuple in the alg's job options
rec.doCBNT.set_Value_and_Lock(False)
rec.doWriteAOD.set_Value_and_Lock (False)
rec.doWriteTAG.set_Value_and_Lock (False)
rec.doHist.set_Value_and_Lock (False)

# These 2 lines are needed for the AODFix mechanism
rec.readRDO=False
rec.doESD=False

# main jobOption - must always be included
#include("RecJobTransforms/UseOracle.py") # DB access
include ("RecExCommon/RecExCommon_topOptions.py")
# Following 3 lines needed for TAG jobs (as is)
#svcMgr.EventSelector.RefName= "StreamAOD"
#svcMgr.EventSelector.CollectionType="ExplicitROOT"
#svcMgr.EventSelector.Query = ""


# include your algorithm job options here
from AthenaCommon.DetFlags import DetFlags
import MagFieldServices.SetupField
include( "JpsiUpsilonTools/configureServices.py" )
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
ExampleJpsiFinder = Analysis__JpsiFinder(name                        = "JpsiFinderName",
                                         OutputLevel                 = INFO,
                                         muAndMu                     = True,
                                         muAndTrack                  = False,
                                         TrackAndTrack               = False,
                                         assumeDiMuons               = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
                                         invMassUpper                = 100000.0,
                                         invMassLower                = 0.0,
                                         Chi2Cut                     = 200.,
                                         oppChargesOnly              = True,
                                         atLeastOneComb              = True,
                                         useCombinedMeasurement      = False, # Only takes effect if combOnly=True
                                         muonCollectionKey           = "Muons",
                                         TrackParticleCollection     = "InDetTrackParticles",
                                         V0VertexFitterTool          = TrkV0Fitter,             # V0 vertex fitter
                                         useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
                                         TrkVertexFitterTool         = TrkVKalVrtFitter,        # VKalVrt vertex fitter
                                         TrackSelectorTool           = InDetTrackSelectorTool,
                                         ConversionFinderHelperTool  = InDetConversionHelper,
                                         VertexPointEstimator        = VtxPointEstimator,
                                         useMCPCuts                  = False)
ToolSvc += ExampleJpsiFinder
ToolSvc += CfgMgr.GoodRunsListSelectionTool("GRLTool",GoodRunsListVec=["data16_13TeV.periodAllYear_DetStatus-v80-pro20-08_DQDefects-00-02-02_PHYS_StandardGRL_All_Good_25ns_TriggerMenu1e34only.xml"])

algSeq = CfgMgr.AthSequencer("AthAlgSeq")
algSeq += CfgMgr.MyAlg()
algSeq.MyAlg.JpsiFinder = ToolSvc.JpsiFinderName
algSeq.MyAlg.GRLTool = ToolSvc.GRLTool
# rec.UserAlgs.set_Value_and_Lock("RunJpsiExample.py")
from JpsiUpsilonTools.JpsiUpsilonToolsConf import JpsiExample
# JpsiExample retrieves original and fitted vertices and saves some variables to a file
algSeq += JpsiExample(outputNTupleName = "JpsiExample.root",
                      JpsiCandidates   = "JpsiCandidates")

# No stats printout
from GaudiCommonSvc.GaudiCommonSvcConf import ChronoStatSvc
chronoStatSvc = ChronoStatSvc()
chronoStatSvc.ChronoPrintOutTable = FALSE
chronoStatSvc.PrintUserTime       = FALSE
chronoStatSvc.StatPrintOutTable   = FALSE

theApp.EvtMax = 1000 # number of event to process

# Stops writing of monitoring ntuples (big files)
from PerfMonComps.PerfMonFlags import jobproperties as jp
jp.PerfMonFlags.doMonitoring = False
jp.PerfMonFlags.doFastMon = False

###

# from AthenaCommon.DetFlags import DetFlags
# import AthenaPoolCnvSvc.ReadAthenaPool
# import MagFieldServices.SetupField
#
#
# # from AthenaCommon.GlobalFlags import globalflags
# # globalflags.DetDescrVersion = "ATLAS-R2-2015-03-01-00"
# from AtlasGeoModel import SetGeometryVersion
# from AtlasGeoModel import GeoModelInit









# import AthenaPoolCnvSvc.ReadAthenaPool
# from AthenaCommon.DetFlags import DetFlags
# from AtlasGeoModel import SetGeometryVersion
# from AtlasGeoModel import GeoModelInit
#
# import MagFieldServices.SetupField
#
# # svcMgr.EventSelector.InputCollections = ["/project/etp5/miholzbo/mc15/mc15_13TeV.424100.Pythia8B_A14_CTEQ6L1_Jpsimu4mu4.merge.AOD.e3735_s2608_s2183_r7772_r7676/AOD.08439866._000181.pool.root.1"]
# svcMgr.EventSelector.InputCollections = ["/project/etp5/miholzbo/data16/data16_13TeV.00303304.physics_Main.merge.AOD.f716_m1620/data16_13TeV.00303304.physics_Main.merge.AOD.f716_m1620._lb1250._0004.1"]
#
#
#
# # ------------------------
# # SET UP FITTER
# # ------------------------
# include( "JpsiUpsilonTools/configureServices.py" )
#
# # ----------------------------------
# # User's analysis requirements here:
# # ----------------------------------
# import os
# # ToolSvc += CfgMgr.GoodRunsListSelectionTool("GRLTool",GoodRunsListVec=[os.path.expandvars()])
#
# from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
# ExampleJpsiFinder = Analysis__JpsiFinder(name                        = "JpsiFinderName",
#                                          OutputLevel                 = INFO,
#                                          muAndMu                     = True,
#                                          muAndTrack                  = False,
#                                          TrackAndTrack               = False,
#                                          assumeDiMuons               = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
#                                          invMassUpper                = 100000.0,
#                                          invMassLower                = 0.0,
#                                          Chi2Cut                     = 200.,
#                                          oppChargesOnly              = True,
#                                          atLeastOneComb              = True,
#                                          useCombinedMeasurement      = False, # Only takes effect if combOnly=True
#                                          muonCollectionKey           = "Muons",
#                                          TrackParticleCollection     = "InDetTrackParticles",
#                                          V0VertexFitterTool          = TrkV0Fitter,             # V0 vertex fitter
#                                          useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
#                                          TrkVertexFitterTool         = TrkVKalVrtFitter,        # VKalVrt vertex fitter
#                                          TrackSelectorTool           = InDetTrackSelectorTool,
#                                          ConversionFinderHelperTool  = InDetConversionHelper,
#                                          VertexPointEstimator        = VtxPointEstimator,
#                                          useMCPCuts                  = False)
# ToolSvc += ExampleJpsiFinder
#
#
# algSeq = CfgMgr.AthSequencer("AthAlgSeq")
# # MyAlg calls the JpsiFinder
# algSeq += CfgMgr.MyAlg()
# algSeq.MyAlg.JpsiFinder = ToolSvc.JpsiFinderName
# #algSeq.MyAlg.OutputLevel=VERBOSE
# # algSeq += CfgMgr.GRLSelectorAlg(Tool=ToolSvc.GRLTool)
#
# #-------------------------------------------------------------
# # User analysis steering algorithm
# #-------------------------------------------------------------
# from JpsiUpsilonTools.JpsiUpsilonToolsConf import JpsiExample
#
# # JpsiExample retrieves original and fitted vertices and saves some variables to a file
# algSeq += JpsiExample(outputNTupleName = "JpsiExample.root",
#                            JpsiCandidates   = "JpsiCandidates")

