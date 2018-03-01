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
#include "flashgg/DataFormats/interface/DiPhotonCandidate.h"
#include "flashgg/DataFormats/interface/DiPhotonMVAResult.h"

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

      // ----------member data ---------------------------
    edm::EDGetTokenT<std::vector< std::vector< flashgg::Jet> > > jetToken_;
    edm::EDGetTokenT< std::vector< flashgg::DiPhotonCandidate> > diphoToken_;
    edm::EDGetTokenT< std::vector< flashgg::DiPhotonMVAResult> > diphoMVAToken_;

    string btagDiscName_;

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
    diphoMVAToken_( consumes< std::vector< flashgg::DiPhotonMVAResult> >(iConfig.getParameter<edm::InputTag>("inputDiPhotonMVA")))
{
    btagDiscName_ = iConfig.getParameter<string>("btagDiscName");

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

    // loop over all diphoton pairs and check if good. If none good, return false
    uint nDiPhotons = diphotons->size();
    for(uint idp=0; idp < nDiPhotons; idp++){

        float leadPt = diphotons->at(idp).leadingPhoton()->pt();
        float subleadPt = diphotons->at(idp).subLeadingPhoton()->pt();
        float mass = diphotons->at(idp).mass();
        float mva = diphotonMVAs->at(idp).mvaValue();

        // std::cout << "    DiPhoton " << idp << ": " << mass << ", " << leadPt << ", " << subleadPt << ", " << mva << std::endl;
        
        if( leadPt/mass < diphotonLeadPtOverMassCut_ || subleadPt/mass < diphotonSubLeadPtOverMassCut_)
            continue;

        if( mva < diphotonMVAcut_ )
            continue;

        uint jetCollectionIdx = diphotons->at(idp).jetCollectionIndex();

        uint njet = jets->at(jetCollectionIdx).size();
        int nGoodJet = 0;
        int nGoodBJet = 0;
        std::cout << "    jet collection " << jetCollectionIdx << ", " << njet << " jets\n";
        for(uint j=0; j<njet; j++){
            const flashgg::Jet& jet = jets->at(jetCollectionIdx).at(j);
            // std::cout << "      " << jet.pt() << std::endl;

            if(jet.pt() > jetPtThresh_ && fabs(jet.eta()) < jetEtaThresh_ && 
               deltaR(jet.eta(), jet.phi(), diphotons->at(idp).leadingPhoton()->eta(), diphotons->at(idp).leadingPhoton()->phi()) > dRLeadPhoJetCut_ && 
               deltaR(jet.eta(), jet.phi(), diphotons->at(idp).subLeadingPhoton()->eta(), diphotons->at(idp).subLeadingPhoton()->phi()) > dRSubLeadPhoJetCut_) {
                nGoodJet += 1;
                float bDisc = jets->at(jetCollectionIdx).at(j).bDiscriminator(btagDiscName_);
                if(bDisc > btagDiscThresh_)
                    nGoodBJet += 1;
            }
            
        }

        // std::cout << "     nJet: " << nGoodJet << std::endl;
        // std::cout << "    nBJet: " << nGoodBJet << std::endl;

        if(nGoodJet < nJetCut_)
            continue;
        
        if(nGoodBJet < nBJetCut_)
            continue;
        
        // if we've made it here, then it is a good photon pair.
        return true;
    }

   return false;
}

//define this as a plug-in
DEFINE_FWK_MODULE(SkimFilter);
