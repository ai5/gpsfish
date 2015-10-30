/* moveStackRejections.cc
 */
#include "osl/search/moveStackRejections.h"
#include "osl/misc/lightMutex.h"
#include "osl/eval/evalTraits.h"
#include "osl/oslConfig.h"
#include <iostream>


std::ostream& osl::search::operator<<(std::ostream& os,osl::search::OnBoardElement const& mp)
{
  return os << "[" << mp.pos() << "," << mp.ptypeO() <<  "]";
}

std::ostream& osl::search::operator<<(std::ostream& os,osl::search::StandElements const& mp)
{
  os << "[";
  for(int ptypeIndex=8;ptypeIndex<=15;ptypeIndex++)
    os << (int)mp.v.c8[ptypeIndex-osl::PTYPE_BASIC_MIN] << ",";
  return os << "]";
}
void osl::search::StateElements::
addMyBoard(osl::Square pos,osl::PtypeO ptypeO)
{
  short posPtypeO=OnBoardElement::makePosPtypeO(pos,ptypeO);
  for(size_t i=0;i<myOnboardMinus.size();++i){
    if(myOnboardMinus[i].posPtypeO==posPtypeO){
      myOnboardMinus[i]=myOnboardMinus[myOnboardMinus.size()-1];
      myOnboardMinus.pop_back();
      return;
    }
  }
  myOnboardPlus.push_back(OnBoardElement(pos,ptypeO));
}
void osl::search::StateElements::
subMyBoard(osl::Square pos,osl::PtypeO ptypeO)
{
  short posPtypeO=OnBoardElement::makePosPtypeO(pos,ptypeO);
  for(size_t i=0;i<myOnboardPlus.size();++i){
    if(myOnboardPlus[i].posPtypeO==posPtypeO){
      myOnboardPlus[i]=myOnboardPlus[myOnboardPlus.size()-1];
      myOnboardPlus.pop_back();
      return;
    }
  }
  myOnboardMinus.push_back(OnBoardElement(pos,ptypeO));
}
void osl::search::StateElements::
addOpBoard(osl::Square pos,osl::PtypeO ptypeO)
{
  short posPtypeO=OnBoardElement::makePosPtypeO(pos,ptypeO);
  for(size_t i=0;i<opOnboardMinus.size();++i){
    if(opOnboardMinus[i].posPtypeO==posPtypeO){
      opOnboardMinus[i]=opOnboardMinus[opOnboardMinus.size()-1];
      opOnboardMinus.pop_back();
      return;
    }
  }
  opOnboardPlus.push_back(OnBoardElement(pos,ptypeO));
}
void osl::search::StateElements::
subOpBoard(osl::Square pos,osl::PtypeO ptypeO)
{
  short posPtypeO=OnBoardElement::makePosPtypeO(pos,ptypeO);
  for(size_t i=0;i<opOnboardPlus.size();++i){
    if(opOnboardPlus[i].posPtypeO==posPtypeO){
      opOnboardPlus[i]=opOnboardPlus[opOnboardPlus.size()-1];
      opOnboardPlus.pop_back();
      return;
    }
  }
  opOnboardMinus.push_back(OnBoardElement(pos,ptypeO));
}
void osl::search::StateElements::addStand(osl::Ptype ptype)
{
  stand.add(ptype);
}
void osl::search::StateElements::subStand(osl::Ptype ptype)
{
  stand.sub(ptype);
}
void osl::search::StateElements::addMyMove(osl::Move move)
{
  if(move.isDrop()){
    addMyBoard(move.to(),move.ptypeO());
    addStand(move.ptype());
  }
  else{
    if(move.isCapture()){
      subOpBoard(move.to(),move.capturePtypeO());
      subStand(unpromote(getPtype(move.capturePtypeO()))); // 相手にとって
      addMyBoard(move.to(),move.ptypeO());
      subMyBoard(move.from(),move.oldPtypeO());
    }
    else{
      addMyBoard(move.to(),move.ptypeO());
      subMyBoard(move.from(),move.oldPtypeO());
    }
  }
}
void osl::search::StateElements::addOpMove(osl::Move move)
{
  if(move.isDrop()){
    addOpBoard(move.to(),move.ptypeO());
    subStand(move.ptype());
  }
  else{
    if(move.isCapture()){
      subMyBoard(move.to(),move.capturePtypeO());
      addStand(unpromote(getPtype(move.capturePtypeO()))); // 相手にとって
      addOpBoard(move.to(),move.ptypeO());
      subOpBoard(move.from(),move.oldPtypeO());
    }
    else{
      addOpBoard(move.to(),move.ptypeO());
      subOpBoard(move.from(),move.oldPtypeO());
    }
  }
}

/**
 * こちらはstateにlastMoveをapplyした後に可能かどうかのチェックをおこなう．
 */
template<osl::Player P>
bool osl::search::StateElements::
validSimpleMove(osl::NumEffectState const& state,
		osl::search::OnBoardElement const& fromElement,
		osl::search::OnBoardElement const& toElement,Move lastMove) const
{
  Square from=fromElement.pos(), to=toElement.pos();
  PtypeO fromPtypeO=fromElement.ptypeO(), toPtypeO=toElement.ptypeO();

  if(fromPtypeO!=toPtypeO &&
     (fromPtypeO!=unpromote(toPtypeO) ||
      (!from.canPromote<P>() && !to.canPromote<P>())))
    return false;
  EffectContent effect=Ptype_Table.getEffect(fromPtypeO,from,to);
  if(!effect.hasEffect()) return false;
  if(effect.hasUnblockableEffect()) return true;
  Offset o=effect.offset();
  Square moveTo=lastMove.to(),moveFrom=lastMove.from();
  Square pos=from+o;
  for (;; pos+=o) {
    if(pos==to) return true;
    if(pos==moveTo) return false;
    if(!state.pieceAt(pos).isEmpty()){
      if(pos==moveFrom){
	for (pos+=o;; pos+=o) {
	  if(pos==to) return true;
	  if(pos==moveTo) return false;
	  if(!state.pieceAt(pos).isEmpty()){
	    break;
	  }
	}
      }
      break;
    }
  }
  return false;
}

template<osl::Player P>
bool osl::search::StateElements::
validCaptureMove(osl::NumEffectState const& state,
		 osl::search::OnBoardElement const& fromElement,
		 osl::search::OnBoardElement const& toElement,
		 osl::search::OnBoardElement const& captureElement,
		 osl::Move lastMove) const
{
  Square to=toElement.pos();
  if(to!=captureElement.pos()) return false;
  Square from=fromElement.pos();
  PtypeO fromPtypeO=fromElement.ptypeO(), toPtypeO=toElement.ptypeO();

  if(fromPtypeO!=toPtypeO &&
     (fromPtypeO!=unpromote(toPtypeO) ||
      (!from.canPromote<P>() && !to.canPromote<P>())))
    return false;
  EffectContent effect=Ptype_Table.getEffect(fromPtypeO,from,to);
  if(!effect.hasEffect()) return false;
  if(effect.hasUnblockableEffect()) return true;
  Offset o=effect.offset();
  Square moveTo=lastMove.to(),moveFrom=lastMove.from();
  Square pos=from+o;
  for (;; pos+=o) {
    if(pos==to) return true;
    if(pos==moveTo) return false;
    if(!state.pieceAt(pos).isEmpty()){
      if(pos==moveFrom){
	for (pos+=o;; pos+=o) {
	  if(pos==to) return true;
	  if(pos==moveTo) return false;
	  if(!state.pieceAt(pos).isEmpty()){
	    break;
	  }
	}
      }
      break;
    }
  }
  return false;
}
/**
 * rejectable patterns
 * 0 - sennichite (or piece losing loop)
 * myPlus 1 myMinus 1 - my simple move
 * myPlus 1 myMinus 1 opMinus 1 - my capture move
 * myPlus 1 - my drop move
 * opPlus 1 opMinus 1 - op simple move
 * opPlus 1 opMinus 1 myPlus 1 - op capture move
 * opMinus 1 - op drop move
 */
template<osl::Player P>
bool osl::search::StateElements::canReject(osl::NumEffectState const& state,bool mayRejectSennichite,bool isRootMove,Move lastMove,Move actualMove) const
{
  const Player altP=alt(P);
  switch(myOnboardPlus.size()){
  case 0:
    switch(opOnboardPlus.size()){
    case 1: // myPlus=0, opPlus=1
      if(opOnboardMinus.size()==1 && myOnboardMinus.size()==0){
	// op simple move
	if(validSimpleMove<altP>(state,opOnboardPlus[0],opOnboardMinus[0],lastMove)){
	  if(!mayRejectSennichite && stand.isZero()){
	    return false;
	  }
	  return stand.geZero();
	}
      }
      return false;
    case 0:  // myPlus=0, opPlus=0
      if(opOnboardMinus.size()==1){
	if(myOnboardMinus.size()==0){
	  StandElements localStand(stand);
	// op drop move
	  Ptype ptype=getPtype(opOnboardMinus[0].ptypeO());
	  if(!isPromoted(ptype)){
	    localStand.sub(ptype);
	    if(!mayRejectSennichite && localStand.isZero()){
	      return false;
	    }
	    return localStand.geZero();
	  }
	}
      }
      else{ // pass moves (including piece loosing cases)
	if(opOnboardMinus.size()==0 && myOnboardMinus.size()==0){
	  if(isRootMove) return stand.gtZero();
	  else return stand.geZero(); 
	}
      }
      return false;
    default: return false;
    }
  case 1:  // myPlus=1
    switch(myOnboardMinus.size()){
    case 1:  // myPlus=1, myMinus=1
      switch(opOnboardMinus.size()){
      case 1:   // myPlus=1, myMinus=1, opMinus=1
	if(opOnboardPlus.size()==0){ // my capture move
	  if(validCaptureMove<P>(state,myOnboardMinus[0],myOnboardPlus[0],opOnboardMinus[0],lastMove))
	    {
	    StandElements localStand(stand);
	    Ptype capturePtype=unpromote(getPtype(opOnboardMinus[0].ptypeO()));
	    // altPに関して減りすぎた分を増やす?
	    // 相手に取って減っているはずなので，それをキャンセルする
	    localStand.add(capturePtype);
	    if(localStand.isZero()){
	      assert(actualMove.player()==P);
	      if(!mayRejectSennichite &&
		 actualMove.ptypeO()==myOnboardPlus[0].ptypeO() &&
		 actualMove.from()==myOnboardMinus[0].pos() &&
		 actualMove.to()==myOnboardPlus[0].pos()) return false;
	      return true;
	    }
	    return localStand.geZero();
	  }
	}
	return false;
      case 0:   // myPlus=1, myMinus=1, opMinus=0
	if(opOnboardPlus.size()==0){ // my simple move
	  if(validSimpleMove<P>(state,myOnboardMinus[0],myOnboardPlus[0],lastMove)){
	    if(stand.isZero()){
	      assert(actualMove.player()==P);
	      if(!mayRejectSennichite &&
		 actualMove.ptypeO()==myOnboardPlus[0].ptypeO() &&
		 actualMove.from()==myOnboardMinus[0].pos() &&
		 actualMove.to()==myOnboardPlus[0].pos()) return false;
	      return true;
	    }
	    return stand.geZero();
	  }
	}
	return false;
      }
    case 0:    // myPlus=1, myMinus=0
      if(opOnboardPlus.size()==1){
	if(opOnboardMinus.size()==1){
	    if(validCaptureMove<altP>(state,opOnboardPlus[0],opOnboardMinus[0],myOnboardPlus[0],lastMove))
	    {
	    StandElements localStand(stand);
	    Ptype capturePtype=unpromote(getPtype(myOnboardPlus[0].ptypeO()));
	    // 次のmoveを実行すると相手に取っては増える．
	    localStand.add(capturePtype);
	    if(!mayRejectSennichite && localStand.isZero()){
	      return false;
	    }
	    return localStand.geZero();
	  }
	}
	else return false;
      }
      else if(opOnboardPlus.size()==0 && opOnboardMinus.size()==0 
	      && !isPromoted(myOnboardPlus[0].ptypeO()) ){
	// my drop move
	  StandElements localStand(stand);
	  localStand.sub(getPtype(myOnboardPlus[0].ptypeO()));
	  if(localStand.isZero()){
	    if(!mayRejectSennichite &&
	       actualMove.ptypeO()==myOnboardPlus[0].ptypeO() &&
	       actualMove.isDrop() &&
	       actualMove.to()==myOnboardPlus[0].pos()) return false;
	    return true;
	  }
	  return localStand.geZero();
      }
    }
  default: return false;
  }
}

std::ostream& osl::search::operator<<(std::ostream& os,osl::search::StateElements const& mps)
{
  {
    os << "[ MyOnboardPlus(";
    for(size_t i=0;i<mps.myOnboardPlus.size();i++)
      os << mps.myOnboardPlus[i] << ",\n";
    os << "),";
  }
  {
    os << "[ MyOnboardMinus(";
    for(size_t i=0;i<mps.myOnboardMinus.size();i++)
      os << mps.myOnboardMinus[i] << ",\n";
    os << "),";
  }
  {
    os << "[ OpOnboardPlus(";
    for(size_t i=0;i<mps.opOnboardPlus.size();i++)
      os << mps.opOnboardPlus[i] << ",\n";
    os << "),";
  }
  {
    os << "[ OpOnboardMinus(";
    for(size_t i=0;i<mps.opOnboardMinus.size();i++)
      os << mps.opOnboardMinus[i] << ",\n";
    os << "),";
  }
  return os << "Stand(" << mps.stand << ") ]\n" << std::endl;
}

#ifdef SHOW_PROBE_COUNTER
struct ProbeCounter{
#ifdef OSL_USE_RACE_DETECTOR
  osl::LightMutex mutex;
#endif
  osl::misc::CArray<long long int,1024> check,hit;
  ProbeCounter(){}
  void incCheck(int d){ 
#ifdef OSL_USE_RACE_DETECTOR
    osl::LightMutex::scoped_lock lk(mutex);
#endif
    check[d]++; 
  }
  void incHit(int d){ 
#ifdef OSL_USE_RACE_DETECTOR
    osl::LightMutex::scoped_lock lk(mutex);
#endif
    hit[d]++; 
  }
  ~ProbeCounter(){
    for(int i=0;i<1024;i++){
      if(check[i]!=0){
	std::cerr << i << " : " << hit[i] << "/" << check[i] << std::endl;
      }
    }
  }
};
ProbeCounter probeCounter;
#endif

template<osl::Player P>
bool osl::search::MoveStackRejections::
probe(osl::NumEffectState const& state,osl::container::MoveStack const& history,int ply,osl::Move const& m,int alpha,int checkCountOfAltP)
{
  StateElements elements;
  elements.addMyMove(m);
  assert(m.player()==P);
  bool existNormal=false;
  for(int i=1;i<ply;i+=2){
    Move m1=history.lastMove(i);
    if(m1.isNormal()){
      assert(m1.player()==alt(P));
      elements.addOpMove(m1);
      existNormal=true;
    }
    else if(m1.isInvalid()) return false;
    if(elements.isLoop()){
      bool mayRejectSennichite=osl::eval::notLessThan(P,alpha,0) && (i>checkCountOfAltP*2-1);
      if(elements.stand.isZero()){
	if(mayRejectSennichite) return true;
      }
      else if(elements.stand.geZero()) return true;
    }
    Move m2=history.lastMove(i+1);
    if(m2.isNormal()){
      assert(m2.player()==P);
      elements.addMyMove(m2);
      existNormal=true;
    }
    else if(m2.isInvalid()) return false;
#ifdef SHOW_PROBE_COUNTER
    probeCounter.incCheck(i);
#endif
    // i==1 -- checkCount=1
    // i==3 -- checkCount=2
    bool mayRejectSennichite=osl::eval::notLessThan(P,alpha,0) && (i>checkCountOfAltP*2-1);
    bool isRootMove=(i == ply-1);
    if(
       existNormal && 
       elements.canReject<P>(state,mayRejectSennichite,isRootMove,m,m2)
       ){
#ifdef SHOW_PROBE_COUNTER
      probeCounter.incHit(i);
#endif
      return true;
    }
  }
  return false;
}
namespace osl
{
  namespace search
  {
    template bool MoveStackRejections::probe<BLACK>(
						    osl::NumEffectState const& state,osl::container::MoveStack const& history,int ply,osl::Move const& m,int alpha, int checkCountOfAltP);
    template bool MoveStackRejections::probe<WHITE>(
						    osl::NumEffectState const& state,osl::container::MoveStack const& history,int ply,osl::Move const& m,int alpha,int checkCountOfAltP);
  }
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

