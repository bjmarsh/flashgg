import FWCore.ParameterSet.Config as cms

from flashgg.MicroAOD.flashggJets_cfi import flashggBTag

process = cms.Process("skim")

## MessageLogger
process.load("FWCore.MessageLogger.MessageLogger_cfi")

process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring("file:../myMicroAODOutputFile_99.root")
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

process.skimFilter = cms.EDFilter("SkimFilter",
                                  inputDiPhotons = cms.InputTag("flashggPreselectedDiPhotons"),
                                  inputDiPhotonMVA = cms.InputTag("flashggDiPhotonMVA"),
                                  inputJets = cms.InputTag("flashggFinalJets"),
                                  btagDiscName = cms.string(flashggBTag),

                                  diphotonMVAcut = cms.double(-999.0),
                                  diphotonLeadPtOverMassCut = cms.double(0.0),
                                  diphotonSubLeadPtOverMassCut = cms.double(0.0),

                                  jetPtThresh = cms.double(25.0),
                                  jetEtaThresh = cms.double(2.4),
                                  btagDiscThresh = cms.double(0.5426),
                                  nJetCut = cms.int32(2),
                                  nBJetCut = cms.int32(1),
                                  dRLeadPhoJetCut = cms.double(0.4),
                                  dRSubLeadPhoJetCut = cms.double(0.4),
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
