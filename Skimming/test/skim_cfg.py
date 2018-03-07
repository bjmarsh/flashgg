import FWCore.ParameterSet.Config as cms

from flashgg.MicroAOD.flashggJets_cfi import flashggBTag
from flashgg.Taggers.flashggTags_cff import flashggTTHHadronicTag, flashggTTHLeptonicTag, bDiscriminator80XReReco

process = cms.Process("skim")

## MessageLogger
process.load("FWCore.MessageLogger.MessageLogger_cfi")

isMC = False

process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag
if isMC:
    process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc')
else:
    process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data')

process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring("PUTFILENAMEHERE")
                            # fileNames = cms.untracked.vstring("file:../myMicroAODOutputFile_99.root")
)

print process.source.fileNames

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1 ) )
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.skimFilter = cms.EDFilter("SkimFilter",
                                  verbose = cms.bool(False),
                                  inputDiPhotons = cms.InputTag("flashggPreselectedDiPhotons"),
                                  inputDiPhotonMVA = cms.InputTag("flashggDiPhotonMVA"),
                                  inputJets = cms.InputTag("flashggFinalJets"),
                                  inputElectrons = cms.InputTag("flashggSelectedElectrons"),
                                  inputMuons = cms.InputTag("flashggSelectedMuons"),
                                  inputVertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
                                  rhoTag = cms.InputTag('fixedGridRhoFastjetAll'),
                                  btagDiscName = cms.string(flashggBTag),

                                  acceptIfNLeptons = cms.int32(2),
                                  dRLeadPhoLepCut = flashggTTHLeptonicTag.deltaRMuonPhoThreshold,
                                  dRSubLeadPhoLepCut = flashggTTHLeptonicTag.deltaRMuonPhoThreshold,
                                  leptonPtThreshold = flashggTTHLeptonicTag.leptonPtThreshold,
                                  muonEtaThreshold = flashggTTHLeptonicTag.muonEtaThreshold,
                                  muPFIsoSumRelThreshold = flashggTTHLeptonicTag.muPFIsoSumRelThreshold,
                                  muMiniIsoSumRelThreshold = flashggTTHLeptonicTag.muMiniIsoSumRelThreshold,
                                  electronEtaThresholds = flashggTTHLeptonicTag.electronEtaThresholds,
                                  useStdLeptonID = flashggTTHLeptonicTag.useStdLeptonID,
                                  useElectronMVARecipe = flashggTTHLeptonicTag.useElectronMVARecipe,
                                  useElectronLooseID = flashggTTHLeptonicTag.useElectronLooseID,

                                  diphotonMVAcut = cms.double(-999.0),
                                  diphotonLeadPtOverMassCut = cms.double(0.0),
                                  diphotonSubLeadPtOverMassCut = cms.double(0.0),

                                  jetPtThresh = cms.double(25.0),
                                  jetEtaThresh = cms.double(2.4),
                                  btagDiscThresh = cms.double(bDiscriminator80XReReco[0]), # loose
                                  nJetCut = cms.int32(2),
                                  nBJetCut = cms.int32(0),
                                  dRLeadPhoJetCut = flashggTTHHadronicTag.dRJetPhoLeadCut,
                                  dRSubLeadPhoJetCut = flashggTTHHadronicTag.dRJetPhoSubleadCut
)

process.load("flashgg.Taggers.flashggDiPhotonMVA_cfi")
process.load("flashgg.Taggers.flashggPreselectedDiPhotons_cfi")
process.load("flashgg.Taggers.flashggUpdatedIdMVADiPhotons_cfi")

process.p = cms.Path(process.flashggUpdatedIdMVADiPhotons * 
                     process.flashggPreselectedDiPhotons * 
                     process.flashggDiPhotonMVA *
                     process.skimFilter
)

process.out = cms.OutputModule("PoolOutputModule",
                               fileName = cms.untracked.string("test_skim.root"),
                               outputCommands = cms.untracked.vstring("keep *","drop *_flashggDiPhotonMVA_*_*","drop *_flashggUpdatedIdMVADiPhotons_*_*","drop *_flashggPreselectedDiPhotons_*_*",),
                               SelectEvents = cms.untracked.PSet( SelectEvents = cms.vstring("p") )
)

process.outpath = cms.EndPath(process.out)
