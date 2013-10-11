#include <map>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>

#include "TH1.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"


class SimpleJetAnalyzer : public edm::EDAnalyzer {

public:
  /// default contructor
  explicit SimpleJetAnalyzer(const edm::ParameterSet& cfg);
  /// default destructor
  ~SimpleJetAnalyzer(){};
  
private:
  /// everything that needs to be done during the even loop
  virtual void analyze(const edm::Event& event, const edm::EventSetup& setup);

  /// check if histogram was booked
  bool booked(const std::string histName) const { return hists_.find(histName.c_str())!=hists_.end(); };
  /// fill histogram if it had been booked before
  void fill(const std::string histName, double value) const { if(booked(histName.c_str())) hists_.find(histName.c_str())->second->Fill(value); };
  // print jet pt for each level of energy corrections
  void print(edm::View<pat::Jet>::const_iterator& jet, unsigned int idx);

private:  
  /// pat jets
  edm::InputTag jets_;
  /// management of 1d histograms
  std::map<std::string,TH1F*> hists_; 
};


SimpleJetAnalyzer::SimpleJetAnalyzer(const edm::ParameterSet& cfg):
  jets_(cfg.getParameter<edm::InputTag>("src"))
{
  // register TFileService
  edm::Service<TFileService> fs;
  // jet multiplicity
  hists_["mult" ]=fs->make<TH1F>("mult" , "N_{Jet}"          ,   15,   0.,   15.);
  // jet pt (for all jets)
  hists_["pt"   ]=fs->make<TH1F>("pt"   , "p_{T}(Jet) [GeV]" ,   60,   0.,  300.);
  // jet eta (for all jets)
  hists_["eta"  ]=fs->make<TH1F>("eta"  , "#eta (Jet)"       ,   60,  -3.,    3.);

  // dijet mass (if available)
  hists_["prunedMass" ]=fs->make<TH1F>("prunedMass" , "M_{pruned} [GeV]"     ,   50,   0.,  500.);
}

void
SimpleJetAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& setup)
{
  // recieve jet collection label
  edm::Handle<edm::View<pat::Jet> > jets;
  event.getByLabel(jets_,jets);

  // loop jets
  for(edm::View<pat::Jet>::const_iterator jet=jets->begin(); jet!=jets->end(); ++jet){

    // fill basic kinematics
    fill( "pt" , jet->pt());
    fill( "eta", jet->eta());

    std::cout<<"jet->eta(): "<<jet->eta()<<std::endl;
    std::cout<<"tau1: "<<jet->userFloat("tau1")<<std::endl;
    std::cout<<"jibberish: "<<jet->userFloat("asergwcqf")<<std::endl;
    std::cout<<"prunedMass:"<<jet->userFloat("prunedMass")<<std::endl;
    fill("prunedMass", jet->userFloat("prunedMass"));
  }
  // jet multiplicity
  fill( "mult" , jets->size());
  // invariant dijet mass for first two leading jets
  if(jets->size()>1){ fill( "mass", ((*jets)[0].p4()+(*jets)[1].p4()).mass());}
}

void
SimpleJetAnalyzer::print(edm::View<pat::Jet>::const_iterator& jet, unsigned int idx)
{
  //edm::LogInfo log("JEC");
  std::cout << "[" << idx << "] :: eta=" << std::setw(10) << jet->eta() << " phi=" << std::setw(10) << jet->phi() << " size: " << jet->availableJECLevels().size() << std::endl;
  for(unsigned int idx=0; idx<jet->availableJECLevels().size(); ++idx){
    pat::Jet correctedJet;
    if(jet->availableJECLevels()[idx].find("L5Flavor")!=std::string::npos|| 
       jet->availableJECLevels()[idx].find("L7Parton")!=std::string::npos ){
      correctedJet=jet->correctedJet(idx, pat::JetCorrFactors::UDS);
    }
    else{
      correctedJet=jet->correctedJet(idx, pat::JetCorrFactors::NONE );
    }
    std::cout << std::setw(10) << correctedJet.currentJECLevel() << " pt=" << std::setw(10) << correctedJet.pt() << std::endl;
  }
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SimpleJetAnalyzer);