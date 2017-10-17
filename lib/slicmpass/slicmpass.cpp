/*
 * File: slicmpass.cpp
 *
 * Description:
 *  This is a sample source file for a library.  It helps to demonstrate
 *  how to setup a project that uses the LLVM build system, header files,
 *  and libraries.
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
//zyuxuan
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/ProfileInfo.h"

/* LLVM Header File
#include "llvm/Support/DataTypes.h"
*/
#include "llvm/IR/Module.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/IndexedMap.h"
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>


// add from SLICMtemplate.cpp
#define DEBUG_TYPE "slicm"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include <algorithm>

// SLICM includes
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "LAMP/LAMPLoadProfile.h"
#include "llvm/Analysis/ProfileInfo.h"





/* Header file global to this project */


using namespace llvm;
using namespace std;

STATISTIC(NumSunk      , "Number of instructions sunk out of loop");
STATISTIC(NumHoisted   , "Number of instructions hoisted out of loop");
STATISTIC(NumMovedLoads, "Number of load insts hoisted or sunk");
STATISTIC(NumMovedCalls, "Number of call insts hoisted or sunk");
STATISTIC(NumPromoted  , "Number of memory locations promoted to registers");

static cl::opt<bool>
DisablePromotion("disable-slicm-promotion", cl::Hidden,
                 cl::desc("Disable memory promotion in SLICM pass"));


namespace {
	struct slicmpass: public LoopPass {
		
		ProfileInfo* PI;
		LAMPLoadProfile* LAMP;
		static char ID; // Pass identification, replacement for typeid
		slicmpass() : LoopPass(ID) {
		//      initializeSLICMPass(*PassRegistry::getPassRegistry()); // 583 - commented out
		}

		virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

		/// This transformation requires natural loop information & requires that
		/// loop preheaders be inserted into the CFG...

		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
		//      AU.setPreservesCFG();                   // 583 - commented out
      			AU.addRequired<DominatorTree>();
			AU.addRequired<LoopInfo>();
		//      AU.addRequiredID(LoopSimplifyID);       // 583 - commented out
			AU.addRequired<AliasAnalysis>();
		//      AU.addPreserved<AliasAnalysis>();       // 583 - commented out
		//      AU.addPreserved("scalar-evolution");    // 583 - commented out
		//      AU.addPreservedID(LoopSimplifyID);      // 583 - commented out
			AU.addRequired<TargetLibraryInfo>();
			AU.addRequired<ProfileInfo>();
			AU.addRequired<LAMPLoadProfile>();
	}

	using llvm::Pass::doFinalization;

	bool doFinalization() {
		assert(LoopToAliasSetMap.empty() && "Didn't free loop alias sets");
		return false;
	}

	private:
		AliasAnalysis *AA;       // Current AliasAnalysis information
		LoopInfo      *LI;       // Current LoopInfo
		DominatorTree *DT;       // Dominator Tree for the current Loop.

		DataLayout *TD;          // DataLayout for constant folding.
		TargetLibraryInfo *TLI;  // TargetLibraryInfo for constant folding.

		// State that is updated as we process loops.
		bool Changed;            // Set to true when we change anything.
		BasicBlock *Preheader;   // The preheader block of the current loop...
		Loop *CurLoop;           // The current loop we are working on...
		AliasSetTracker *CurAST; // AliasSet information for the current loop...
		bool MayThrow;           // The current loop contains an instruction which
                             // may throw, thus preventing code motion of
                             // instructions with side effects.
		DenseMap<Loop*, AliasSetTracker*> LoopToAliasSetMap;

		/// cloneBasicBlockAnalysis - Simple Analysis hook. Clone alias set info.
		void cloneBasicBlockAnalysis(BasicBlock *From, BasicBlock *To, Loop *L);

		/// deleteAnalysisValue - Simple Analysis hook. Delete value V from alias
		/// set.
		void deleteAnalysisValue(Value *V, Loop *L);

		/// SinkRegion - Walk the specified region of the CFG (defined by all blocks
		/// dominated by the specified block, and that are in the current loop) in
		/// reverse depth first order w.r.t the DominatorTree.  This allows us to
    		/// visit uses before definitions, allowing us to sink a loop body in one
    		/// pass without iteration.
    
    		void SinkRegion(DomTreeNode *N);

    		/// HoistRegion - Walk the specified region of the CFG (defined by all
    		/// blocks dominated by the specified block, and that are in the current
    		/// loop) in depth first order w.r.t the DominatorTree.  This allows us to
    		/// visit definitions before uses, allowing us to hoist a loop body in one
    		/// pass without iteration.
    		///
    		void HoistRegion(DomTreeNode *N);

    		/// inSubLoop - Little predicate that returns true if the specified basic
    		/// block is in a subloop of the current one, not the current one itself.
    
    		bool inSubLoop(BasicBlock *BB) {
      			assert(CurLoop->contains(BB) && "Only valid if BB is IN the loop");
      			return LI->getLoopFor(BB) != CurLoop;
    		}

    		/// sink - When an instruction is found to only be used outside of the loop,
    		/// this function moves it to the exit blocks and patches up SSA form as
    		/// needed.
    		void sink(Instruction &I);

    		/// hoist - When an instruction is found to only use loop invariant operands
    		/// that is safe to hoist, this instruction is called to do the dirty work.
    		void hoist(Instruction &I);

    		/// isSafeToExecuteUnconditionally - Only sink or hoist an instruction if it
    		/// is not a trapping instruction or if it is a trapping instruction and is
    		/// guaranteed to execute.
    		bool isSafeToExecuteUnconditionally(Instruction &I);

    		/// isGuaranteedToExecute - Check that the instruction is guaranteed to
    		/// execute.
    		bool isGuaranteedToExecute(Instruction &I);

    		/// pointerInvalidatedByLoop - Return true if the body of this loop may
    		/// store into the memory location pointed to by V.
    		bool pointerInvalidatedByLoop(Value *V, uint64_t Size,
                                  const MDNode *TBAAInfo) {
      		// Check to see if any of the basic blocks in CurLoop invalidate *V.
      		return CurAST->getAliasSetForPointer(V, Size, TBAAInfo).isMod();
	}

	bool canSinkOrHoistInst(Instruction &I);
	bool canSinkOrHoistLoadAndDependency(Instruction &I);

	bool isNotUsedInLoop(Instruction &I);

	void PromoteAliasSet(AliasSet &AS,
	SmallVectorImpl<BasicBlock*> &ExitBlocks,
	SmallVectorImpl<Instruction*> &InsertPts);
  };
}

char slicmpass::ID = 0;
/*
// 583 - commented out INITIALIZE_ macros & createSLICMPass
INITIALIZE_PASS_BEGIN(SLICM, "slicm", "Loop Invariant Code Motion", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTree)
INITIALIZE_PASS_DEPENDENCY(LoopInfo)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfo)
INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_END(SLICM, "slicm", "Loop Invariant Code Motion", false, false)

Pass *llvm::createSLICMPass() { return new SLICM(); }
*/
static RegisterPass<slicmpass> X("slicmpass", "Speculative Loop Invariant Code Motion");

/// Hoist expressions out of the specified loop. Note, alias info for inner
/// loop is not preserved so it is not a good idea to run SLICM multiple
/// times on one loop.
///
bool slicmpass::runOnLoop(Loop *L, LPPassManager &LPM) {
	Changed = false;

	// Get our Loop and Alias Analysis information...
	LI = &getAnalysis<LoopInfo>();
	AA = &getAnalysis<AliasAnalysis>();
	DT = &getAnalysis<DominatorTree>();

	TD = getAnalysisIfAvailable<DataLayout>();
	TLI = &getAnalysis<TargetLibraryInfo>();
	
	PI = &getAnalysis<ProfileInfo>();
	LAMP = &getAnalysis<LAMPLoadProfile>();

	CurAST = new AliasSetTracker(*AA);
	// Collect Alias info from subloops.
	for (Loop::iterator LoopItr = L->begin(), LoopItrE = L->end();LoopItr != LoopItrE; ++LoopItr) {
		Loop *InnerL = *LoopItr;
		AliasSetTracker *InnerAST = LoopToAliasSetMap[InnerL];
		assert(InnerAST && "Where is my AST?");

		// What if InnerLoop was modified by other passes ?
		CurAST->add(*InnerAST);

		// Once we've incorporated the inner loop's AST into ours, we don't need the
		// subloop's anymore.
		delete InnerAST;
		LoopToAliasSetMap.erase(InnerL);
	}
	CurLoop = L;
	// Get the preheader block to move instructions into...
	Preheader = L->getLoopPreheader();

	// Loop over the body of this loop, looking for calls, invokes, and stores.
	// Because subloops have already been incorporated into AST, we skip blocks in
	// subloops.

	for (Loop::block_iterator I = L->block_begin(), E = L->block_end(); I != E; ++I) {
		BasicBlock *BB = *I;
		if (LI->getLoopFor(BB) == L) {       // Ignore blocks in subloops.	
			CurAST->add(*BB);                 // Incorporate the specified basic block
		}
	}

	MayThrow = false;
	// TODO: We've already searched for instructions which may throw in subloops.
	// We may want to reuse this information.
	for (Loop::block_iterator BB = L->block_begin(), BBE = L->block_end(); (BB != BBE) && !MayThrow ; ++BB)
		for (BasicBlock::iterator I = (*BB)->begin(), E = (*BB)->end();(I != E) && !MayThrow; ++I)
			MayThrow |= I->mayThrow();

	// We want to visit all of the instructions in this loop... that are not parts
	// of our subloops (they have already had their invariants hoisted out of
	// their loop, into this loop, so there is no need to process the BODIES of
	// the subloops).
  
	// Traverse the body of the loop in depth first order on the dominator tree so
	// that we are guaranteed to see definitions before we see uses.  This allows
	// us to sink instructions in one pass, without iteration.  After sinking
	// instructions, we perform another pass to hoist them out of the loop.
  
	if (L->hasDedicatedExits())
		SinkRegion(DT->getNode(L->getHeader()));
	if (Preheader)
		HoistRegion(DT->getNode(L->getHeader()));

	// Now that all loop invariants have been removed from the loop, promote any
	// memory references to scalars that we can.
	if (!DisablePromotion && Preheader && L->hasDedicatedExits()) {
		SmallVector<BasicBlock *, 8> ExitBlocks;
		SmallVector<Instruction *, 8> InsertPts;

    		// Loop over all of the alias sets in the tracker object.
		for (AliasSetTracker::iterator I = CurAST->begin(), E = CurAST->end(); I != E; ++I)
			PromoteAliasSet(*I, ExitBlocks, InsertPts);
	}



	for (Loop::iterator LoopItr = L->begin(), LoopItrE = L->end();LoopItr != LoopItrE; ++LoopItr) {
		Loop *InnerL = *LoopItr;
		AliasSetTracker *InnerAST = LoopToAliasSetMap[InnerL];
		assert(InnerAST && "Where is my AST?");

		// What if InnerLoop was modified by other passes ?
		CurAST->add(*InnerAST);

		// Once we've incorporated the inner loop's AST into ours, we don't need the
		// subloop's anymore.
		delete InnerAST;
		LoopToAliasSetMap.erase(InnerL);
	}
	CurLoop = L;
	// Get the preheader block to move instructions into...
	Preheader = L->getLoopPreheader();

	// Loop over the body of this loop, looking for calls, invokes, and stores.
	// Because subloops have already been incorporated into AST, we skip blocks in
	// subloops.



	errs()<<"The first time to check the preheader:\n";
	errs()<<*Preheader<<"\n";


	// print all insts in the loop
	errs()<<"the instructions in the original basic block:\n";
	for (Loop::block_iterator I = L->block_begin(), E = L->block_end(); I != E; ++I) {
		BasicBlock* BB = *I;
		if (LI->getLoopFor(BB) == L){
			errs()<<*BB<<"\n";
		}
	}



	map<Instruction*,vector<Instruction*> > dependChains;//for BFS
	map<Instruction*, Instruction*> hoistedInstructions;//current inst, first load inst
	map<Instruction*, BasicBlock*> instBBMap;
	map<Instruction*, Instruction*> whereToSplit;

	// LAMP profiling information
	map<unsigned int, Instruction*> idToInstMap = LAMP->IdToInstMap;
	map<Instruction*, unsigned int> instToIdMap = LAMP->InstToIdMap;
	map<pair<Instruction*, Instruction*>*, unsigned int> depToTimesMap = LAMP->DepToTimesMap;

	// record Load and dependent store numbers
	map<Instruction*, float> LoadDepFreqMap;
	map<Instruction*, int> LoadDependNumMap;
	
	map<BasicBlock*, int> redoBBMap;
	vector<Instruction*> storeInstVec;

	// get all of the store in the loop
	// store them into store map
	// get all load execution count
	
	for (map<pair<Instruction*, Instruction*>*, unsigned int >::iterator it = depToTimesMap.begin(); it != depToTimesMap.end(); it++){
		Instruction* ldInst = it->first->first;
		if (LoadDependNumMap.find(ldInst)==LoadDependNumMap.end()){
			LoadDependNumMap[ldInst] = it->second;
		}
		else{
			LoadDependNumMap[ldInst] += it->second;
		}
	}
	
	for (Loop::block_iterator I = L->block_begin(), E = L->block_end(); I != E; ++I) {
		BasicBlock *BB = *I;
		if ((LI->getLoopFor(BB) == L) && (redoBBMap.find(BB)==redoBBMap.end())){      
			for (BasicBlock::iterator inst = BB->begin(); inst!=BB->end(); inst++){
				if (inst->getOpcode()==Instruction::Store){
					storeInstVec.push_back(inst);
				}
				else if (inst->getOpcode()==Instruction::Load){
					LoadDepFreqMap[inst] = float(LoadDependNumMap[inst])/float(PI->getExecutionCount(BB));
				}
			}
		}
	}


	// hoist loads and get its dependency chain
	for (Loop::block_iterator I = L->block_begin(), E = L->block_end(); I != E; ++I) {
		BasicBlock *BB = *I;
		if ((LI->getLoopFor(BB) == L) && (redoBBMap.find(BB)==redoBBMap.end())){       // Ignore blocks in subloops.
			
			// find the first load and split the block from this load
			// check if the source register of this load instruction is unchanged
			// if it is unchanged, then this load instruction can be hoisted
			for (BasicBlock::iterator inst = BB->begin(); inst!=BB->end(); inst++){
				if ((canSinkOrHoistLoadAndDependency(*inst))&&(inst->getOpcode()==Instruction::Load)&&(CurLoop->hasLoopInvariantOperands(inst)) && isSafeToExecuteUnconditionally(*inst)){
					Instruction* firstLoadInst = inst;
					

					// if there is a eligible load instruction existing inside the basic block
					// use BFS to find all instructions on the depend chain within this basic block			
					errs()<<" #### first load inst: "<<*firstLoadInst<<"\n";
			
					vector<Instruction*> dependencyChain;
					dependencyChain.push_back(firstLoadInst);
					hoistedInstructions[firstLoadInst] = firstLoadInst;
					inst++;
					hoist(*firstLoadInst);

					for (unsigned int i=0; i<dependencyChain.size(); i++){
						for (Value::use_iterator UI = dependencyChain[i]->use_begin(); UI!=dependencyChain[i]->use_end(); UI++){
							Instruction *User = dyn_cast<Instruction>(*UI);
							errs()<<"User is :"<<*User<<"\n";
							if ((canSinkOrHoistLoadAndDependency(*User))&&(CurLoop->hasLoopInvariantOperands(User)) && isSafeToExecuteUnconditionally(*User)){
								errs()<<"Hoist user\n";
								for (unsigned int j=0; j<User->getNumOperands(); j++){
									Value* tempV=User->getOperand(j);
									Instruction* tempI = dyn_cast<Instruction>(tempV);
																	
									if ((tempI!=dependencyChain[i])&&(tempI!=NULL)){
										// need to insert User into another dependency Chain
										Instruction* oldFirstLoad = hoistedInstructions[tempI];
										dependChains[oldFirstLoad].push_back(User);
									}
								}
								dependencyChain.push_back(User);
								hoistedInstructions[User] = firstLoadInst;
								if (User==inst) inst++;
								hoist(*User);
							}  					
						}
					}

					dependChains[firstLoadInst] = dependencyChain;
					instBBMap[firstLoadInst] = BB;
					whereToSplit[firstLoadInst] = inst;
				}
			}
		}
	}

	// create redoBB, flag and so on...
	for (map<Instruction*,vector<Instruction*> >::iterator it = dependChains.begin(); it!=dependChains.end(); it++){
		Instruction* splitInst = whereToSplit[it->first];
		BasicBlock* splitBlock = instBBMap[it->first];
		// create a flag at the end of the preheader for redoBB
		AllocaInst *flag = new AllocaInst(Type::getInt1Ty(Preheader->getContext()), "flag", Preheader->getTerminator());
		StoreInst *ST = new StoreInst(ConstantInt::getFalse(Preheader->getContext()), flag, Preheader->getTerminator());

		// create comparision instruction after all store instructions
		Instruction* loadInst = it->first;
		Value* LoadRegValue = loadInst->getOperand(0);
		for (unsigned int i=0; i<storeInstVec.size(); i++){
			Instruction* storeInst = storeInstVec[i];
			Value* StoreRegValue = storeInst->getOperand(1);
			ICmpInst* ICMP = new ICmpInst(storeInst->getNextNode(), ICmpInst::ICMP_EQ, StoreRegValue, LoadRegValue, "cmp");
			StoreInst* StoreFlag = new StoreInst(ICMP, flag);
			StoreFlag->insertAfter(ICMP);	
		}


 		// create load and CMP instruction 
		LoadInst* LD = new LoadInst(flag, "flag", splitInst);

		// split the block
		BasicBlock* redoBB = SplitBlock(splitBlock, LD->getNextNode(), this);	
		redoBBMap[redoBB] = 1;

		// create redo BB
		BasicBlock* nextBB = SplitEdge(splitBlock, redoBB, this);

		// create a branch
		BranchInst::Create(nextBB, redoBB, LD, splitBlock->getTerminator());
		// erase the original terminator
		splitBlock->getTerminator()->eraseFromParent();

		// copy the hoisted instruction into redoBB
		vector<Instruction*> dependencyChain = it->second;
		map <Instruction*, int> storeMap;
		map <Instruction*, Instruction*> loadMap;
		for (unsigned int i=0; i<dependencyChain.size(); i++){
			// clone and insert into redo BB
			Instruction* hoistedInst = dependencyChain[i]->clone();
			redoBB->getInstList().insert(redoBB->getTerminator(), hoistedInst);
			
			if (i!=0){
				for (unsigned int j=0; j<dependencyChain[i]->getNumOperands(); j++){
					Value* v = dependencyChain[i]->getOperand(j);
					Instruction* operandInst = dyn_cast<Instruction>(v);
					if (loadMap.find(operandInst)!=loadMap.end()){
						hoistedInst->setOperand(j,loadMap[operandInst]);
					}	
				}
			}

			// need to add store and load instruction to save the value of the dest reg into a stack
			// insert store in Preheader
			AllocaInst *var = new AllocaInst(IntegerType::get(Preheader->getContext(),32), "var", dependencyChain[i]->getNextNode());
			var->setAlignment(4);
			StoreInst *storeVar = new StoreInst(dependencyChain[i], var);
			storeVar->insertAfter(var);
			storeMap[storeVar] = 1;

			// insert store in redo BB
			StoreInst *storeVar2 = new StoreInst(hoistedInst, var);
			storeVar2->insertAfter(hoistedInst);
			storeMap[storeVar2] = 1; 

			// all user of the hoisted instruction need to load var before use it 
			for (Value::use_iterator UI = dependencyChain[i]->use_begin(); UI != dependencyChain[i]->use_end(); UI++){
				Instruction *User = dyn_cast<Instruction>(*UI);

				for (unsigned int j =0; j!=User->getNumOperands(); j++){
					Instruction* temp = dyn_cast<Instruction>(User->getOperand(j));
					if ((temp==dependencyChain[i])&&(storeMap.find(User)==storeMap.end())){
						LoadInst* loadVar = new LoadInst(var, "load var", User);
						LoadInst* loadVar_redoBB = new LoadInst(var, "load_var", storeVar2->getNextNode());
						loadMap[loadVar] = loadVar_redoBB;
						
						User->setOperand(j, loadVar);
					}
				}	 
			}
		}
	}


	// FOR TEST: check the inst in the preheader
	errs()<<"check after the inst is hoisted into the preheader. now insts in the preheader are:\n";
	errs()<<*Preheader<<"\n";
				

	errs()<<"the instructions in the modified basic block:\n";
	for (Loop::block_iterator I = L->block_begin(), E = L->block_end(); I != E; ++I) {
		BasicBlock* BB = *I;
		if (LI->getLoopFor(BB) == L){
			errs()<<*BB<<"\n";
		}
	}


	// Clear out loops state information for the next iteration
	CurLoop = 0;
	Preheader = 0;

	// If this loop is nested inside of another one, save the alias information
	// for when we process the outer loop.
	if (L->getParentLoop())
		LoopToAliasSetMap[L] = CurAST;
	else
		delete CurAST;

	return Changed;
}

/// SinkRegion - Walk the specified region of the CFG (defined by all blocks
/// dominated by the specified block, and that are in the current loop) in
/// reverse depth first order w.r.t the DominatorTree.  This allows us to visit
/// uses before definitions, allowing us to sink a loop body in one pass without
/// iteration.
///
void slicmpass::SinkRegion(DomTreeNode *N) {
	assert(N != 0 && "Null dominator tree node?");
	BasicBlock *BB = N->getBlock();

	// If this subregion is not in the top level loop at all, exit.
	if (!CurLoop->contains(BB)) return;

	// We are processing blocks in reverse dfo, so process children first.
	const std::vector<DomTreeNode*> &Children = N->getChildren();
	for (unsigned i = 0, e = Children.size(); i != e; ++i)
		SinkRegion(Children[i]);

	// Only need to process the contents of this block if it is not part of a
	// subloop (which would already have been processed).
	if (inSubLoop(BB)) return;

	for (BasicBlock::iterator II = BB->end(); II != BB->begin(); ) {
		Instruction &I = *--II;

		// If the instruction is dead, we would try to sink it because it isn't used
		// in the loop, instead, just delete it.
		if (isInstructionTriviallyDead(&I, TLI)) {
			DEBUG(dbgs() << "SLICM deleting dead inst: " << I << '\n');
			++II;
			CurAST->deleteValue(&I);
			I.eraseFromParent();
			Changed = true;
			continue;
		}

		// Check to see if we can sink this instruction to the exit blocks
		// of the loop.  We can do this if the all users of the instruction are
		// outside of the loop.  In this case, it doesn't even matter if the
		// operands of the instruction are loop invariant.
	
		if (isNotUsedInLoop(I) && canSinkOrHoistInst(I)) {
			++II;
			sink(I);
		}	
	}
}

/// HoistRegion - Walk the specified region of the CFG (defined by all blocks
/// dominated by the specified block, and that are in the current loop) in depth
/// first order w.r.t the DominatorTree.  This allows us to visit definitions
/// before uses, allowing us to hoist a loop body in one pass without iteration.
///
void slicmpass::HoistRegion(DomTreeNode *N) {
	assert(N != 0 && "Null dominator tree node?");
	BasicBlock *BB = N->getBlock();

	// If this subregion is not in the top level loop at all, exit.
	if (!CurLoop->contains(BB)) return;

	// Only need to process the contents of this block if it is not part of a
	// subloop (which would already have been processed).
	if (!inSubLoop(BB))
		for (BasicBlock::iterator II = BB->begin(), E = BB->end(); II != E; ) {
			Instruction &I = *II++;
	// Try constant folding this instruction.  If all the operands are
	// constants, it is technically hoistable, but it would be better to just
	// fold it.
	if (Constant *C = ConstantFoldInstruction(&I, TD, TLI)) {
		DEBUG(dbgs() << "SLICM folding inst: " << I << "  --> " << *C << '\n');
		CurAST->copyValue(&I, C);
		CurAST->deleteValue(&I);
		I.replaceAllUsesWith(C);
		I.eraseFromParent();
		continue;
	}

	// Try hoisting the instruction out to the preheader.  We can only do this
	// if all of the operands of the instruction are loop invariant and if it
	// is safe to hoist the instruction.
	//
	if (CurLoop->hasLoopInvariantOperands(&I) && canSinkOrHoistInst(I) &&
		isSafeToExecuteUnconditionally(I))
		hoist(I);
	}

	const std::vector<DomTreeNode*> &Children = N->getChildren();
	for (unsigned i = 0, e = Children.size(); i != e; ++i)
		HoistRegion(Children[i]);
}

/// canSinkOrHoistInst - Return true if the hoister and sinker can handle this
/// instruction.


bool slicmpass::canSinkOrHoistInst(Instruction &I) {
	// Loads have extra constraints we have to verify before we can hoist them.
	if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
		if (!LI->isUnordered())
			return false;        // Don't hoist volatile/atomic loads!

		// Loads from constant memory are always safe to move, even if they end up
		// in the same alias set as something that ends up being modified.
		if (AA->pointsToConstantMemory(LI->getOperand(0)))
			return true;
		if (LI->getMetadata("invariant.load"))
			return true;

		// Don't hoist loads which have may-aliased stores in loop.
		uint64_t Size = 0;
		if (LI->getType()->isSized())
			Size = AA->getTypeStoreSize(LI->getType());
		return !pointerInvalidatedByLoop(LI->getOperand(0), Size, LI->getMetadata(LLVMContext::MD_tbaa));
  	} 
	else if (CallInst *CI = dyn_cast<CallInst>(&I)) {
		// Don't sink or hoist dbg info; it's legal, but not useful.
		if (isa<DbgInfoIntrinsic>(I))
			return false;

		// Handle simple cases by querying alias analysis.
		AliasAnalysis::ModRefBehavior Behavior = AA->getModRefBehavior(CI);
		if (Behavior == AliasAnalysis::DoesNotAccessMemory)
			return true;
		if (AliasAnalysis::onlyReadsMemory(Behavior)) {
		// If this call only reads from memory and there are no writes to memory
		// in the loop, we can hoist or sink the call as appropriate.
			bool FoundMod = false;
			for (AliasSetTracker::iterator I = CurAST->begin(), E = CurAST->end(); I != E; ++I) {
       			 	AliasSet &AS = *I;
				if (!AS.isForwardingAliasSet() && AS.isMod()) {
					FoundMod = true;
					break;
				}
			}
			if (!FoundMod) return true;
		}
		// FIXME: This should use mod/ref information to see if we can hoist or
		// sink the call.

		return false;
	}

	// Only these instructions are hoistable/sinkable.
	if (!isa<BinaryOperator>(I) && !isa<CastInst>(I) && !isa<SelectInst>(I) &&
		!isa<GetElementPtrInst>(I) && !isa<CmpInst>(I) &&
		!isa<InsertElementInst>(I) && !isa<ExtractElementInst>(I) &&
		!isa<ShuffleVectorInst>(I) && !isa<ExtractValueInst>(I) &&
		!isa<InsertValueInst>(I))
		return false;

  	return isSafeToExecuteUnconditionally(I);
}









/// yuxuan: to test can a load be sink or hoist
bool slicmpass::canSinkOrHoistLoadAndDependency(Instruction &I) {
	// Loads have extra constraints we have to verify before we can hoist them.
	if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
		if (!LI->isUnordered())
			return false;        // Don't hoist volatile/atomic loads!

		// Loads from constant memory are always safe to move, even if they end up
		// in the same alias set as something that ends up being modified.
		if (AA->pointsToConstantMemory(LI->getOperand(0)))
			return true;
		if (LI->getMetadata("invariant.load"))
			return true;

		// Don't hoist loads which have may-aliased stores in loop.
	/*	uint64_t Size = 0;
		if (LI->getType()->isSized())
			Size = AA->getTypeStoreSize(LI->getType());
		return !pointerInvalidatedByLoop(LI->getOperand(0), Size, LI->getMetadata(LLVMContext::MD_tbaa));*/
		for (unsigned i = 0, e = I.getNumOperands(); i!=e; ++i){
			Value *V = I.getOperand(i);
			Instruction *temp = dyn_cast<Instruction>(V);
			if (temp!=NULL)
			for (Value::use_iterator UI = temp->use_begin(), E = temp->use_end(); UI!=E; UI++){
				Instruction *User = dyn_cast<Instruction>(*UI);
				if (CurLoop->contains(User)){
					if(isa<StoreInst>(User)){
						return false;
					}
				}
			}
		
		}
		return true;
  	} 
	else if (CallInst *CI = dyn_cast<CallInst>(&I)) {
		// Don't sink or hoist dbg info; it's legal, but not useful.
		if (isa<DbgInfoIntrinsic>(I))
			return false;

		// Handle simple cases by querying alias analysis.
		AliasAnalysis::ModRefBehavior Behavior = AA->getModRefBehavior(CI);
		if (Behavior == AliasAnalysis::DoesNotAccessMemory)
			return true;
		if (AliasAnalysis::onlyReadsMemory(Behavior)) {
		// If this call only reads from memory and there are no writes to memory
		// in the loop, we can hoist or sink the call as appropriate.
			bool FoundMod = false;
			for (AliasSetTracker::iterator I = CurAST->begin(), E = CurAST->end(); I != E; ++I) {
       			 	AliasSet &AS = *I;
				if (!AS.isForwardingAliasSet() && AS.isMod()) {
					FoundMod = true;
					break;
				}
			}
			if (!FoundMod) return true;
		}
		// FIXME: This should use mod/ref information to see if we can hoist or
		// sink the call.

		return false;
	}

	// Only these instructions are hoistable/sinkable.
	if (!isa<BinaryOperator>(I) && !isa<CastInst>(I) && !isa<SelectInst>(I) &&
		!isa<GetElementPtrInst>(I) && !isa<CmpInst>(I) &&
		!isa<InsertElementInst>(I) && !isa<ExtractElementInst>(I) &&
		!isa<ShuffleVectorInst>(I) && !isa<ExtractValueInst>(I) &&
		!isa<InsertValueInst>(I))
		return false;

  	return isSafeToExecuteUnconditionally(I);
}











/// isNotUsedInLoop - Return true if the only users of this instruction are
/// outside of the loop.  If this is true, we can sink the instruction to the
/// exit blocks of the loop.
///
bool slicmpass::isNotUsedInLoop(Instruction &I) {
	for (Value::use_iterator UI = I.use_begin(), E = I.use_end(); UI != E; ++UI) {
		Instruction *User = cast<Instruction>(*UI);
		if (PHINode *PN = dyn_cast<PHINode>(User)) {
		// PHI node uses occur in predecessor blocks!
			for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i)
				if (PN->getIncomingValue(i) == &I)
					if (CurLoop->contains(PN->getIncomingBlock(i)))
						return false;
    		} 
		else if (CurLoop->contains(User)) {
      			return false;
    		}
	}
  	return true;
}


/// sink - When an instruction is found to only be used outside of the loop,
/// this function moves it to the exit blocks and patches up SSA form as needed.
/// This method is guaranteed to remove the original instruction from its
/// position, and may either delete it or move it to outside of the loop.
///
void slicmpass::sink(Instruction &I) {
	DEBUG(dbgs() << "SLICM sinking instruction: " << I << "\n");

	SmallVector<BasicBlock*, 8> ExitBlocks;	
	CurLoop->getUniqueExitBlocks(ExitBlocks);

	if (isa<LoadInst>(I)) ++NumMovedLoads;
	else if (isa<CallInst>(I)) ++NumMovedCalls;
	++NumSunk;
	Changed = true;

	// The case where there is only a single exit node of this loop is common
	// enough that we handle it as a special (more efficient) case.  It is more
	// efficient to handle because there are no PHI nodes that need to be placed.
	if (ExitBlocks.size() == 1) {
		if (!DT->dominates(I.getParent(), ExitBlocks[0])) {
			// Instruction is not used, just delete it.
			CurAST->deleteValue(&I);
			// If I has users in unreachable blocks, eliminate.
			// If I is not void type then replaceAllUsesWith undef.
			// This allows ValueHandlers and custom metadata to adjust itself.
			if (!I.use_empty())
				I.replaceAllUsesWith(UndefValue::get(I.getType()));
			I.eraseFromParent();
		} 
		else {
			// Move the instruction to the start of the exit block, after any PHI
			// nodes in it.
			I.moveBefore(ExitBlocks[0]->getFirstInsertionPt());
	
			// This instruction is no longer in the AST for the current loop, because
			// we just sunk it out of the loop.  If we just sunk it into an outer
			// loop, we will rediscover the operation when we process it.
			CurAST->deleteValue(&I);
		}
		return;
	}
	
	if (ExitBlocks.empty()) {
		// The instruction is actually dead if there ARE NO exit blocks.
		CurAST->deleteValue(&I);
		// If I has users in unreachable blocks, eliminate.
		// If I is not void type then replaceAllUsesWith undef.
		// This allows ValueHandlers and custom metadata to adjust itself.
		if (!I.use_empty())
			I.replaceAllUsesWith(UndefValue::get(I.getType()));
		I.eraseFromParent();
    		return;
	}

	// Otherwise, if we have multiple exits, use the SSAUpdater to do all of the
	// hard work of inserting PHI nodes as necessary.
	SmallVector<PHINode*, 8> NewPHIs;	
	SSAUpdater SSA(&NewPHIs);

	if (!I.use_empty())
		SSA.Initialize(I.getType(), I.getName());

	// Insert a copy of the instruction in each exit block of the loop that is
	// dominated by the instruction.  Each exit block is known to only be in the
	// ExitBlocks list once.
	BasicBlock *InstOrigBB = I.getParent();
	unsigned NumInserted = 0;
	
	for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i) {
		BasicBlock *ExitBlock = ExitBlocks[i];
		if (!DT->dominates(InstOrigBB, ExitBlock))
			continue;

		// Insert the code after the last PHI node.
		BasicBlock::iterator InsertPt = ExitBlock->getFirstInsertionPt();

		// If this is the first exit block processed, just move the original
		// instruction, otherwise clone the original instruction and insert
		// the copy.
		Instruction *New;
		if (NumInserted++ == 0) {
			I.moveBefore(InsertPt);
			New = &I;
		} 
		else {
			New = I.clone();
			if (!I.getName().empty())
				New->setName(I.getName()+".le");
			ExitBlock->getInstList().insert(InsertPt, New);
		}

		// Now that we have inserted the instruction, inform SSAUpdater.
		if (!I.use_empty())
			SSA.AddAvailableValue(ExitBlock, New);
	}

	// If the instruction doesn't dominate any exit blocks, it must be dead.
	if (NumInserted == 0) {
		CurAST->deleteValue(&I);
		if (!I.use_empty())
			I.replaceAllUsesWith(UndefValue::get(I.getType()));
		I.eraseFromParent();
		return;
	}

	// Next, rewrite uses of the instruction, inserting PHI nodes as needed.
  	for (Value::use_iterator UI = I.use_begin(), UE = I.use_end(); UI != UE; ) {
		// Grab the use before incrementing the iterator.
    		Use &U = UI.getUse();
		// Increment the iterator before removing the use from the list.
		++UI;
		SSA.RewriteUseAfterInsertions(U);
	}

	// Update CurAST for NewPHIs if I had pointer type.
	if (I.getType()->isPointerTy())
		for (unsigned i = 0, e = NewPHIs.size(); i != e; ++i)
			CurAST->copyValue(&I, NewPHIs[i]);
	// Finally, remove the instruction from CurAST.  It is no longer in the loop.
  	CurAST->deleteValue(&I);
}

/// hoist - When an instruction is found to only use loop invariant operands
/// that is safe to hoist, this instruction is called to do the dirty work.
///
void slicmpass::hoist(Instruction &I) {
	DEBUG(dbgs() << "SLICM hoisting to " << Preheader->getName() << ": "<< I << "\n");

	// Move the new node to the Preheader, before its terminator.
	I.moveBefore(Preheader->getTerminator());

	if (isa<LoadInst>(I)) ++NumMovedLoads;
	else if (isa<CallInst>(I)) ++NumMovedCalls;
	++NumHoisted;
	Changed = true;
}

/// isSafeToExecuteUnconditionally - Only sink or hoist an instruction if it is
/// not a trapping instruction or if it is a trapping instruction and is
/// guaranteed to execute.
///
bool slicmpass::isSafeToExecuteUnconditionally(Instruction &Inst) {
	// If it is not a trapping instruction, it is always safe to hoist.
	if (isSafeToSpeculativelyExecute(&Inst))
		return true;
	return isGuaranteedToExecute(Inst);
}

bool slicmpass::isGuaranteedToExecute(Instruction &Inst) {

	// Somewhere in this loop there is an instruction which may throw and make us	
	// exit the loop.
	if (MayThrow)
		return false;

	// Otherwise we have to check to make sure that the instruction dominates all
	// of the exit blocks.  If it doesn't, then there is a path out of the loop
	// which does not execute this instruction, so we can't hoist it.

	// If the instruction is in the header block for the loop (which is very
	// common), it is always guaranteed to dominate the exit blocks.  Since this
	// is a common case, and can save some work, check it now.
	if (Inst.getParent() == CurLoop->getHeader())
		return true;

	// Get the exit blocks for the current loop.
	SmallVector<BasicBlock*, 8> ExitBlocks;
	CurLoop->getExitBlocks(ExitBlocks);

	// Verify that the block dominates each of the exit blocks of the loop.
	for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i)
		if (!DT->dominates(Inst.getParent(), ExitBlocks[i]))
			return false;

	// As a degenerate case, if the loop is statically infinite then we haven't
	// proven anything since there are no exit blocks.
	if (ExitBlocks.empty())
		return false;

	return true;
}

namespace {
	class LoopPromoter : public LoadAndStorePromoter {
		Value *SomePtr;  // Designated pointer to store to.
  		SmallPtrSet<Value*, 4> &PointerMustAliases;
    		SmallVectorImpl<BasicBlock*> &LoopExitBlocks;
    		SmallVectorImpl<Instruction*> &LoopInsertPts;
    		AliasSetTracker &AST;
    		DebugLoc DL;
    		int Alignment;
    		MDNode *TBAATag;
  	public:
    		LoopPromoter(Value *SP,
			const SmallVectorImpl<Instruction*> &Insts, SSAUpdater &S,
                 	SmallPtrSet<Value*, 4> &PMA,
                 	SmallVectorImpl<BasicBlock*> &LEB,
                 	SmallVectorImpl<Instruction*> &LIP,
                 	AliasSetTracker &ast, DebugLoc dl, int alignment,
                 	MDNode *TBAATag)
      		: LoadAndStorePromoter(Insts, S), SomePtr(SP),
        	  PointerMustAliases(PMA), LoopExitBlocks(LEB), LoopInsertPts(LIP),
        	  AST(ast), DL(dl), Alignment(alignment), TBAATag(TBAATag) {}

		virtual bool isInstInList(Instruction *I, const SmallVectorImpl<Instruction*> &) const {
      			Value *Ptr;
      			if (LoadInst *LI = dyn_cast<LoadInst>(I))
        			Ptr = LI->getOperand(0);
      			else
        			Ptr = cast<StoreInst>(I)->getPointerOperand();
      			return PointerMustAliases.count(Ptr);
		}

		virtual void doExtraRewritesBeforeFinalDeletion() const {
			// Insert stores after in the loop exit blocks.  Each exit block gets a
			// store of the live-out values that feed them.  Since we've already told
			// the SSA updater about the defs in the loop and the preheader
			// definition, it is all set and we can start using it.
      			for (unsigned i = 0, e = LoopExitBlocks.size(); i != e; ++i) {
        			BasicBlock *ExitBlock = LoopExitBlocks[i];
        			Value *LiveInValue = SSA.GetValueInMiddleOfBlock(ExitBlock);
        			Instruction *InsertPos = LoopInsertPts[i];
       				StoreInst *NewSI = new StoreInst(LiveInValue, SomePtr, InsertPos);
        			NewSI->setAlignment(Alignment);
        			NewSI->setDebugLoc(DL);
        			if (TBAATag) NewSI->setMetadata(LLVMContext::MD_tbaa, TBAATag);
			}
		}

		virtual void replaceLoadWithValue(LoadInst *LI, Value *V) const {
			// Update alias analysis.
			AST.copyValue(LI, V);
		}
		virtual void instructionDeleted(Instruction *I) const {
			AST.deleteValue(I);
		}
	};
} // end anon namespace

/// PromoteAliasSet - Try to promote memory values to scalars by sinking
/// stores out of the loop and moving loads to before the loop.  We do this by
/// looping over the stores in the loop, looking for stores to Must pointers
/// which are loop invariant.
///
void slicmpass::PromoteAliasSet(AliasSet &AS, SmallVectorImpl<BasicBlock*> &ExitBlocks, SmallVectorImpl<Instruction*> &InsertPts) {
	// We can promote this alias set if it has a store, if it is a "Must" alias
  	// set, if the pointer is loop invariant, and if we are not eliminating any
  	// volatile loads or stores.
  	if (AS.isForwardingAliasSet() || !AS.isMod() || !AS.isMustAlias() ||
      		AS.isVolatile() || !CurLoop->isLoopInvariant(AS.begin()->getValue()))
    		return;

  	assert(!AS.empty() && "Must alias set should have at least one pointer element in it!");
	Value *SomePtr = AS.begin()->getValue();

	// It isn't safe to promote a load/store from the loop if the load/store is
	// conditional.  For example, turning:
	//
  	//    for () { if (c) *P += 1; }
  	//
  	// into:
  	//
  	//    tmp = *P;  for () { if (c) tmp +=1; } *P = tmp;
  	//
  	// is not safe, because *P may only be valid to access if 'c' is true.
  	//
  	// It is safe to promote P if all uses are direct load/stores and if at
  	// least one is guaranteed to be executed.
	bool GuaranteedToExecute = false;

	SmallVector<Instruction*, 64> LoopUses;
	SmallPtrSet<Value*, 4> PointerMustAliases;

	// We start with an alignment of one and try to find instructions that allow
	// us to prove better alignment.
	unsigned Alignment = 1;
	MDNode *TBAATag = 0;

	// Check that all of the pointers in the alias set have the same type.  We
	// cannot (yet) promote a memory location that is loaded and stored in
	// different sizes.  While we are at it, collect alignment and TBAA info.
	for (AliasSet::iterator ASI = AS.begin(), E = AS.end(); ASI != E; ++ASI) {
		Value *ASIV = ASI->getValue();
		PointerMustAliases.insert(ASIV);

		// Check that all of the pointers in the alias set have the same type.  We
		// cannot (yet) promote a memory location that is loaded and stored in
		// different sizes.
		if (SomePtr->getType() != ASIV->getType())
 			return;

		for (Value::use_iterator UI = ASIV->use_begin(), UE = ASIV->use_end(); UI != UE; ++UI) {
      			// Ignore instructions that are outside the loop.
      			Instruction *Use = dyn_cast<Instruction>(*UI);
      			if (!Use || !CurLoop->contains(Use))
        			continue;

      			// If there is an non-load/store instruction in the loop, we can't promote
      			// it.
      			if (LoadInst *load = dyn_cast<LoadInst>(Use)) {
        			assert(!load->isVolatile() && "AST broken");
        			if (!load->isSimple())
          				return;
      			} 
			else if (StoreInst *store = dyn_cast<StoreInst>(Use)) {
        			// Stores *of* the pointer are not interesting, only stores *to* the
        			// pointer.
        			if (Use->getOperand(1) != ASIV)
          				continue;
        			assert(!store->isVolatile() && "AST broken");
        			if (!store->isSimple())
          				return;

			        // Note that we only check GuaranteedToExecute inside the store case
      		 	 	// so that we do not introduce stores where they did not exist before
				// (which would break the LLVM concurrency model).
	
 				// If the alignment of this instruction allows us to specify a more
				// restrictive (and performant) alignment and if we are sure this
				// instruction will be executed, update the alignment.
				// Larger is better, with the exception of 0 being the best alignment.
				unsigned InstAlignment = store->getAlignment();
				if ((InstAlignment > Alignment || InstAlignment == 0) && Alignment != 0)
					if (isGuaranteedToExecute(*Use)) {
						GuaranteedToExecute = true;
						Alignment = InstAlignment;
					}
	
       		 		if (!GuaranteedToExecute)
       	   				GuaranteedToExecute = isGuaranteedToExecute(*Use);
	
      			} 
			else
       		 		return; // Not a load or store.
      					// Merge the TBAA tags.
      			if (LoopUses.empty()) {
        			// On the first load/store, just take its TBAA tag.
        			TBAATag = Use->getMetadata(LLVMContext::MD_tbaa);
      			} 
			else if (TBAATag) {
				TBAATag = MDNode::getMostGenericTBAA(TBAATag,
				Use->getMetadata(LLVMContext::MD_tbaa));
			}
      
			LoopUses.push_back(Use);
		}
	}

			// If there isn't a guaranteed-to-execute instruction, we can't promote.
	if (!GuaranteedToExecute)
		return;

	// Otherwise, this is safe to promote, lets do it!
	DEBUG(dbgs() << "SLICM: Promoting value stored to in loop: " <<*SomePtr<<'\n');
	Changed = true;
	++NumPromoted;
	
	// Grab a debug location for the inserted loads/stores; given that the
	// inserted loads/stores have little relation to the original loads/stores,
	// this code just arbitrarily picks a location from one, since any debug
	// location is better than none.
	DebugLoc DL = LoopUses[0]->getDebugLoc();

	// Figure out the loop exits and their insertion points, if this is the
	// first promotion.
	if (ExitBlocks.empty()) {
		CurLoop->getUniqueExitBlocks(ExitBlocks);
		InsertPts.resize(ExitBlocks.size());
		for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i)
			InsertPts[i] = ExitBlocks[i]->getFirstInsertionPt();
	}

	// We use the SSAUpdater interface to insert phi nodes as required.
	SmallVector<PHINode*, 16> NewPHIs;
	SSAUpdater SSA(&NewPHIs);
	LoopPromoter Promoter(SomePtr, LoopUses, SSA, PointerMustAliases, ExitBlocks, InsertPts, *CurAST, DL, Alignment, TBAATag);
	// Set up the preheader to have a definition of the value.  It is the live-out
	// value from the preheader that uses in the loop will use.	
	LoadInst *PreheaderLoad = new LoadInst(SomePtr, SomePtr->getName()+".promoted", Preheader->getTerminator());
	PreheaderLoad->setAlignment(Alignment);
	PreheaderLoad->setDebugLoc(DL);
	if (TBAATag) 
		PreheaderLoad->setMetadata(LLVMContext::MD_tbaa, TBAATag);
	SSA.AddAvailableValue(Preheader, PreheaderLoad);

  	// Rewrite all the loads in the loop and remember all the definitions from
  	// stores in the loop.
	Promoter.run(LoopUses);

	// If the SSAUpdater didn't use the load in the preheader, just zap it now.
	if (PreheaderLoad->use_empty())
		PreheaderLoad->eraseFromParent();
}


/// cloneBasicBlockAnalysis - Simple Analysis hook. Clone alias set info.
void slicmpass::cloneBasicBlockAnalysis(BasicBlock *From, BasicBlock *To, Loop *L) {
	AliasSetTracker *AST = LoopToAliasSetMap.lookup(L);
	if (!AST)
		return;

	AST->copyValue(From, To);
}

/// deleteAnalysisValue - Simple Analysis hook. Delete value V from alias
/// set.
void slicmpass::deleteAnalysisValue(Value *V, Loop *L) {
	AliasSetTracker *AST = LoopToAliasSetMap.lookup(L);
	if (!AST)
		return;

	AST->deleteValue(V);
}
