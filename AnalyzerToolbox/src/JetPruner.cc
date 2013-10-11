// -*- C++ -*-
//
// Package:    JetPruner
// Class:      JetPruner
// 
/**\class JetPruner JetPruner.cc JetTools/AnalyzerToolbox/src/JetPruner.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  john stupak
//         Created:  Fri Oct 11 16:24:13 CDT 2013
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include <fastjet/JetDefinition.hh>
#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequence.hh>
#include <fastjet/tools/Pruner.hh>


//
// class declaration
//

class JetPruner : public edm::EDAnalyzer {
public:
  JetPruner();
  explicit JetPruner(const edm::ParameterSet&);
  ~JetPruner();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  void prune(pat::Jet* theJet, const std::vector<fastjet::PseudoJet> thePseudoJet);

private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  virtual void beginRun(edm::Run const&, edm::EventSetup const&);
  virtual void endRun(edm::Run const&, edm::EventSetup const&);
  virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
  virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

  // ----------member data ---------------------------

  fastjet::Pruner *fjPruner_;

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
JetPruner::JetPruner(){}

JetPruner::JetPruner(const edm::ParameterSet& iConfig){
  
  std::string jetAlgString=iConfig.getParameter<std::string>("pruningJetAlg");
  /*fastjet::JetAlgorithm {
    fastjet::kt_algorithm = 0, fastjet::cambridge_algorithm = 1, fastjet::antikt_algorithm = 2, fastjet::genkt_algorithm = 3,
    fastjet::cambridge_for_passive_algorithm = 11, fastjet::genkt_for_passive_algorithm = 13, fastjet::ee_kt_algorithm = 50, fastjet::ee_genkt_algorithm = 53,
    fastjet::plugin_algorithm = 99
    }
  */

  fastjet::JetAlgorithm jetAlg;
  if(jetAlgString=="KT") jetAlg=fastjet::kt_algorithm;
  if (jetAlgString=="CA") jetAlg=fastjet::cambridge_algorithm;
  if (jetAlgString=="AK") jetAlg=fastjet::antikt_algorithm;

  double pruningJetSize=iConfig.getParameter<double>("pruningJetSize");
  fastjet::JetDefinition jetDef(jetAlg, pruningJetSize);

  double RcutFactor=iConfig.getParameter<double>("RcutFactor");
  double zCut=iConfig.getParameter<double>("zCut");

  fjPruner_=new fastjet::Pruner(jetDef, zCut, RcutFactor);
}


JetPruner::~JetPruner()
{
 
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
JetPruner::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;



#ifdef THIS_IS_AN_EVENT_EXAMPLE
  Handle<ExampleData> pIn;
  iEvent.getByLabel("example",pIn);
#endif
   
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  ESHandle<SetupData> pSetup;
  iSetup.get<SetupRecord>().get(pSetup);
#endif
}

void JetPruner::prune(pat::Jet* theJet, const std::vector<fastjet::PseudoJet> thePseudoJet){

  fastjet::PseudoJet prunedJet=(*fjPruner_)(thePseudoJet[0]);

  double prunedMass=prunedJet.m();
  theJet->addUserFloat("prunedMass",prunedMass);

  std::vector<fastjet::PseudoJet> kept_subjets = prunedJet.pieces();
  double largest_mass_subjet = 0;
  for (size_t i = 0; i < kept_subjets.size(); i++){
      if (kept_subjets[i].m() > largest_mass_subjet){
	largest_mass_subjet = kept_subjets[i].m();
      }
  }
  
  double massDrop=1;
  if (prunedMass!=0) massDrop = largest_mass_subjet/prunedMass;
  theJet->addUserFloat("massDrop",massDrop);
}

// ------------ method called once each job just before starting event loop  ------------
void 
JetPruner::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
JetPruner::endJob() 
{
}

// ------------ method called when starting to processes a run  ------------
void 
JetPruner::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
JetPruner::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
JetPruner::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
JetPruner::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
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