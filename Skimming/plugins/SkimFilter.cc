// -*- C++ -*-
//
// Package:    flashgg/SkimFilter
// Class:      SkimFilter
// 
/**\class SkimFilter SkimFilter.cc flashgg/Skimming/plugins/SkimFilter.cc

*/
//
// Original Author:  bmarsh@umail.ucsb.edu 6-10-2015
//         Created:  Thu, 01 Mar 2018 18:14:31 GMT
//
//


// system include files
#include <memory>
#include <iostream> 

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDMException.h"

#include "DataFormats/Common/interface/Handle.h"
#include "flashgg/DataFormats/interface/Jet.h"
#include "flashgg/DataFormats/interface/Electron.h"
#include "flashgg/DataFormats/interface/Muon.h"
#include "flashgg/DataFormats/interface/DiPhotonCandidate.h"
#include "flashgg/DataFormats/interface/DiPhotonMVAResult.h"

#include "flashgg/Taggers/interface/LeptonSelection.h"

#include "DataFormats/Math/interface/deltaR.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"



//
// class declaration
//

class SkimFilter : public edm::stream::EDFilter<> {
  public:
    explicit SkimFilter(const edm::ParameterSet&);
    ~SkimFilter();

  private:
    virtual bool filter(edm::Event&, const edm::EventSetup&) override;
    bool CheckJetCollection(uint jetCollectionIdx, const std::vector< std::vector< flashgg::Jet> >* jets, const flashgg::DiPhotonCandidate& diphotons);

      // ----------member data ---------------------------
    edm::EDGetTokenT<std::vector< std::vector< flashgg::Jet> > > jetToken_;
    edm::EDGetTokenT< std::vector< flashgg::DiPhotonCandidate> > diphoToken_;
    edm::EDGetTokenT< std::vector< flashgg::DiPhotonMVAResult> > diphoMVAToken_;
    edm::EDGetTokenT< edm::View< flashgg::Electron> > electronToken_;
    edm::EDGetTokenT< edm::View< flashgg::Muon> > muonToken_;
    edm::EDGetTokenT< edm::View< reco::Vertex> > vertexToken_;
    edm::EDGetTokenT<double> rhoToken_;

    bool verbose_;

    string btagDiscName_;

    int acceptIfNLeptons_;
    double dRLeadPhoLepCut_;
    double dRSubLeadPhoLepCut_;
    double leptonPtThreshold_;
    double muonEtaThreshold_;
    double muPFIsoSumRelThreshold_;
    double muMiniIsoSumRelThreshold_;
    vector<double>  electronEtaThresholds_;
    bool useStdLeptonID_;
    bool useElectronMVARecipe_;
    bool useElectronLooseID_;

    double jetPtThresh_;
    double jetEtaThresh_;
    double btagDiscThresh_;

    double diphotonMVAcut_;
    double diphotonLeadPtOverMassCut_;
    double diphotonSubLeadPtOverMassCut_;

    int nJetCut_;
    int nBJetCut_;

    double dRLeadPhoJetCut_;
    double dRSubLeadPhoJetCut_;
};

//
// constructors and destructor
//
SkimFilter::SkimFilter(const edm::ParameterSet& iConfig) :
    jetToken_( consumes< std::vector< std::vector< flashgg::Jet> > >(iConfig.getParameter<edm::InputTag>("inputJets"))),
    diphoToken_( consumes< std::vector< flashgg::DiPhotonCandidate> >(iConfig.getParameter<edm::InputTag>("inputDiPhotons"))),
    diphoMVAToken_( consumes< std::vector< flashgg::DiPhotonMVAResult> >(iConfig.getParameter<edm::InputTag>("inputDiPhotonMVA"))),
    electronToken_( consumes< edm::View< flashgg::Electron> >(iConfig.getParameter<edm::InputTag>("inputElectrons"))),
    muonToken_( consumes< edm::View< flashgg::Muon> >(iConfig.getParameter<edm::InputTag>("inputMuons"))),
    vertexToken_( consumes< edm::View< reco::Vertex> >(iConfig.getParameter<edm::InputTag>("inputVertices"))),
    rhoToken_( consumes<double>(iConfig.getParameter<edm::InputTag>("rhoTag")))
{
    verbose_ = iConfig.getParameter<bool>("verbose");

    btagDiscName_ = iConfig.getParameter<string>("btagDiscName");

    acceptIfNLeptons_ = iConfig.getParameter<int>("acceptIfNLeptons");
    dRLeadPhoLepCut_ = iConfig.getParameter<double>("dRLeadPhoLepCut");    
    dRSubLeadPhoLepCut_ = iConfig.getParameter<double>("dRSubLeadPhoLepCut");    
    muPFIsoSumRelThreshold_ = iConfig.getParameter<double>( "muPFIsoSumRelThreshold");
    muMiniIsoSumRelThreshold_ = iConfig.getParameter<double>( "muMiniIsoSumRelThreshold");
    electronEtaThresholds_ = iConfig.getParameter<vector<double > >( "electronEtaThresholds");
    useStdLeptonID_ = iConfig.getParameter<bool>("useStdLeptonID");
    useElectronMVARecipe_ = iConfig.getParameter<bool>("useElectronMVARecipe");
    useElectronLooseID_ = iConfig.getParameter<bool>("useElectronLooseID");

    jetPtThresh_ = iConfig.getParameter<double>("jetPtThresh");
    jetEtaThresh_ = iConfig.getParameter<double>("jetEtaThresh");
    btagDiscThresh_ = iConfig.getParameter<double>("btagDiscThresh");

    diphotonMVAcut_ = iConfig.getParameter<double>("diphotonMVAcut");
    diphotonLeadPtOverMassCut_ = iConfig.getParameter<double>("diphotonLeadPtOverMassCut");
    diphotonSubLeadPtOverMassCut_ = iConfig.getParameter<double>("diphotonSubLeadPtOverMassCut");

    nJetCut_ = iConfig.getParameter<int>("nJetCut");
    nBJetCut_ = iConfig.getParameter<int>("nBJetCut");

    dRLeadPhoJetCut_ = iConfig.getParameter<double>("dRLeadPhoJetCut");    
    dRSubLeadPhoJetCut_ = iConfig.getParameter<double>("dRSubLeadPhoJetCut");    
}


SkimFilter::~SkimFilter()
{
}

//
// member functions
//

// ------------ method called on each new Event  ------------
bool
SkimFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

    edm::Handle< std::vector< std::vector< flashgg::Jet> > > jets_h;
    iEvent.getByToken(jetToken_, jets_h);
    const std::vector< std::vector< flashgg::Jet> >* jets = jets_h.product();
    
    edm::Handle< std::vector< flashgg::DiPhotonCandidate> > diphos_h;
    iEvent.getByToken(diphoToken_, diphos_h);
    const std::vector< flashgg::DiPhotonCandidate >* diphotons = diphos_h.product();

    edm::Handle< std::vector< flashgg::DiPhotonMVAResult> > diphomva_h;
    iEvent.getByToken(diphoMVAToken_, diphomva_h);
    const std::vector< flashgg::DiPhotonMVAResult >* diphotonMVAs = diphomva_h.product();

    edm::Handle< edm::View<flashgg::Electron> > electrons_h;
    iEvent.getByToken(electronToken_, electrons_h);

    edm::Handle< edm::View<flashgg::Muon> > muons_h;
    iEvent.getByToken(muonToken_, muons_h);

    edm::Handle< edm::View< reco::Vertex> > vertices_h;
    iEvent.getByToken(vertexToken_, vertices_h);

    edm::Handle<double>  rho_h;
    iEvent.getByToken(rhoToken_,rho_h);
    double rho    = *rho_h;

    // select loose leptons
    std::vector<edm::Ptr<flashgg::Muon> > goodMuons;
    if( !useStdLeptonID_) {
        goodMuons = selectAllMuonsSum16( muons_h->ptrs(), vertices_h->ptrs(), muonEtaThreshold_ , 
                                         leptonPtThreshold_, muMiniIsoSumRelThreshold_ );
    } else {
        goodMuons = selectAllMuons( muons_h->ptrs(), vertices_h->ptrs(), muonEtaThreshold_ , 
                                    leptonPtThreshold_, muPFIsoSumRelThreshold_ );
    }

    std::vector<edm::Ptr<flashgg::Electron> > goodElectrons ;
    goodElectrons = selectStdAllElectrons(electrons_h->ptrs(), vertices_h->ptrs(), leptonPtThreshold_, electronEtaThresholds_,
                                          useElectronMVARecipe_, useElectronLooseID_,
                                          rho, iEvent.isRealData() );


    // loop over all diphoton pairs and check if good. If none good, return false
    uint nDiPhotons = diphotons->size();
    for(uint idp=0; idp < nDiPhotons; idp++){

        float leadPt = diphotons->at(idp).leadingPhoton()->pt();
        float subleadPt = diphotons->at(idp).subLeadingPhoton()->pt();
        float mass = diphotons->at(idp).mass();
        float mva = diphotonMVAs->at(idp).mvaValue();

        if(verbose_) std::cout << "    DiPhoton " << idp << ": " << mass << ", " << leadPt << ", " << subleadPt << ", " << mva << std::endl;
        
        if( leadPt/mass < diphotonLeadPtOverMassCut_ || subleadPt/mass < diphotonSubLeadPtOverMassCut_)
            continue;

        if( mva < diphotonMVAcut_ )
            continue;

        
        if(verbose_) std::cout << "    n loose electrons: " << goodElectrons.size() << std::endl;
        int nGoodLep = 0;
        for(uint ilep=0; ilep < goodElectrons.size(); ilep++){
            if(deltaR(goodElectrons.at(ilep)->eta(), goodElectrons.at(ilep)->phi(), diphotons->at(idp).leadingPhoton()->eta(), diphotons->at(idp).leadingPhoton()->phi()) > dRLeadPhoLepCut_ &&
               deltaR(goodElectrons.at(ilep)->eta(), goodElectrons.at(ilep)->phi(), diphotons->at(idp).subLeadingPhoton()->eta(), diphotons->at(idp).subLeadingPhoton()->phi()) > dRSubLeadPhoLepCut_ )
                nGoodLep++;                
        }
        if(verbose_) std::cout << "    n loose muons    : " << goodMuons.size() << std::endl;
        for(uint ilep=0; ilep < goodMuons.size(); ilep++){
            if(deltaR(goodMuons.at(ilep)->eta(), goodMuons.at(ilep)->phi(), diphotons->at(idp).leadingPhoton()->eta(), diphotons->at(idp).leadingPhoton()->phi()) > dRLeadPhoLepCut_ &&
               deltaR(goodMuons.at(ilep)->eta(), goodMuons.at(ilep)->phi(), diphotons->at(idp).subLeadingPhoton()->eta(), diphotons->at(idp).subLeadingPhoton()->phi()) > dRSubLeadPhoLepCut_ )
                nGoodLep++;                
        }

        if(nGoodLep >= acceptIfNLeptons_)
            return true;

        uint jetCollectionIdx = diphotons->at(idp).jetCollectionIndex();

        // check jet collections
        if(!CheckJetCollection(jetCollectionIdx, jets, diphotons->at(idp)) && (jetCollectionIdx==0 || !CheckJetCollection(0, jets, diphotons->at(idp))))
        // if(!CheckJetCollection(jetCollectionIdx, jets, diphotons->at(idp)))
            continue;
        
        // if we've made it here, then it is a good photon pair.
        return true;
    }

   return false;
}

bool SkimFilter::CheckJetCollection(uint jetCollectionIdx, const std::vector< std::vector< flashgg::Jet> >* jets, const flashgg::DiPhotonCandidate& diphoton){

    if (jetCollectionIdx > jets->size() - 1){
        if(verbose_) std::cout << "SHOULDN'T GET HERE" << std::endl;
        return false;
    }

    uint njet = jets->at(jetCollectionIdx).size();
    int nGoodJet = 0;
    int nGoodBJet = 0;
    if(verbose_) std::cout << "    jet collection " << jetCollectionIdx << ", " << njet << " jets\n";
    for(uint j=0; j<njet; j++){
        const flashgg::Jet& jet = jets->at(jetCollectionIdx).at(j);
        if(verbose_) std::cout << "      " << jet.pt() << std::endl;

        if(jet.pt() > jetPtThresh_ && fabs(jet.eta()) < jetEtaThresh_ && 
           deltaR(jet.eta(), jet.phi(), diphoton.leadingPhoton()->eta(), diphoton.leadingPhoton()->phi()) > dRLeadPhoJetCut_ && 
           deltaR(jet.eta(), jet.phi(), diphoton.subLeadingPhoton()->eta(), diphoton.subLeadingPhoton()->phi()) > dRSubLeadPhoJetCut_) {
            nGoodJet += 1;
            float bDisc = jets->at(jetCollectionIdx).at(j).bDiscriminator(btagDiscName_);
            if(bDisc > btagDiscThresh_)
                nGoodBJet += 1;
        }
            
    }

    if(verbose_) std::cout << "     nJet: " << nGoodJet << std::endl;
    if(verbose_) std::cout << "    nBJet: " << nGoodBJet << std::endl;

    if(nGoodJet < nJetCut_)
        return false;
        
    if(nGoodBJet < nBJetCut_)
        return false;

    return true;
 
}

//define this as a plug-in
DEFINE_FWK_MODULE(SkimFilter);
