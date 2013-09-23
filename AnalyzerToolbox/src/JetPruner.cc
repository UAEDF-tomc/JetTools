// -*- C++ -*-
//
// Package:    JetTools/AnalyzerToolbox
// Class:      JetPruner
// 
/**\class JetPruner JetPruner.cc JetTools/AnalyzerToolbox/src/JetPruner.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  john stupak
//         Created:  Sun Sep 22 18:31:27 CDT 2013
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "fastjet/tools/Pruner.hh"

//
// class declaration
//

class JetPruner : public edm::EDProducer {
   public:
      explicit JetPruner(const edm::ParameterSet&);
      ~JetPruner();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginJob() ;
      virtual void produce(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      
      virtual void beginRun(edm::Run&, edm::EventSetup const&);
      virtual void endRun(edm::Run&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);

      // ----------member data ---------------------------
  edm::InputTag src_ ;

  double RcutFactor_;
  double zCut_;

  fastjet::Pruner* pruner;

};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
JetPruner::JetPruner(const edm::ParameterSet& iConfig):src_(iConfig.getParameter<edm::InputTag>("src"))
{
  produces<std::vector<pat::Jet> >();

  RcutFactor_=iConfig.getParameter<double>("RcutFactor");
  zCut_=iConfig.getParameter<double>("zCut");

  pruner(fastjet::cambridge_algorithm, zCut_, RcutFactor_);

}


JetPruner::~JetPruner()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
JetPruner::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   // read input collection
   edm::Handle<edm::View<pat::Jet> > jets;
   iEvent.getByLabel(src_, jets);

   // prepare room for output
   std::vector<pat::Jet> outJets;   outJets.reserve(jets->size());

   for ( typename edm::View<pat::Jet>::const_iterator jetIt = jets->begin() ; jetIt != jets->end() ; ++jetIt ) {
     std::vector<fastjet::PseudoJet> j;

     std::vector<reco::PFCandidatePtr> constituents=jetIt->getPFConstituents();
     for ( std::vector<reco::PFCandidatePtr>::const_iterator constituentIt = constituents->begin() ; constituentIt != constituents->end() ; ++constituentIt ) {
       LorentzVector lv=constituentIt->p4();
       fastjet::PseudoJet c=fastjet::PseudoJet(lv->px(), lv->py, lv->pz(), lv->energy());
       j->push_back(c);
     }

     double R = 0.8;
     fastjet::JetDefinition jet_def(fastjet::cambridge_algorithm, R);
     fastjet::ClusterSequence cs(input_particles, jet_def);
     vector<fastjet::PseudoJet> jets = sorted_by_pt(cs.inclusive_jets());

     PseudoJet prunedJet=pruner(jets[0]);

     pat::Jet outJet(*jetIt);
     outJet.addUserFloat("prunedMass", prunedJet.m());
     
     outJets.push_back(outJet);
     std::auto_ptr<std::vector<pat::Jet> > out(new std::vector<pat::Jet>(outJets));
     iEvent.put(out);
}

// ------------ method called once each job just before starting event loop  ------------
void 
JetPruner::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
JetPruner::endJob() {
}

// ------------ method called when starting to processes a run  ------------
void 
JetPruner::beginRun(edm::Run&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
JetPruner::endRun(edm::Run&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
JetPruner::beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
JetPruner::endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
JetPruner::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(JetPruner);
